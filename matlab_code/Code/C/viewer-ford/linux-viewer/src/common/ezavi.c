#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/types.h>
#include <inttypes.h>

#include "ezavi.h"
#include "ioutils.h"

#ifndef XFREE
#define XFREE(a) { if (a) { free(a); } a = NULL; }
#endif

#define dbg(args...) fprintf(stderr, args)
#undef dbg
#define dbg(args...)

#define err(args...) fprintf(stderr, args)
//#undef err
//#define err(args...)


#define AVIF_HASINDEX 0x10
#define AVIIF_KEYFRAME 0x10

typedef struct _chunk_header {
    uint32_t id;
    uint32_t length;
} chunk_header_t, chunk_t;

// encoder definition
typedef struct _encoder encoder_t;

struct _encoder {
    void (*destroy) (encoder_t *self);
    int (*encode_and_write_bgr) (encoder_t *self, const uint8_t *data);
    int (*encode_and_write_bgr_bottom_up) (encoder_t *self, 
            const uint8_t *data);

    char *name;
    ezavi_t *ezavi;
    uint32_t fourcc;
    uint32_t max_bytes_per_frame;
    uint32_t suggested_buffer_size;

    void *priv;
};

// list valid encoders
typedef struct _encoder_descriptor {
    const char *name;
    encoder_t * (*create) (ezavi_t *ezavi);
} encoder_descriptor_t;

static encoder_t *raw_encoder_new (ezavi_t *ezavi);

static encoder_descriptor_t valid_encoders[] = {
    { "raw", raw_encoder_new },
    { NULL, NULL }
};

// ezavi class definition
struct _ezavi {
    ezavi_params_t params;

    int file_count;
    char * cur_fname;
    FILE *fp;

    uint8_t *bottom_up_buf;
    unsigned int frame_count;

    unsigned int movi_start;
    int64_t movi_size;

    encoder_t *encoder;
};

// structure definitions from msdn.microsoft.com

typedef uint32_t FOURCC;

typedef struct _avimainheader {
    FOURCC fcc;
    uint32_t  cb;
    uint32_t  dwMicroSecPerFrame;
    uint32_t  dwMaxBytesPerSec;
    uint32_t  dwPaddingGranularity;
    uint32_t  dwFlags;
    uint32_t  dwTotalFrames;
    uint32_t  dwInitialFrames;
    uint32_t  dwStreams;
    uint32_t  dwSuggestedBufferSize;
    uint32_t  dwWidth;
    uint32_t  dwHeight;
    uint32_t  dwReserved[4];
} AVIMAINHEADER;

typedef struct _avistreamheader {
    FOURCC fcc;
    uint32_t  cb;
    FOURCC fccType;
    FOURCC fccHandler;
    uint32_t  dwFlags;
    uint16_t   wPriority;
    uint16_t   wLanguage;
    uint32_t  dwInitialFrames;
    uint32_t  dwScale;
    uint32_t  dwRate;
    uint32_t  dwStart;
    uint32_t  dwLength;
    uint32_t  dwSuggestedBufferSize;
    uint32_t  dwQuality;
    uint32_t  dwSampleSize;
    struct {
        short int left;
        short int top;
        short int right;
        short int bottom;
    }  rcFrame;
} AVISTREAMHEADER;

typedef struct tagBITMAPINFOHEADER{
    uint32_t  biSize; 
    int32_t   biWidth; 
    int32_t   biHeight; 
    uint16_t   biPlanes; 
    uint16_t   biBitCount; 
    uint32_t  biCompression; 
    uint32_t  biSizeImage; 
    int32_t   biXPelsPerMeter; 
    int32_t   biYPelsPerMeter; 
    uint32_t  biClrUsed; 
    uint32_t  biClrImportant; 
} BITMAPINFOHEADER, *PBITMAPINFOHEADER; 

typedef struct _avioldindex_entry {
    uint32_t   dwChunkId;
    uint32_t   dwFlags;
    uint32_t   dwOffset;
    uint32_t   dwSize;
} AVIOLDINDEX_ENTRY;

static inline uint32_t 
make_fourcc (char a, char b, char c, char d) {
    uint32_t result = 0;
    char *t = (char*) &result;
    t[0] = a; t[1] = b; t[2] = c; t[3] = d;
    return result;
}

static inline uint32_t
make_fourcc_from_str (const char s[4]) {
    return make_fourcc (s[0], s[1], s[2], s[3]);
}

static chunk_t *
make_chunk (const char id[4], uint32_t length, void *data)
{
    chunk_t *chunk = (chunk_t*) malloc (sizeof(chunk_t) + length);
    chunk->id = make_fourcc_from_str (id);
    chunk->length = length;
    memcpy (chunk + 1, data, length);
    return chunk;
}

static chunk_t *
make_empty_chunk (const char id[4], uint32_t length)
{
    chunk_t *chunk = (chunk_t*) calloc (1, sizeof(chunk_t) + length);
    chunk->id = make_fourcc_from_str (id);
    chunk->length = length;
    return chunk;
}

static chunk_t *
make_list_chunk (const char id[4], int free_chunks, ...)
{
    chunk_t *result = (chunk_t*) malloc (sizeof(chunk_t) + 4);
    result->id = make_fourcc_from_str ("LIST");
    result->length = 4;
    memcpy (result + 1, id, sizeof(id));

    va_list ap;
    va_start (ap, free_chunks);
    int i;
    for (i=0; ; i++) {
        chunk_t *c = va_arg (ap, chunk_t *);
        if (! c) return result;

        uint32_t oldlen = result->length;

        result->length += sizeof (chunk_header_t) + c->length;
        result = (chunk_t*) realloc (result, sizeof(chunk_t) + result->length);

        uint8_t *dstart = ((uint8_t*)result) + sizeof(chunk_t) + oldlen;
        memcpy (dstart, c, sizeof(chunk_t) + c->length);
        if (free_chunks) free (c);
    }
}

ezavi_t * 
ezavi_new (const ezavi_params_t *p)
{
    ezavi_t *s = (ezavi_t*) calloc (1, sizeof(ezavi_t));

    s->params.file_prefix = strdup (p->file_prefix);
    if (p->path)
        s->params.path = strdup (p->path);
    else
        s->params.path = NULL;
    s->params.date_in_file = p->date_in_file;

    s->params.codec = strdup (p->codec);

    s->params.width = p->width;
    s->params.height = p->height;
    s->params.src_stride = p->src_stride;
    s->params.frame_rate = p->frame_rate;
    s->params.split_point = p->split_point;

    if (s->params.split_point == 0) {
        s->params.split_point = 1 << 30;  // default split point - 1 GB
    }

    s->movi_start = 1024;

    s->file_count = 0;
    s->cur_fname = get_unique_filename (s->params.path, s->params.file_prefix,
            s->params.date_in_file, "avi");

    // try to create the encoder
    int enc_ind;
    for (enc_ind = 0; 
            valid_encoders[enc_ind].name != NULL; 
            enc_ind++) {
        if (!strcmp (valid_encoders[enc_ind].name, p->codec)) {
            s->encoder = valid_encoders[enc_ind].create (s);
            break;
        }
    }
    if (!s->encoder) {
        ezavi_destroy (s);
        return NULL;
    }

    // try to open the output file
    printf ("Saving AVI to \"%s\"...\n", s->cur_fname);
    s->fp = fopen (s->cur_fname, "wb+");
    if (! s->fp) {
        perror ("fopen");
        ezavi_destroy (s);
        return NULL;
    }
    fseek (s->fp, s->movi_start + 4, SEEK_SET);

    // allocate temporary buffers
    s->bottom_up_buf = (uint8_t*) malloc (p->height * p->src_stride);

    s->frame_count = 0;
    s->movi_size = 0;

    return s;
}

void 
ezavi_destroy (ezavi_t *s)
{
    ezavi_finish (s);
    XFREE (s->params.codec);
    XFREE (s->params.file_prefix);
    XFREE (s->params.path);
    XFREE (s->bottom_up_buf);
    if (s->encoder) {
        s->encoder->destroy (s->encoder);
    }

    free (s);
}

char *
ezavi_get_filename (ezavi_t * s)
{
    return s->cur_fname;
}

static int
write_headers (ezavi_t *s) 
{
#define _CHECK_STATUS(expected, msg) if (status != expected) { \
    perror(msg); return -1; }

    int status;

    status = fseek (s->fp, 0, SEEK_SET);
    _CHECK_STATUS (0, "fseek");

    char *riff_header = "RIFF\0\0\0\0AVI ";
    status = fwrite (riff_header, 12, 1, s->fp);
    _CHECK_STATUS (1, "fwrite");

    int width = s->params.width;
    int height = s->params.height;
    int frame_rate = s->params.frame_rate;
    int max_bytes_per_second = (int)s->encoder->max_bytes_per_frame*frame_rate;
    int suggested_buffer_size = s->encoder->suggested_buffer_size;

    AVIMAINHEADER chunk_mainheader = {
        make_fourcc_from_str ("avih"),
        sizeof (AVIMAINHEADER) - sizeof (chunk_header_t),
        1e6 / frame_rate,               // microseconds per frame
        max_bytes_per_second,           // maximum bytes per second
        1,                              // padding granularity
        AVIF_HASINDEX,                  // flags
        s->frame_count,
        0,                              // not interleaved, no initial frames
        1,                              // numstreams - hardcode to 1
        suggested_buffer_size,          // suggested buffer size
        width,
        height,
        { 0, 0, 0, 0 }
    };

    AVISTREAMHEADER chunk_strh_video = {
        make_fourcc_from_str ("strh"),
        sizeof (AVISTREAMHEADER) - sizeof (chunk_header_t),
        make_fourcc_from_str ("vids"),
        s->encoder->fourcc,             // codec
        0,                              // no flags
        0,                              // priority
        0,                              // no language
        0,                              // no skew
        1000,                           // dwRate
        (int) 1000 * frame_rate,        // dwScale
        0,                              //
        s->frame_count,                 //
        suggested_buffer_size,          // suggested buffer size
        0,                              // ignore quality
        0,                              // ignore video_sample_size
        { 0, 0, width, height }            // this stream spans the entire frame
    };

    BITMAPINFOHEADER bmih = {
        sizeof (BITMAPINFOHEADER),
        width, height,                  // image size
        1,                              // 1 plane?
        24,                             // bits per pixel
        0,                              // no image compression (BI_RGB = 0L)
        suggested_buffer_size,          // bytes per frame
        0,                              // ignore X pixels per meter
        0,                              // ignore Y pixels per meter
        0,                              // no color indices used
        0                               // color indices not important
    };

    chunk_t *chunk_strf_video = make_chunk ("strf", sizeof(bmih), &bmih);
    chunk_t *list_strl_video = make_list_chunk ("strl", 0,
            &chunk_strh_video, chunk_strf_video, NULL);

    free (chunk_strf_video);

    chunk_t *list_hdrl = make_list_chunk ("hdrl", 0, 
            &chunk_mainheader, list_strl_video, NULL);
    free (list_strl_video);

    status = fwrite (list_hdrl, list_hdrl->length + sizeof(chunk_header_t), 1,
            s->fp);
    free (list_hdrl);
    _CHECK_STATUS (1, "fwrite");

    // padding chunk (variable length)
    int junksize = s->movi_start - ftell (s->fp) - 8 - 8;
    chunk_t *chunk_junk = make_empty_chunk ("JUNK", junksize);
    status = fwrite (chunk_junk, junksize + sizeof(chunk_header_t), 1, s->fp);
    free (chunk_junk);
    _CHECK_STATUS (1, "fwrite");

    // movi list header
    uint32_t movilen = 0;
    if (s->movi_size > UINT32_MAX) {
        err("AVI file too big, movi list length not written "
                "correctly.\n");
        movilen = 4;
    } else {
        movilen = (uint32_t) s->movi_size + 4;
    }

    status = fwrite ("LIST", 4, 1, s->fp);
    _CHECK_STATUS (1, "fwrite");
    status = fwrite (&movilen, 4, 1, s->fp);
    _CHECK_STATUS (1, "fwrite");
    status = fwrite ("movi", 4, 1, s->fp);
    _CHECK_STATUS (1, "fwrite");

    assert (ftell (s->fp) == s->movi_start + 4);

    // try to write the riff size
    fseeko (s->fp, 0, SEEK_END);
    off_t real_riff_size = ftello (s->fp) - sizeof(chunk_header_t);
    fseeko (s->fp, 4, SEEK_SET);
    uint32_t riff_size = 0;
    if (real_riff_size > UINT32_MAX) {
        err("AVI file too big. RIFF size not written correctly\n");
        riff_size = 0;
    } else {
        riff_size = real_riff_size;
    }
    status = fwrite (&riff_size, 4, 1, s->fp);
    _CHECK_STATUS (1, "fwrite");
    return 0;
}

static int
write_index (ezavi_t *s)
{
    dbg("ezavi: writing index...\n");
    int status;
    fseek (s->fp, 0, SEEK_END);
    chunk_header_t index_hdr = {
        make_fourcc_from_str("idx1"),
        s->frame_count*sizeof(AVIOLDINDEX_ENTRY)
    };

    uint32_t frame_chunk_offset = s->movi_start + 4;

    status = fwrite (&index_hdr, sizeof(index_hdr), 1, s->fp);
    if (1 != status) {
        perror("fwrite");
        return -1;
    }

    off_t index_offset = ftello (s->fp);

    uint32_t i;
    for (i=0; i<s->frame_count; i++) {
        // seek to the chunk in question and figure out how big it is
        fseek (s->fp, frame_chunk_offset, SEEK_SET);

        chunk_header_t cur_chunk_header;
        fread (&cur_chunk_header, sizeof (cur_chunk_header), 1, s->fp);

        uint32_t cur_chunk_size = cur_chunk_header.length + 
            sizeof(cur_chunk_header);

        frame_chunk_offset += cur_chunk_size;

        // seek to the index and write an entry for the chunk
        fseek (s->fp, index_offset, SEEK_SET);

        AVIOLDINDEX_ENTRY index_entry = {
            make_fourcc_from_str("00db"),
            AVIIF_KEYFRAME,
            frame_chunk_offset,
            cur_chunk_size
        };
//        dbg("ezavi: frame %d starts at %d\n", i, offset);
        status = fwrite (&index_entry, sizeof(index_entry), 1, s->fp);
        if (1 != status) {
            perror("fwrite");
            return -1;
        }
    }
    dbg("ezavi: wrote %d index entries\n", i);
    return 0;
}

static int
check_file_size (ezavi_t *s, int next_frame_size)
{
    uint32_t projected_index_size = s->frame_count * 
       sizeof(AVIOLDINDEX_ENTRY) + 8;
    uint32_t pos = ftell (s->fp);
    uint32_t projected_file_size = pos + next_frame_size + projected_index_size;

    if (projected_file_size > s->params.split_point) {
        dbg("ezavi:  AVI file getting too big.  splitting file at %u bytes\n",
                pos);

        // split off to a new file
        write_index (s);
        write_headers (s);
        fclose (s->fp);

        s->file_count++;
        free (s->cur_fname);
        s->cur_fname = get_unique_filename (s->params.path,
                s->params.file_prefix, s->params.date_in_file, "avi");

        printf ("Continuing AVI as \"%s\"...\n", s->cur_fname);
        s->fp = fopen(s->cur_fname, "wb+");
        if (! s->fp) {
            perror("fopen");
            return -1;
        }
        fseek (s->fp, s->movi_start + 4, SEEK_SET);
    }
    return 0;
}

void 
ezavi_finish (ezavi_t *s)
{
    if (s->fp) {
        write_index (s);
        write_headers (s);
        fclose (s->fp);
        s->fp = NULL;
        free (s->cur_fname);
        s->cur_fname = NULL;
    }
}

static int
ezavi_write_to_file (ezavi_t *s, const uint8_t *data, int len)
{
    dbg("ezavi: frame %d starts at %ld (%d data bytes)\n", 
            s->frame_count, ftell(s->fp), len);
    int status;
    status = check_file_size (s, len + sizeof (chunk_header_t));
    if (0 != status) return status;

    chunk_header_t video_chunk_header = {
        make_fourcc_from_str ("00db"),
        len
    };

    status = fwrite (&video_chunk_header, 
            sizeof(video_chunk_header), 1, s->fp);

    if (1 != status) {
        perror("fwrite");
        return -1;
    }
    status = fwrite (data, len, 1, s->fp);
    if (1 != status) {
        perror("fwrite");
        return -1;
    }
    s->frame_count++;
    s->movi_size += len + sizeof (chunk_header_t);

    fflush (s->fp);
    return 0;
}

int 
ezavi_write_video (ezavi_t *s, const uint8_t *data)
{
    return s->encoder->encode_and_write_bgr (s->encoder, data);
}

int 
ezavi_write_video_bottom_up (ezavi_t *s, const uint8_t *data)
{
    if (s->encoder->encode_and_write_bgr_bottom_up) {
        return s->encoder->encode_and_write_bgr_bottom_up (s->encoder, data);
    } else {
        int i;
        int width = s->params.width;
        int height = s->params.height;
        int src_stride = s->params.src_stride;

        for (i=0; i<height; i++) {
            memcpy (s->bottom_up_buf + (height - i - 1) * src_stride,
                    data + i * src_stride,
                    width * 3);
        }
        return s->encoder->encode_and_write_bgr (s->encoder, data);
    }
}

double 
ezavi_get_video_pts (ezavi_t *s)
{
    return s->frame_count / (double) s->params.frame_rate;
    return 0;
}

unsigned int 
ezavi_get_frame_count (ezavi_t * s)
{
    return s->frame_count;
}

// ================ uncompressed BGR encoder ================

typedef struct _raw_encoder_priv_t {
    uint8_t *swap_buf;
    int rowstride; 
    int bpf;            // bytes per frame
} raw_encoder_priv_t;

static void
raw_encoder_destroy (encoder_t *self)
{
    raw_encoder_priv_t *priv = (raw_encoder_priv_t*) self->priv;
    free (priv->swap_buf);
    free (priv);
    free (self);
}

static int
raw_encoder_encode_and_write (encoder_t *self, const uint8_t *data)
{
    raw_encoder_priv_t *priv = (raw_encoder_priv_t*) self->priv;
    int i;
    int width = self->ezavi->params.width;
    int height = self->ezavi->params.height;
    int src_stride = self->ezavi->params.src_stride;

    for (i=0; i<height; i++) {
        memcpy (priv->swap_buf + i*priv->rowstride, 
                data + (height - i - 1)*src_stride,
                width*3);
    }

    return ezavi_write_to_file (self->ezavi, priv->swap_buf, priv->bpf);
}

static int
raw_encoder_encode_and_write_bottom_up (encoder_t *self, const uint8_t *data)
{
    raw_encoder_priv_t *priv = (raw_encoder_priv_t*) self->priv;
    int can_fastcopy = (0 == self->ezavi->params.width % 4);

    if (can_fastcopy) {
        return ezavi_write_to_file (self->ezavi, data, priv->bpf);
    } else {
        int i;
        int width = self->ezavi->params.width;
        int height = self->ezavi->params.height;
        int src_stride = self->ezavi->params.src_stride;

        for (i=0; i<height; i++) {
            memcpy (priv->swap_buf + i*priv->rowstride, 
                    data + i*src_stride,
                    width*3);
        }

        return ezavi_write_to_file (self->ezavi, priv->swap_buf, priv->bpf);
    }
}

static encoder_t *
raw_encoder_new (ezavi_t *ezavi)
{
    encoder_t *self = (encoder_t*) malloc (sizeof (encoder_t));
    self->destroy = raw_encoder_destroy;
    self->encode_and_write_bgr = raw_encoder_encode_and_write;
    self->encode_and_write_bgr_bottom_up = 
        raw_encoder_encode_and_write_bottom_up;
    self->ezavi = ezavi;
    self->name = "raw";

    raw_encoder_priv_t *priv = 
        (raw_encoder_priv_t*) malloc (sizeof (raw_encoder_priv_t));

    // each row must be padded to a multiple of 4 bytes
    int base_rowstride = ezavi->params.width * 3;
    priv->rowstride = base_rowstride + (4 - (base_rowstride%4))%4;

    priv->bpf = ezavi->params.height * priv->rowstride;
    priv->swap_buf = (uint8_t*) malloc (priv->bpf);

    self->max_bytes_per_frame = priv->bpf;
    self->suggested_buffer_size = priv->bpf;
    self->fourcc = make_fourcc_from_str ("DIB ");
    self->priv = priv;

    return self;
}

// ==========
