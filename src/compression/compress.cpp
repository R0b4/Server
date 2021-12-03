#include <zlib.h>
#include <stdio.h>
#include <assert.h>

//source: https://www.zlib.net/zlib_how.html

int gzip_compress(FILE *source, FILE *dest, int level)
{
    constexpr uInt chunk_size = 16384;

    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[chunk_size];
    unsigned char out[chunk_size];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit2(&strm, level, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY); //https://stackoverflow.com/questions/49622938/gzip-compression-using-zlib-into-buffer
    
    if (ret != Z_OK) return ret;

    /* compress until end of file */
    do {
        strm.avail_in = fread(in, 1, chunk_size, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = chunk_size;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = chunk_size - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}