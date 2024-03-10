// Copyright 2016-2024 ClickHouse, Inc.
// Modified by Michael Bernardi
// Licensed under the Apache 2.0 License.

#include "util.hpp"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>

#define ALWAYS_INLINE __attribute__((__always_inline__))

typedef uint64_t UInt64;
typedef uint32_t UInt32;
typedef uint8_t UInt8;

template <std::integral T> constexpr T byteswap(T value) noexcept
{
    static_assert(std::has_unique_object_representations_v<T>, "T may not have padding bits");
    auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
    std::ranges::reverse(value_representation);
    return std::bit_cast<T>(value_representation);
}

/** Returns a mask that has '1' for `bits` LSB set:
 * maskLowBits<UInt8>(3) => 00000111
 * maskLowBits<Int8>(3) => 00000111
 */
template <typename T> inline T maskLowBits(unsigned char bits)
{
    using UnsignedT = std::make_unsigned_t<T>;
    if (bits == 0)
    {
        return 0;
    }

    UnsignedT result = static_cast<UnsignedT>(~UnsignedT{0});
    if (bits < sizeof(T) * 8)
    {
        result = static_cast<UnsignedT>(result >> (sizeof(UnsignedT) * 8 - bits));
    }

    return static_cast<T>(result);
}

class BitReader
{
    const char *const source_begin;
    const char *const source_end;
    const char *source_current;

    using BufferType = unsigned __int128;
    BufferType bits_buffer = 0;

    UInt8 bits_count = 0;

  public:
    BitReader(const char *begin, size_t size) : source_begin(begin), source_end(begin + size), source_current(begin)
    {
    }

    ~BitReader() = default;

    // reads bits_to_read high-bits from bits_buffer
    ALWAYS_INLINE UInt64 readBits(UInt8 bits_to_read)
    {
        if (bits_to_read > bits_count)
            fillBitBuffer();

        return getBitsFromBitBuffer<CONSUME>(bits_to_read);
    }

    UInt8 peekByte()
    {
        if (bits_count < 8)
            fillBitBuffer();

        return getBitsFromBitBuffer<PEEK>(8);
    }

    ALWAYS_INLINE UInt8 readBit()
    {
        return static_cast<UInt8>(readBits(1));
    }

    // skip bits from bits_buffer
    void skipBufferedBits(UInt8 bits)
    {
        bits_buffer <<= bits;
        bits_count -= bits;
    }

    bool eof() const
    {
        return bits_count == 0 && source_current >= source_end;
    }

    // number of bits that was already read by clients with readBits()
    UInt64 count() const
    {
        return (source_current - source_begin) * 8 - bits_count;
    }

    UInt64 remaining() const
    {
        return (source_end - source_current) * 8 + bits_count;
    }

  private:
    enum GetBitsMode
    {
        CONSUME,
        PEEK
    };
    // read data from internal buffer, if it has not enough bits, result is undefined.
    template <GetBitsMode mode> UInt64 getBitsFromBitBuffer(UInt8 bits_to_read)
    {
        assert(bits_to_read > 0);

        // push down the high-bits
        const UInt64 result = static_cast<UInt64>(bits_buffer >> (sizeof(bits_buffer) * 8 - bits_to_read));

        if constexpr (mode == CONSUME)
        {
            // 'erase' high-bits that were have read
            skipBufferedBits(bits_to_read);
        }

        return result;
    }

    // Fills internal bits_buffer with data from source, reads at most 64 bits
    ALWAYS_INLINE size_t fillBitBuffer()
    {
        const size_t available = source_end - source_current;
        const auto bytes_to_read = std::min<size_t>(64 / 8, available);
        if (available == 0)
        {
            if (bytes_to_read == 0)
                return 0;

            throw std::runtime_error(
                string_format("Buffer is empty, but requested to read %d more bytes.", bytes_to_read));
        }

        UInt64 tmp_buffer = 0;
        memcpy(&tmp_buffer, source_current, bytes_to_read);
        source_current += bytes_to_read;

        if constexpr (std::endian::native == std::endian::little)
            tmp_buffer = byteswap(tmp_buffer);

        bits_buffer |= BufferType(tmp_buffer) << ((sizeof(BufferType) - sizeof(tmp_buffer)) * 8 - bits_count);
        bits_count += static_cast<UInt8>(bytes_to_read) * 8;

        return bytes_to_read;
    }
};

class BitWriter
{
    char *dest_begin;
    char *dest_end;
    char *dest_current;

    using BufferType = unsigned __int128;
    BufferType bits_buffer = 0;

    UInt8 bits_count = 0;

    static constexpr UInt8 BIT_BUFFER_SIZE = sizeof(bits_buffer) * 8;

  public:
    BitWriter(char *begin, size_t size) : dest_begin(begin), dest_end(begin + size), dest_current(begin)
    {
    }

    ~BitWriter()
    {
        flush();
    }

    // write `bits_to_write` low-bits of `value` to the buffer
    void writeBits(UInt8 bits_to_write, UInt64 value)
    {
        assert(bits_to_write > 0);

        UInt32 capacity = BIT_BUFFER_SIZE - bits_count;
        if (capacity < bits_to_write)
        {
            doFlush();
            capacity = BIT_BUFFER_SIZE - bits_count;
        }

        // write low bits of value as high bits of bits_buffer
        const UInt64 mask = maskLowBits<UInt64>(bits_to_write);
        BufferType v = value & mask;
        v <<= capacity - bits_to_write;

        bits_buffer |= v;
        bits_count += bits_to_write;
    }

    // flush contents of bits_buffer to the dest_current, partial bytes are completed with zeroes.
    void flush()
    {
        bits_count =
            (bits_count + 8 - 1) & ~(8 - 1); // align up to 8-bytes, so doFlush will write all data from bits_buffer
        while (bits_count != 0)
            doFlush();
    }

    UInt64 count() const
    {
        return (dest_current - dest_begin) * 8 + bits_count;
    }

  private:
    void doFlush()
    {
        // write whole bytes to the dest_current, leaving partial bits in bits_buffer
        const size_t available = dest_end - dest_current;
        const size_t to_write = std::min<size_t>(sizeof(UInt64), bits_count / 8); // align to 8-bit boundary

        if (available < to_write)
        {
            throw std::runtime_error(string_format(
                "Can not write past end of buffer. Space available is %d bytes, required to write %d bytes.", available,
                to_write));
        }

        UInt64 tmp_buffer = static_cast<UInt64>(bits_buffer >> (sizeof(bits_buffer) - sizeof(UInt64)) * 8);
        if constexpr (std::endian::native == std::endian::little)
            tmp_buffer = byteswap(tmp_buffer);

        memcpy(dest_current, &tmp_buffer, to_write);
        dest_current += to_write;

        bits_buffer <<= to_write * 8;
        bits_count -= to_write * 8;
    }
};

// Unsafe since __builtin_ctz()-family explicitly state that result is undefined on x == 0
template <typename T> inline size_t getTrailingZeroBitsUnsafe(T x)
{
    assert(x != 0);

    if constexpr (sizeof(T) <= sizeof(unsigned int))
    {
        return __builtin_ctz(x);
    }
    else if constexpr (sizeof(T) <= sizeof(unsigned long int)) /// NOLINT
    {
        return __builtin_ctzl(x);
    }
    else
    {
        return __builtin_ctzll(x);
    }
}

template <typename T> inline size_t getTrailingZeroBits(T x)
{
    if (!x)
        return sizeof(x) * 8;

    return getTrailingZeroBitsUnsafe(x);
}

template <typename T> inline uint32_t getLeadingZeroBitsUnsafe(T x)
{
    assert(x != 0);

    if constexpr (sizeof(T) <= sizeof(unsigned int))
    {
        return __builtin_clz(x);
    }
    else if constexpr (sizeof(T) <= sizeof(unsigned long int)) /// NOLINT
    {
        return __builtin_clzl(x);
    }
    else
    {
        return __builtin_clzll(x);
    }
}

template <typename T> inline size_t getLeadingZeroBits(T x)
{
    if (!x)
        return sizeof(x) * 8;

    return getLeadingZeroBitsUnsafe(x);
}

struct BinaryValueInfo
{
    UInt8 leading_zero_bits;
    UInt8 data_bits;
    UInt8 trailing_zero_bits;
};

template <typename T> BinaryValueInfo getBinaryValueInfo(const T &value)
{
    constexpr UInt8 bit_size = sizeof(T) * 8;

    const UInt8 lz = getLeadingZeroBits(value);
    const UInt8 tz = getTrailingZeroBits(value);
    const UInt8 data_size = value == 0 ? 0 : static_cast<UInt8>(bit_size - lz - tz);

    return {lz, data_size, tz};
}

inline void reverseMemcpy(void *dst, const void *src, size_t size)
{
    uint8_t *uint_dst = reinterpret_cast<uint8_t *>(dst);
    const uint8_t *uint_src = reinterpret_cast<const uint8_t *>(src);

    uint_dst += size;
    while (size)
    {
        --uint_dst;
        *uint_dst = *uint_src;
        ++uint_src;
        --size;
    }
}

template <std::endian endian, typename T> inline T unalignedLoadEndian(const void *address)
{
    T res{};
    if constexpr (std::endian::native == endian)
        memcpy(&res, address, sizeof(res));
    else
        reverseMemcpy(&res, address, sizeof(res));
    return res;
}

template <std::endian endian, typename T> inline void unalignedStoreEndian(void *address, T &src)
{
    static_assert(std::is_trivially_copyable_v<T>);
    if constexpr (std::endian::native == endian)
        memcpy(address, &src, sizeof(src));
    else
        reverseMemcpy(address, &src, sizeof(src));
}

template <typename T> inline T unalignedLoadLittleEndian(const void *address)
{
    return unalignedLoadEndian<std::endian::little, T>(address);
}

template <typename T>
inline void unalignedStoreLittleEndian(void *address, const typename std::enable_if<true, T>::type &src)
{
    unalignedStoreEndian<std::endian::little>(address, src);
}

constexpr UInt8 getBitLengthOfLength(UInt8 data_bytes_size)
{
    // 1-byte value is 8 bits, and we need 4 bits to represent 8 : 1000,
    // 2-byte         16 bits        =>    5
    // 4-byte         32 bits        =>    6
    // 8-byte         64 bits        =>    7
    const UInt8 bit_lengths[] = {0, 4, 5, 0, 6, 0, 0, 0, 7};
    assert(data_bytes_size >= 1 && data_bytes_size < sizeof(bit_lengths) && bit_lengths[data_bytes_size] != 0);
    return bit_lengths[data_bytes_size];
};

template <typename T> UInt32 getCompressedDataSize(UInt32 uncompressed_size)
{
    UInt8 data_bytes_size = sizeof(T);
    const UInt32 items_count = uncompressed_size / data_bytes_size;

    static const auto DATA_BIT_LENGTH = getBitLengthOfLength(data_bytes_size);
    // -1 since there must be at least 1 non-zero bit.
    static const auto LEADING_ZEROES_BIT_LENGTH = DATA_BIT_LENGTH - 1;

    // worst case (for 32-bit value):
    // 11 + 5 bits of leading zeroes bit-size + 5 bits of data bit-size + non-zero data bits.
    const UInt32 max_item_size_bits = 2 + LEADING_ZEROES_BIT_LENGTH + DATA_BIT_LENGTH + data_bytes_size * 8;

    // + 8 is to round up to next byte.
    return (items_count * max_item_size_bits + 8) / 8;
}

template <typename T> UInt32 compressDataForType(const char *source, UInt32 source_size, char *dest, UInt32 dest_size)
{
    if (source_size % sizeof(T) != 0)
        throw std::runtime_error(string_format("Cannot compress with Gorilla codec, data size %d is not aligned to %d",
                                               source_size, sizeof(T)));

    const char *const source_end = source + source_size;
    const char *const dest_start = dest;
    const char *const dest_end = dest + dest_size;

    const UInt32 items_count = source_size / sizeof(T);

    unalignedStoreLittleEndian<UInt32>(dest, items_count);
    dest += sizeof(items_count);

    T prev_value = 0;
    // That would cause first XORed value to be written in-full.
    BinaryValueInfo prev_xored_info{0, 0, 0};

    if (source < source_end)
    {
        prev_value = unalignedLoadLittleEndian<T>(source);
        unalignedStoreLittleEndian<T>(dest, prev_value);

        source += sizeof(prev_value);
        dest += sizeof(prev_value);
    }

    BitWriter writer(dest, dest_end - dest);

    static const auto DATA_BIT_LENGTH = getBitLengthOfLength(sizeof(T));
    // -1 since there must be at least 1 non-zero bit.
    static const auto LEADING_ZEROES_BIT_LENGTH = DATA_BIT_LENGTH - 1;

    while (source < source_end)
    {
        const T curr_value = unalignedLoadLittleEndian<T>(source);
        source += sizeof(curr_value);

        const auto xored_data = curr_value ^ prev_value;
        const BinaryValueInfo curr_xored_info = getBinaryValueInfo(xored_data);

        if (xored_data == 0)
        {
            writer.writeBits(1, 0);
        }
        else if (prev_xored_info.data_bits != 0 &&
                 prev_xored_info.leading_zero_bits <= curr_xored_info.leading_zero_bits &&
                 prev_xored_info.trailing_zero_bits <= curr_xored_info.trailing_zero_bits)
        {
            writer.writeBits(2, 0b10);
            writer.writeBits(prev_xored_info.data_bits, xored_data >> prev_xored_info.trailing_zero_bits);
        }
        else
        {
            writer.writeBits(2, 0b11);
            writer.writeBits(LEADING_ZEROES_BIT_LENGTH, curr_xored_info.leading_zero_bits);
            writer.writeBits(DATA_BIT_LENGTH, curr_xored_info.data_bits);
            writer.writeBits(curr_xored_info.data_bits, xored_data >> curr_xored_info.trailing_zero_bits);
            prev_xored_info = curr_xored_info;
        }

        prev_value = curr_value;
    }

    writer.flush();

    return static_cast<UInt32>((dest - dest_start) + (writer.count() + 7) / 8);
}

template <typename T> void decompressDataForType(const char *source, UInt32 source_size, char *dest, UInt32 dest_size)
{
    const char *const source_end = source + source_size;

    if (source + sizeof(UInt32) > source_end)
        return;

    const UInt32 items_count = unalignedLoadLittleEndian<UInt32>(source);
    source += sizeof(items_count);

    T prev_value = 0;

    // decoding first item
    if (source + sizeof(T) > source_end || items_count < 1)
        return;

    if (static_cast<UInt64>(items_count) * sizeof(T) > dest_size)
        throw std::runtime_error("Cannot decompress Gorilla-encoded data: corrupted input data.");

    prev_value = unalignedLoadLittleEndian<T>(source);
    unalignedStoreLittleEndian<T>(dest, prev_value);

    source += sizeof(prev_value);
    dest += sizeof(prev_value);

    BitReader reader(source, source_size - sizeof(items_count) - sizeof(prev_value));

    BinaryValueInfo prev_xored_info{0, 0, 0};

    static const auto DATA_BIT_LENGTH = getBitLengthOfLength(sizeof(T));
    // -1 since there must be at least 1 non-zero bit.
    static const auto LEADING_ZEROES_BIT_LENGTH = DATA_BIT_LENGTH - 1;

    // since data is tightly packed, up to 1 bit per value, and last byte is padded with zeroes,
    // we have to keep track of items to avoid reading more that there is.
    for (UInt32 items_read = 1; items_read < items_count && !reader.eof(); ++items_read)
    {
        T curr_value = prev_value;
        BinaryValueInfo curr_xored_info = prev_xored_info;
        T xored_data = 0;

        if (reader.readBit() == 1)
        {
            if (reader.readBit() == 1)
            {
                // 0b11 prefix
                curr_xored_info.leading_zero_bits = reader.readBits(LEADING_ZEROES_BIT_LENGTH);
                curr_xored_info.data_bits = reader.readBits(DATA_BIT_LENGTH);
                curr_xored_info.trailing_zero_bits =
                    sizeof(T) * 8 - curr_xored_info.leading_zero_bits - curr_xored_info.data_bits;
            }
            // else: 0b10 prefix - use prev_xored_info

            if (curr_xored_info.leading_zero_bits == 0 && curr_xored_info.data_bits == 0 &&
                curr_xored_info.trailing_zero_bits == 0) [[unlikely]]
            {
                throw std::runtime_error("Cannot decompress Gorilla-encoded data: corrupted input data.");
            }

            xored_data = static_cast<T>(reader.readBits(curr_xored_info.data_bits));
            xored_data <<= curr_xored_info.trailing_zero_bits;
            curr_value = prev_value ^ xored_data;
        }
        // else: 0b0 prefix - use prev_value

        unalignedStoreLittleEndian<T>(dest, curr_value);
        dest += sizeof(curr_value);

        prev_xored_info = curr_xored_info;
        prev_value = curr_value;
    }
}
