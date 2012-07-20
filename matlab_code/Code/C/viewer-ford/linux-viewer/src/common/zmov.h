#ifndef _ZMOV_H
#define _ZMOV_H

#include <zlib.h>

struct zmov_header
{
    int width;
    int height;
    int stride;
};

typedef struct zmov_writer zmov_writer_t;
struct zmov_writer
{
    struct zmov_header header;

    int     bytes_per_frame;

    gzFile *f;
    char    *path;

    int     nframes;
};

zmov_writer_t *zmov_writer_create(const char *path, int width, int height, int stride);
int zmov_writer_add(zmov_writer_t *zm, void *data);
int zmov_writer_finish(zmov_writer_t *zm);
void zmov_writer_destroy(zmov_writer_t *zm);

typedef struct zmov_reader zmov_reader_t;
struct zmov_reader
{
    struct zmov_header header;
    int     bytes_per_frame;

    char   *path;
    gzFile *f;
};

zmov_reader_t *zmov_reader_create(const char *path);
int zmov_reader_read(zmov_reader_t *zm, void *data);
void zmov_reader_destroy(zmov_reader_t *zm);

#endif
