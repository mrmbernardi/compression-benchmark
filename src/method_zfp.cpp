#include "method.hpp"
#include <cassert>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <vector>
#include <zfp.h>

template <typename F> zfp_type get_zfp_type();
template <> zfp_type get_zfp_type<float>()
{
    return zfp_type_float;
}
template <> zfp_type get_zfp_type<double>()
{
    return zfp_type_double;
}

template <typename F> size_t Zfp<F>::compress(std::span<const F> input)
{
    zfp_type type;     /* array scalar type */
    zfp_field *field;  /* array meta data */
    zfp_stream *zfp;   /* compressed stream */
    size_t bufsize;    /* byte size of compressed buffer */
    bitstream *stream; /* bit stream to write to or read from */
    size_t zfpsize;    /* byte size of compressed stream */

    /* allocate meta data for the 1D array a[nx] */
    type = get_zfp_type<F>();
    void *storage_ptr = const_cast<F *>(input.data());
    field = zfp_field_1d(storage_ptr, type, input.size());

    /* allocate meta data for a compressed stream */
    zfp = zfp_stream_open(NULL);
    zfp_stream_set_accuracy(zfp, Method<F>::error);

    /* allocate buffer for compressed data */
    bufsize = zfp_stream_maximum_size(zfp, field) + ((ZFP_META_BITS + 8) / 8);
    compressed_buffer = std::vector<std::byte>(bufsize);

    /* associate bit stream with allocated buffer */
    stream = stream_open(compressed_buffer.data(), bufsize);
    zfp_stream_set_bit_stream(zfp, stream);
    zfp_stream_rewind(zfp);

    /* compress array and output compressed stream */
    zfp_write_header(zfp, field, ZFP_HEADER_META);
    zfpsize = zfp_compress(zfp, field);
    if (!zfpsize)
    {
        throw std::runtime_error("zfp compression failed");
    }
    zfpsize += ((ZFP_META_BITS + 8) / 8);
    compressed_buffer.resize(zfpsize);

    /* clean up */
    zfp_stream_flush(zfp);
    zfp_field_free(field);
    zfp_stream_close(zfp);
    stream_close(stream);
    return zfpsize;
}

template <typename F> std::span<const F> Zfp<F>::decompress()
{
    zfp_field *field;  /* array meta data */
    zfp_stream *zfp;   /* compressed stream */
    bitstream *stream; /* bit stream to write to or read from */
    size_t zfpsize;    /* byte size of compressed stream */
    
    field = zfp_field_alloc();
    stream = stream_open(compressed_buffer.data(), compressed_buffer.size());
    zfp = zfp_stream_open(stream);
    zfp_stream_set_accuracy(zfp, Method<F>::error);
    zfp_stream_rewind(zfp);
    zfp_read_header(zfp, field, ZFP_HEADER_META);
    assert(field->type == get_zfp_type<F>());
    assert(field->sx == 0);
    results = std::vector<F>(field->nx);
    zfp_field_set_pointer(field, results.data());
    zfpsize = zfp_decompress(zfp, field);
    if (!zfpsize)
    {
        throw std::runtime_error("zfp decompression failed");
    }

    /* clean up */
    zfp_field_free(field);
    zfp_stream_close(zfp);
    stream_close(stream);
    return results;
}
template class Zfp<float>;
template class Zfp<double>;