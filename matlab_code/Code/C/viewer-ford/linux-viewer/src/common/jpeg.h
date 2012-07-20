#ifndef _jpeg_h_
#define _jpeg_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int
jpeg_decompress_to_8u_rgb (const uint8_t * src, int src_size,
        uint8_t * dest, int width, int height, int stride);
int
jpeg_compress_8u_gray (const uint8_t * src, int width, int height, int stride,
        uint8_t * dest, int * destsize, int quality);
int
jpeg_compress_8u_rgb (const uint8_t * src, int width, int height, int stride,
        uint8_t * dest, int * destsize, int quality);
int
jpeg_compress_8u_bgra (const uint8_t * src, int width, int height, int stride,
        uint8_t * dest, int * destsize, int quality);

int
jpeg_decompress_to_8u_rgb_IPP (const uint8_t * src, int src_size,
        uint8_t * dest, int width, int height, int stride);


#ifdef __cplusplus
}
#endif


#endif
