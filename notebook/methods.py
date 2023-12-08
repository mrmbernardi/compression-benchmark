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


lib = ctypes.CDLL(os.path.abspath("../build/libcompression-benchmark.so"))
lib.reconstruct.argtypes = [
    ctypes.POINTER(bench_result),
    ctypes.c_char_p,
    ctypes.c_char,
    ctypes.c_void_p,
    ctypes.c_int,
]
lib.reconstruct.restype = ctypes.c_int


def reconstruct(method_name: str, array: np.ndarray):
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
    )
    if ret != 0:
        raise Exception("Reconstruct failed")
    return np.frombuffer(buf, dtype=array.dtype)


BSC = "Bsc (lossless)"
LFZIP_BSC = "LfZip with Bsc"
LFZIP_LZ4 = "LfZip with Lz4"
LFZIP_STREAMSPLIT2_BSC = "LfZip with Stream Split (2) with Bsc"
LFZIP_STREAMSPLIT2_LZ4 = "LfZip with Stream Split (2) with Lz4"
LFZIP_STREAMSPLIT2_ZSTD3 = "LfZip with Stream Split (2) with Zstd (3)"
LFZIP_STREAMSPLIT4_BSC = "LfZip with Stream Split (4) with Bsc"
LFZIP_STREAMSPLIT4_LZ4 = "LfZip with Stream Split (4) with Lz4"
LFZIP_STREAMSPLIT4_ZSTD3 = "LfZip with Stream Split (4) with Zstd (3)"
LFZIP_STREAMSPLIT8_BSC = "LfZip with Stream Split (8) with Bsc"
LFZIP_STREAMSPLIT8_LZ4 = "LfZip with Stream Split (8) with Lz4"
LFZIP_STREAMSPLIT8_ZSTD3 = "LfZip with Stream Split (8) with Zstd (3)"
LFZIP_ZSTD3 = "LfZip with Zstd (3)"
LZ4 = "Lz4 (lossless)"
MACHETE = "Machete"
QUANTISE_BSC = "Quantise with Bsc"
QUANTISE_LZ4 = "Quantise with Lz4"
QUANTISE_STREAMSPLIT2_BSC = "Quantise with Stream Split (2) with Bsc"
QUANTISE_STREAMSPLIT2_LZ4 = "Quantise with Stream Split (2) with Lz4"
QUANTISE_STREAMSPLIT2_ZSTD3 = "Quantise with Stream Split (2) with Zstd (3)"
QUANTISE_STREAMSPLIT4_BSC = "Quantise with Stream Split (4) with Bsc"
QUANTISE_STREAMSPLIT4_LZ4 = "Quantise with Stream Split (4) with Lz4"
QUANTISE_STREAMSPLIT4_ZSTD3 = "Quantise with Stream Split (4) with Zstd (3)"
QUANTISE_STREAMSPLIT8_BSC = "Quantise with Stream Split (8) with Bsc"
QUANTISE_STREAMSPLIT8_LZ4 = "Quantise with Stream Split (8) with Lz4"
QUANTISE_STREAMSPLIT8_ZSTD3 = "Quantise with Stream Split (8) with Zstd (3)"
QUANTISE_ZSTD3 = "Quantise with Zstd (3)"
STREAMSPLIT4_BSC = "Stream Split (4) with Bsc (lossless)"
STREAMSPLIT4_LZ4 = "Stream Split (4) with Lz4 (lossless)"
STREAMSPLIT4_ZSTD3 = "Stream Split (4) with Zstd (3) (lossless)"
STREAMSPLIT8_BSC = "Stream Split (8) with Bsc (lossless)"
STREAMSPLIT8_LZ4 = "Stream Split (8) with Lz4 (lossless)"
STREAMSPLIT8_ZSTD3 = "Stream Split (8) with Zstd (3) (lossless)"
SZ3 = "Sz3"
ZSTD3 = "Zstd (3) (lossless)"

AllMethods = [
    BSC,
    LFZIP_BSC,
    LFZIP_LZ4,
    LFZIP_STREAMSPLIT2_BSC,
    LFZIP_STREAMSPLIT2_LZ4,
    LFZIP_STREAMSPLIT2_ZSTD3,
    LFZIP_STREAMSPLIT4_BSC,
    LFZIP_STREAMSPLIT4_LZ4,
    LFZIP_STREAMSPLIT4_ZSTD3,
    LFZIP_STREAMSPLIT8_BSC,
    LFZIP_STREAMSPLIT8_LZ4,
    LFZIP_STREAMSPLIT8_ZSTD3,
    LFZIP_ZSTD3,
    LZ4,
    MACHETE,
    QUANTISE_BSC,
    QUANTISE_LZ4,
    QUANTISE_STREAMSPLIT2_BSC,
    QUANTISE_STREAMSPLIT2_LZ4,
    QUANTISE_STREAMSPLIT2_ZSTD3,
    QUANTISE_STREAMSPLIT4_BSC,
    QUANTISE_STREAMSPLIT4_LZ4,
    QUANTISE_STREAMSPLIT4_ZSTD3,
    QUANTISE_STREAMSPLIT8_BSC,
    QUANTISE_STREAMSPLIT8_LZ4,
    QUANTISE_STREAMSPLIT8_ZSTD3,
    QUANTISE_ZSTD3,
    STREAMSPLIT4_BSC,
    STREAMSPLIT4_LZ4,
    STREAMSPLIT4_ZSTD3,
    STREAMSPLIT8_BSC,
    STREAMSPLIT8_LZ4,
    STREAMSPLIT8_ZSTD3,
    SZ3,
    ZSTD3,
]
