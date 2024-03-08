#include "method.hpp"
#include <span>
#include <stdexcept>
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
    void *buffer;      /* storage for compressed stream */
    size_t bufsize;    /* byte size of compressed buffer */
    bitstream *stream; /* bit stream to write to or read from */
    size_t zfpsize;    /* byte size of compressed stream */

    /* allocate meta data for the 1D array a[nx] */
    type = get_zfp_type<F>();
    field = zfp_field_1d(input.data(), type, input.size());

    /* allocate meta data for a compressed stream */
    zfp = zfp_stream_open(NULL);
    zfp_stream_set_accuracy(zfp, Method<F>::error);

    /* allocate buffer for compressed data */
    bufsize = zfp_stream_maximum_size(zfp, field);
    buffer = malloc(bufsize); // TODO Fix

    /* associate bit stream with allocated buffer */
    stream = stream_open(buffer, bufsize);
    zfp_stream_set_bit_stream(zfp, stream);
    zfp_stream_rewind(zfp);

    /* compress array and output compressed stream */
    zfpsize = zfp_compress(zfp, field);
    if (!zfpsize)
    {
        throw std::runtime_error("zfp compression failed");
    }

    /* clean up */
    zfp_field_free(field);
    zfp_stream_close(zfp);
    stream_close(stream);
    free(buffer); // TODO Fix

    return zfpsize;
}

template <typename F> std::span<const F> Zfp<F>::decompress()
{
    return results;
}