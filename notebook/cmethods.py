import ctypes
import os
import numpy as np


class bench_result(ctypes.Structure):
    _fields_ = [
        ("original_size", ctypes.c_size_t),
        ("compressed_size", ctypes.c_size_t),
        ("compression_time", ctypes.c_double),
        ("decompression_time", ctypes.c_double),
        ("max_error", ctypes.c_double),
        ("mean_absolute_error", ctypes.c_double),
    ]

    # https://stackoverflow.com/a/62011887/3901677
    def __repr__(self) -> str:
        values = ", ".join(f"{name}={value}" for name, value in self._asdict().items())
        return f"<{self.__class__.__name__}: {values}>"

    def _asdict(self) -> dict:
        return {field[0]: getattr(self, field[0]) for field in self._fields_}


lib = ctypes.CDLL(
    os.path.join(
        os.path.dirname(os.path.realpath(__file__)),
        "../build/libcompression-benchmark.so",
    )
)
lib.reconstruct.argtypes = [
    ctypes.POINTER(bench_result),
    ctypes.c_char_p,
    ctypes.c_char,
    ctypes.c_void_p,
    ctypes.c_int,
    ctypes.c_double,
]
lib.reconstruct.restype = ctypes.c_int


def reconstruct(method_name: str, array: np.ndarray, error_bound: float):
    if array.dtype == np.float32:
        dtype_char = ord("f")
    elif array.dtype == np.float64:
        dtype_char = ord("d")
    else:
        raise Exception("Unsupported data type")
    buf = array.tobytes()
    results = bench_result()
    ret = lib.reconstruct(
        ctypes.byref(results),
        ctypes.create_string_buffer(method_name.encode("ascii")),
        dtype_char,
        buf,
        array.size,
        error_bound
    )
    if ret != 0:
        raise Exception("Reconstruct failed")
    return (
        np.frombuffer(buf, dtype=array.dtype),
        results.compressed_size,
        results.compression_time,
        results.decompression_time,
    )


class Method:
    def __init__(self, method_name, supported):
        self.method_name = method_name
        self.__name__ = method_name
        self.supported = supported

    def __repr__(self) -> str:
        return f"Method({self.method_name.__repr__()}, {self.supported.__repr__()})"

    def reconstruct(self, data: np.ndarray, error_bound=1.0) -> tuple[np.ndarray, int]:
        return reconstruct(self.method_name, data, error_bound)


BSC = Method("Bsc (lossless)", ["float", "double"])
INTFLOAT_BSC = Method("IntFloat with Bsc", ["float", "double"])
INTFLOAT_LZ4 = Method("IntFloat with Lz4", ["float", "double"])
INTFLOAT_PCODEC = Method("IntFloat with Pcodec", ["float", "double"])
INTFLOAT_PCODECINT = Method("IntFloat with Pcodec (int)", ["float", "double"])
INTFLOAT_PCODECUINT = Method("IntFloat with Pcodec (uint)", ["float", "double"])
INTFLOAT_SNAPPY = Method("IntFloat with Snappy", ["float", "double"])
INTFLOAT_STREAMSPLIT4_BSC = Method("IntFloat with Stream Split (4) with Bsc", ["float"])
INTFLOAT_STREAMSPLIT4_LZ4 = Method("IntFloat with Stream Split (4) with Lz4", ["float"])
INTFLOAT_STREAMSPLIT4_SNAPPY = Method("IntFloat with Stream Split (4) with Snappy", ["float"])
INTFLOAT_STREAMSPLIT4_ZSTD3 = Method("IntFloat with Stream Split (4) with Zstd (3)", ["float"])
INTFLOAT_STREAMSPLIT8_BSC = Method("IntFloat with Stream Split (8) with Bsc", ["double"])
INTFLOAT_STREAMSPLIT8_LZ4 = Method("IntFloat with Stream Split (8) with Lz4", ["double"])
INTFLOAT_STREAMSPLIT8_SNAPPY = Method("IntFloat with Stream Split (8) with Snappy", ["double"])
INTFLOAT_STREAMSPLIT8_ZSTD3 = Method("IntFloat with Stream Split (8) with Zstd (3)", ["double"])
INTFLOAT_ZSTD3 = Method("IntFloat with Zstd (3)", ["float", "double"])
LFZIP_BSC = Method("LfZip with Bsc", ["float", "double"])
LFZIP_LZ4 = Method("LfZip with Lz4", ["float", "double"])
LFZIP_SNAPPY = Method("LfZip with Snappy", ["float", "double"])
LFZIP_STREAMSPLITV_BSC = Method("LfZip with Stream Split (V) with Bsc", ["float", "double"])
LFZIP_STREAMSPLITV_LZ4 = Method("LfZip with Stream Split (V) with Lz4", ["float", "double"])
LFZIP_STREAMSPLITV_SNAPPY = Method("LfZip with Stream Split (V) with Snappy", ["float", "double"])
LFZIP_STREAMSPLITV_ZSTD3 = Method("LfZip with Stream Split (V) with Zstd (3)", ["float", "double"])
LFZIP_ZSTD3 = Method("LfZip with Zstd (3)", ["float", "double"])
LZ4 = Method("Lz4 (lossless)", ["float", "double"])
MACHETE = Method("Machete", ["double"])
MASK_BSC = Method("Mask with Bsc", ["float", "double"])
MASK_LZ4 = Method("Mask with Lz4", ["float", "double"])
MASK_PCODEC = Method("Mask with Pcodec", ["float", "double"])
MASK_PCODECINT = Method("Mask with Pcodec (int)", ["float", "double"])
MASK_PCODECUINT = Method("Mask with Pcodec (uint)", ["float", "double"])
MASK_SNAPPY = Method("Mask with Snappy", ["float", "double"])
MASK_STREAMSPLIT4_BSC = Method("Mask with Stream Split (4) with Bsc", ["float"])
MASK_STREAMSPLIT4_LZ4 = Method("Mask with Stream Split (4) with Lz4", ["float"])
MASK_STREAMSPLIT4_SNAPPY = Method("Mask with Stream Split (4) with Snappy", ["float"])
MASK_STREAMSPLIT4_ZSTD3 = Method("Mask with Stream Split (4) with Zstd (3)", ["float"])
MASK_STREAMSPLIT8_BSC = Method("Mask with Stream Split (8) with Bsc", ["double"])
MASK_STREAMSPLIT8_LZ4 = Method("Mask with Stream Split (8) with Lz4", ["double"])
MASK_STREAMSPLIT8_SNAPPY = Method("Mask with Stream Split (8) with Snappy", ["double"])
MASK_STREAMSPLIT8_ZSTD3 = Method("Mask with Stream Split (8) with Zstd (3)", ["double"])
MASK_ZSTD3 = Method("Mask with Zstd (3)", ["float", "double"])
PCODECINT = Method("Pcodec (int) (lossless)", ["float", "double"])
PCODEC = Method("Pcodec (lossless)", ["float", "double"])
PCODECUINT = Method("Pcodec (uint) (lossless)", ["float", "double"])
QUANTISE_BSC = Method("Quantise with Bsc", ["float", "double"])
QUANTISE_LZ4 = Method("Quantise with Lz4", ["float", "double"])
QUANTISE_SNAPPY = Method("Quantise with Snappy", ["float", "double"])
QUANTISE_STREAMSPLITV_BSC = Method("Quantise with Stream Split (V) with Bsc", ["float", "double"])
QUANTISE_STREAMSPLITV_LZ4 = Method("Quantise with Stream Split (V) with Lz4", ["float", "double"])
QUANTISE_STREAMSPLITV_SNAPPY = Method("Quantise with Stream Split (V) with Snappy", ["float", "double"])
QUANTISE_STREAMSPLITV_ZSTD3 = Method("Quantise with Stream Split (V) with Zstd (3)", ["float", "double"])
QUANTISE_ZSTD3 = Method("Quantise with Zstd (3)", ["float", "double"])
SNAPPY = Method("Snappy (lossless)", ["float", "double"])
STREAMSPLIT4_BSC = Method("Stream Split (4) with Bsc (lossless)", ["float"])
STREAMSPLIT4_LZ4 = Method("Stream Split (4) with Lz4 (lossless)", ["float"])
STREAMSPLIT4_SNAPPY = Method("Stream Split (4) with Snappy (lossless)", ["float"])
STREAMSPLIT4_ZSTD3 = Method("Stream Split (4) with Zstd (3) (lossless)", ["float"])
STREAMSPLIT8_BSC = Method("Stream Split (8) with Bsc (lossless)", ["double"])
STREAMSPLIT8_LZ4 = Method("Stream Split (8) with Lz4 (lossless)", ["double"])
STREAMSPLIT8_SNAPPY = Method("Stream Split (8) with Snappy (lossless)", ["double"])
STREAMSPLIT8_ZSTD3 = Method("Stream Split (8) with Zstd (3) (lossless)", ["double"])
SZ3 = Method("Sz3", ["float", "double"])
ZSTD3 = Method("Zstd (3) (lossless)", ["float", "double"])

AllMethods = [
    BSC,
    INTFLOAT_BSC,
    INTFLOAT_LZ4,
    INTFLOAT_PCODEC,
    INTFLOAT_PCODECINT,
    INTFLOAT_PCODECUINT,
    INTFLOAT_SNAPPY,
    INTFLOAT_STREAMSPLIT4_BSC,
    INTFLOAT_STREAMSPLIT4_LZ4,
    INTFLOAT_STREAMSPLIT4_SNAPPY,
    INTFLOAT_STREAMSPLIT4_ZSTD3,
    INTFLOAT_STREAMSPLIT8_BSC,
    INTFLOAT_STREAMSPLIT8_LZ4,
    INTFLOAT_STREAMSPLIT8_SNAPPY,
    INTFLOAT_STREAMSPLIT8_ZSTD3,
    INTFLOAT_ZSTD3,
    LFZIP_BSC,
    LFZIP_LZ4,
    LFZIP_SNAPPY,
    LFZIP_STREAMSPLITV_BSC,
    LFZIP_STREAMSPLITV_LZ4,
    LFZIP_STREAMSPLITV_SNAPPY,
    LFZIP_STREAMSPLITV_ZSTD3,
    LFZIP_ZSTD3,
    LZ4,
    MACHETE,
    MASK_BSC,
    MASK_LZ4,
    MASK_PCODEC,
    MASK_PCODECINT,
    MASK_PCODECUINT,
    MASK_SNAPPY,
    MASK_STREAMSPLIT4_BSC,
    MASK_STREAMSPLIT4_LZ4,
    MASK_STREAMSPLIT4_SNAPPY,
    MASK_STREAMSPLIT4_ZSTD3,
    MASK_STREAMSPLIT8_BSC,
    MASK_STREAMSPLIT8_LZ4,
    MASK_STREAMSPLIT8_SNAPPY,
    MASK_STREAMSPLIT8_ZSTD3,
    MASK_ZSTD3,
    PCODECINT,
    PCODEC,
    PCODECUINT,
    QUANTISE_BSC,
    QUANTISE_LZ4,
    QUANTISE_SNAPPY,
    QUANTISE_STREAMSPLITV_BSC,
    QUANTISE_STREAMSPLITV_LZ4,
    QUANTISE_STREAMSPLITV_SNAPPY,
    QUANTISE_STREAMSPLITV_ZSTD3,
    QUANTISE_ZSTD3,
    SNAPPY,
    STREAMSPLIT4_BSC,
    STREAMSPLIT4_LZ4,
    STREAMSPLIT4_SNAPPY,
    STREAMSPLIT4_ZSTD3,
    STREAMSPLIT8_BSC,
    STREAMSPLIT8_LZ4,
    STREAMSPLIT8_SNAPPY,
    STREAMSPLIT8_ZSTD3,
    SZ3,
    ZSTD3,
]
