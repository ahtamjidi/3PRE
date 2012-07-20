#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <zlib.h>
#include <assert.h>
#include <arpa/inet.h>

#include "zmov.h"

#define COMPRESS_LEVEL 1

#define MAGIC 0x1234ededUL

static int write_header(struct zmov_header *hdr, gzFile *f)
{
    uint32_t u;

    u = htonl(MAGIC);
    if (gzwrite(f, &u, sizeof(u))<=0)
        return -1;

    u = htonl(hdr->width);
    gzwrite(f, &u, sizeof(u));

    u = htonl(hdr->height);
    gzwrite(f, &u, sizeof(u));

    u = htonl(hdr->stride);
    gzwrite(f, &u, sizeof(u));

    return 0;
}

static int read_header(struct zmov_header *hdr, gzFile *f)
{
    uint32_t u;

    if (gzread(f, &u, sizeof(u)) <= 1)
        return -1;

    assert (ntohl(u) == MAGIC);

    gzread(f, &u, sizeof(u));
    hdr->width = ntohl(u);

    gzread(f, &u, sizeof(u));
    hdr->height = ntohl(u);

    gzread(f, &u, sizeof(u));
    hdr->stride = ntohl(u);

    return 0;
}

/** Write a simple zlib-compressed stream of images which can be post
    processed into an avi. **/
zmov_writer_t *zmov_writer_create(const char *path, int width, int height, int stride)
{
    zmov_writer_t *zm = (zmov_writer_t*) calloc(1, sizeof(zmov_writer_t));

    zm->path = strdup(path);
    zm->f = gzopen(zm->path, "w");

    zm->header.width = width;
    zm->header.height = height;
    zm->header.stride = stride;

    zm->bytes_per_frame = stride * height;
  
    write_header(&zm->header, zm->f);

    gzsetparams(zm->f, Z_BEST_SPEED, Z_DEFAULT_STRATEGY);
    return zm;
}

int zmov_writer_add(zmov_writer_t *zm, void *data)
{
    if (gzwrite(zm->f, data, zm->bytes_per_frame)<=0)
        return -1;

//    printf("zmov frame %6d : %8d -> %d\n", zm->nframes, zm->bytes_per_frame, 0);
    zm->nframes++;

    return 0;
}

int zmov_writer_finish(zmov_writer_t *zm)
{
    gzclose(zm->f);
    printf("zmov closed file %s\n", zm->path);
    return 0;
}

void zmov_writer_destroy(zmov_writer_t *zm)
{
    free(zm->path);
    free(zm);
}

///////////////////////////////////////////////////////////////////

zmov_reader_t* zmov_reader_create(const char *path)
{
    zmov_reader_t *zm = (zmov_reader_t*) calloc(1, sizeof(zmov_reader_t));

    zm->path = strdup(path);
    zm->f = gzopen(path, "r");
    if (zm->f == NULL) {
        printf("Couldn't open zmov\n");
        free(zm->path);
        free(zm);
        return NULL;
    }

    read_header(&zm->header, zm->f);

    zm->bytes_per_frame = zm->header.stride * zm->header.height;

    printf("%5d x %5d\n", zm->header.width, zm->header.height);
    return zm;
}

int zmov_reader_read(zmov_reader_t *zm, void *data)
{
    if (gzread(zm->f, data, zm->bytes_per_frame) <= 0)
        return -1;

    return 0;
}

void zmov_reader_destroy(zmov_reader_t *zm)
{
    gzclose(zm->f);
    free(zm->path);
    free(zm);
}
