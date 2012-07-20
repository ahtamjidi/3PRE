// file: ppm.h
// auth: Albert Huang <albert@csail.mit.edu>
// date: July 26, 2006
// desc: reads a PPM file into a memory buffer
//
// $Id: ppm.h 288 2006-08-01 03:30:30Z mwalter $

#ifndef __ppm_h__
#define __ppm_h__

#include <stdio.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

int ppm_read (FILE *fp, uint8_t **pixels, 
        int *width, int *height, int *rowstride);

int ppm_write (FILE *fp, const uint8_t *pixels,
        int width, int height, int rowstride);

int ppm_write_bottom_up (FILE *fp, uint8_t *pixels,
        int width, int height, int rowstride);

int pgm_read (FILE *fp, uint8_t **pixels,
        int *width, int *height, int *rowstride);

int pgm_write (FILE *fp, const uint8_t *pixels,
        int width, int height, int rowstride);

#ifdef __cplusplus
}
#endif

#endif
