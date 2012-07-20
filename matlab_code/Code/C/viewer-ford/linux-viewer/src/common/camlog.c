#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include <inttypes.h>
#include "ppm.h"

#include "camlog.h"
#include "pixels.h"

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#if 0
#define dbg(args...) fprintf(stderr, args)
#define dbgl(args...) { fprintf (stderr, "%s:%d ", __FILE__, __LINE__); \
    fprintf (stderr, args); }
#define USE_DBG
#else
#define dbg(args...)
#define dbgl(args...)
#endif
#define errl(args...) { fprintf (stderr, "%s:%d ", __FILE__, __LINE__); \
    fprintf (stderr, args); }
#define err(args...) fprintf(stderr, args)

typedef enum {
    CAMLOG_VERSION_INVALID,
    CAMLOG_VERSION_LEGACY,
    CAMLOG_VERSION_0
} camlog_version_t;

typedef enum 
{
    CAMLOG_MODE_READ,
    CAMLOG_MODE_WRITE
} camlog_mode_t;

struct _Camlog {
    FILE *fp;
    camlog_mode_t mode;
    off_t file_size;

    // read mode only
    camlog_version_t log_version;

    camlog_frame_info_t first_frame_info;
    camlog_frame_info_t last_frame_info;

    camlog_frame_info_t next_frame_info;
    camlog_frame_info_t prev_frame_info;

    // legacy read mode only
    int computed_frame_spacing;
    uint32_t frame_spacing;

    // write mode only
    uint64_t prev_frame_offset;
    uint32_t frameno;
};


// ========================= legacy log functions ========================
#define LOG_MARKER  0xEDED
#define LOG_HEADER_SIZE 8

typedef enum {
    LOG_TYPE_FRAME_DATA = 1,
    LOG_TYPE_FRAME_FORMAT = 2,
    LOG_TYPE_FRAME_TIMESTAMP = 3,
    LOG_TYPE_COMMENT = 4,
    LOG_TYPE_SOURCE_UID = 6,
    LOG_TYPE_FRAME_INFO_0 = 7,
    LOG_TYPE_MAX
} LogType;

// LOG_TYPE_FRAME_INFO_0:
//    uint16_t width;
//    uint16_t height;
//    uint16_t stride;
//    uint32_t pixelformat;
//    uint64_t timestamp;
//    uint32_t bus_timestamp;
//    uint64_t source_uid;
//    uint32_t frameno;
//    uint64_t prev_frame_offset;
#define LOG_FRAME_INFO_0_SIZE 42

static inline void
log_put_uint16 (uint16_t val, FILE * f)
{
    uint16_t fval = htons (val);
    fwrite (&fval, 2, 1, f);
};

static inline void
log_put_uint32 (uint32_t val, FILE * f)
{
    uint32_t fval = htonl (val);
    fwrite (&fval, 4, 1, f);
}

static inline void
log_put_uint64 (uint64_t val, FILE * f)
{
    uint8_t b[8] = { val >> 56, val >> 48, val >> 40, val >> 32,
        val >> 24, val >> 16, val >> 8, val,
    };
    fwrite (b, 1, 8, f);
}

static inline void
log_put_field (uint16_t type, uint32_t length, FILE * f)
{
    log_put_uint16 (LOG_MARKER, f);
    log_put_uint16 (type, f);
    log_put_uint32 (length, f);
};

static inline int
log_get_uint16 (uint16_t * val, FILE * f)
{
    if (fread (val, 2, 1, f) != 1)
        return -1;
    *val = ntohs (*val);
    return 0;
};

static inline int
log_get_uint32 (uint32_t * val, FILE * f)
{
    if (fread (val, 4, 1, f) != 1)
        return -1;
    *val = ntohl (*val);
    return 0;
};

static inline int
log_get_uint64 (uint64_t * val, FILE * f)
{
    uint8_t b[8];
    if (fread (b, 1, 8, f) != 8)
        return -1;
    *val = ((uint64_t)b[0] << 56) |
        ((uint64_t)b[1] << 48) |
        ((uint64_t)b[2] << 40) |
        ((uint64_t)b[3] << 32) |
        ((uint64_t)b[4] << 24) |
        ((uint64_t)b[5] << 16) |
        ((uint64_t)b[6] << 8) |
        (uint64_t)b[7];
    return 0;
};

static inline int
log_get_next_field (uint16_t * type, uint32_t * length, FILE * f)
{
    uint16_t marker;
    if (log_get_uint16 (&marker, f) < 0)
        return -1;
    if (marker != LOG_MARKER) {
        fprintf (stderr, "Error: marker not found when reading log\n");
        return -1;
    }
    if (log_get_uint16 (type, f) < 0)
        return -1;
    if (log_get_uint32 (length, f) < 0)
        return -1;
    return 0;
};

static inline int
log_seek_to_field (FILE *f, uint16_t expected_type, uint32_t expected_length)
{
    uint16_t type;
    uint32_t length;
    while (0 == log_get_next_field (&type, &length, f)) {
        if (type == expected_type) {
            if (length == expected_length) return 0;
            else return -1;
        }
        fseeko (f, length, SEEK_CUR);
    }
    return -1;
}

/* Given that we are at any point in a camera log file, sync up to
 * the next field in the file by scanning for marker bytes and confirming
 * that valid data is present there. */
static int
log_resync (FILE * f)
{
    /* First, check if we are at a marker right now.  If so, assume
     * we are already synched. */
    uint16_t marker;
    if (log_get_uint16 (&marker, f) < 0)
        return -1;
    if (marker == LOG_MARKER) {
        fseeko (f, -2, SEEK_CUR);
        return 0;
    }

    int totalbytes = 0;
    uint8_t chunk[256];
    int offset = 0;
    while (1) {
        /* Read a chunk of bytes to scan for the marker */
        int len = fread (chunk + offset, 1, sizeof (chunk) - offset, f);
        if (len == 0)
            return -1;
        len += offset;

        int i;
        /* Look for the marker, and save the last 7 bytes for later
         * so a field won't cross a chunk boundary. */
        for (i = 0; i < len - 7; i++) {
            totalbytes++;
            /* Check for marker */
            if (chunk[i] != 0xED || chunk[i+1] != 0xED)
                continue;

            /* Make sure type is sane */
            uint16_t type = (chunk[i+2] << 8) | chunk[i+3];
            uint32_t length = (chunk[i+4] << 24) | (chunk[i+5] << 16) |
                (chunk[i+6] << 8) | chunk[i+7];
            if (type >= LOG_TYPE_MAX || type == 0)
                continue;
            
            /* Assume the length is correct, and seek to the next field. */
            off_t seekdist = (off_t)length - (off_t)(len-i-8);
            if (fseeko (f, seekdist, SEEK_CUR) < 0)
                return -1;

            /* Check for the presence of marker and type at next field */
            if (log_get_uint16 (&marker, f) < 0)
                return -1;
            if (log_get_uint16 (&type, f) < 0)
                return -1;
            if (marker == LOG_MARKER && type > 0 && type < 10) {
                /* Seek back to the start of the field */
                fseeko (f, -(off_t)length-12, SEEK_CUR);
                return 0;
            }
            /* Seek back to where the last chunk left off */
            fseeko (f, -seekdist-4, SEEK_CUR);
        }

        /* Copy any unscanned bytes at the end of the chunk to the
         * beginning of the next chunk. */
        memmove (chunk, chunk + i, len - i);
        offset = len - i;
    }
    return -1;
}
// =================================================

static camlog_version_t camlog_detect_version (Camlog *self);
static void legacy_compute_frame_spacing (Camlog *self); // legacy
static void find_last_frame_info (Camlog *self);
static int internal_peek_next_frame_info (Camlog *self, 
        camlog_frame_info_t *fmd);

Camlog* 
camlog_new (const char *fname, const char *mode)
{
    if (! strcmp (mode, "r") && ! strcmp (mode, "w")) {
        errl ("mode must be either 'w' or 'r'!!\n");
        return NULL;
    }

    Camlog *self = (Camlog*) calloc(1, sizeof(Camlog));

    struct stat statbuf;
    if (mode[0] == 'r') {
        if (stat (fname, &statbuf) < 0) {
            perror ("stat");
            errl ("Couldn't open [%s]\n", fname);
            camlog_destroy (self);
            return NULL;
        }
        self->mode = CAMLOG_MODE_READ;
    } else {
        self->mode = CAMLOG_MODE_WRITE;
    }

    self->fp = fopen(fname, mode);
    if (! self->fp) {
        perror ("fopen");
        errl ("Couldn't open [%s]\n", fname);
        camlog_destroy (self);
        return NULL;
    }

    self->prev_frame_offset = 0;
    self->frameno = 0;
    self->file_size = 0;

    if (self->mode == CAMLOG_MODE_READ) {
        self->file_size = statbuf.st_size;

        // determine the version of this logfile
        self->log_version = camlog_detect_version (self);
        dbgl ("log version: %d\n", self->log_version);

        if (self->log_version == CAMLOG_VERSION_INVALID) {
            errl ("Not a valid camera logfile:\n");
            errl ("      [%s]\n", fname);
            camlog_destroy (self);
            return NULL;
        }

        if (self->log_version == CAMLOG_VERSION_LEGACY) {
            legacy_compute_frame_spacing (self);
        }

        find_last_frame_info (self);
        fseeko (self->fp, 0, SEEK_SET);

        internal_peek_next_frame_info (self, &self->first_frame_info);
        memcpy (&self->next_frame_info, &self->first_frame_info,
                sizeof (camlog_frame_info_t));

        memset (&self->prev_frame_info, 0, sizeof (self->prev_frame_info));
    }

    return self;
}

void 
camlog_destroy (Camlog *self)
{
    if (self->fp) {
        fclose (self->fp);
    }
    memset (self,0,sizeof(Camlog));
    free (self);
}

int64_t camlog_get_file_size (const Camlog *self)
{
    return self->file_size;
}

// =============

static inline double
timestamp_to_offset_s (Camlog *self, int64_t timestamp)
{
    return (timestamp - self->first_frame_info.timestamp) * 1e-6;
}

static camlog_version_t
camlog_detect_version (Camlog *self)
{
    off_t fpos = ftello (self->fp);
    camlog_version_t version = CAMLOG_VERSION_INVALID;
    uint16_t ftype;
    uint32_t flen;
    while (!log_get_next_field (&ftype, &flen, self->fp)) {
        if (ftype == LOG_TYPE_FRAME_FORMAT) {
            version = CAMLOG_VERSION_LEGACY;
            break;
        } else if (ftype == LOG_TYPE_FRAME_INFO_0) {
            version = CAMLOG_VERSION_0;
            break;
        }
        fseeko(self->fp, flen, SEEK_CUR);
    }
    fseeko (self->fp, fpos, SEEK_SET);
    return version;
}

static int
camlog_get_frame_datalen (FILE *f, uint32_t *datalen)
{
    uint16_t type = 0;
    if (log_get_next_field (&type, datalen, f) != 0) return -1;
    if (type != LOG_TYPE_FRAME_DATA) return -1;

    return 0;
}

static int
camlog_get_frame_info_0 (Camlog *self, camlog_frame_info_t *fmd)
{
    uint16_t type = 0;
    uint32_t length;

    FILE *f = self->fp;
    fmd->frame_offset = ftello (f);
    if (log_get_next_field (&type, &length, f) != 0) goto fail;
    if (type != LOG_TYPE_FRAME_INFO_0 || length != LOG_FRAME_INFO_0_SIZE) 
        goto fail;

    if (log_get_uint16 (&fmd->width, f) != 0 ||
        log_get_uint16 (&fmd->height, f) != 0 ||
        log_get_uint16 (&fmd->stride, f) != 0 ||
        log_get_uint32 (&fmd->pixelformat, f) != 0 ||
        log_get_uint64 (&fmd->timestamp, f) != 0 ||
        log_get_uint32 (&fmd->bus_timestamp, f) != 0 ||
        log_get_uint64 (&fmd->source_uid, f) != 0 ||
        log_get_uint32 (&fmd->frameno, f) != 0 ||
        log_get_uint64 (&fmd->prev_frame_offset, f) != 0)
        goto fail;

    // haven't yet filled in fmd->datalen.  That gets filled in by whatever
    // function actually reads the frame data

    dbgl ("     (%dx%d %d %s fn %d tso %.3f)\n",
            fmd->width, fmd->height, fmd->stride,
            pixel_format_str (fmd->pixelformat), fmd->frameno,
            timestamp_to_offset_s (self, fmd->timestamp));
//    dbgl ("       (off %"PRId64" prev %"PRId64" ts %"PRId64")\n",
//            fmd->frame_offset, fmd->prev_frame_offset, fmd->timestamp);

    return 0;
fail:
    fseeko (f, fmd->frame_offset, SEEK_SET);
    return -1;
}

static int
camlog_get_legacy_frame_info (Camlog *self, camlog_frame_info_t *fmd)
{
    dbgl ("get_legacy_frame_info\n");
    FILE *f = self->fp;

    // read frame format
    if (0 != log_seek_to_field (f, LOG_TYPE_FRAME_FORMAT, 10)) return -1;
    if (log_get_uint16 (&fmd->width, f) != 0 ||
        log_get_uint16 (&fmd->height, f) != 0 ||
        log_get_uint16 (&fmd->stride, f) != 0 ||
        log_get_uint32 (&fmd->pixelformat, f) != 0) 
        return -1;

    int64_t frame_start_offset = ftello (f) - 18;

    // read frame timestamp
    if (0 != log_seek_to_field (f, LOG_TYPE_FRAME_TIMESTAMP, 12)) return -1;
    uint32_t sec, usec;
    if (log_get_uint32 (&sec, f) != 0 ||
        log_get_uint32 (&usec, f) != 0 ||
        log_get_uint32 (&fmd->bus_timestamp, f) != 0) 
        return -1;
    fmd->timestamp = (uint64_t) sec * 1000000 + usec;

    // read frame source uid
    uint16_t type = 0;
    uint32_t length;
    off_t fpos = ftello (f);
    if (log_get_next_field (&type, &length, f) != 0 ||
        type != LOG_TYPE_SOURCE_UID || length != 8 ||
        log_get_uint64 (&fmd->source_uid, f) != 0) {

        // it's okay if unable to read source_uid
        fseeko (f, fpos, SEEK_SET);
        fmd->source_uid = 0;
    }

    if (self->frame_spacing) {
        fmd->frameno = frame_start_offset / self->frame_spacing;
    } else {
        fmd->frameno = 0;
    }
    fmd->prev_frame_offset = frame_start_offset - self->frame_spacing;

    return 0;
}

static int
read_next_frame_info (Camlog *self, 
        camlog_frame_info_t *fmd)
{
    memset (fmd, 0, sizeof (camlog_frame_info_t));
    off_t fpos = ftello (self->fp);

    // try to read new format log files
    int status = camlog_get_frame_info_0 (self, fmd);
    if (0 != status) {
        // couldn't read new format log file.  try to read legacy log format
        if (0 != camlog_get_legacy_frame_info (self, fmd)) goto fail;
    }

    // both new and legacy log formats have the LOG_TYPE_FRAME_DATA field
    // try to read that now.
    if (0 != camlog_get_frame_datalen (self->fp, &fmd->datalen)) 
        goto fail;

//    if (&self->next_frame_info != fmd && 
//        self->next_frame_info.frameno != fmd->frameno) {
//        memcpy (&self->next_frame_info, fmd, sizeof (camlog_frame_info_t));
//    }

    // file pointer is now positioned at start of image data
    return 0;

fail:
    // on failure, return the file pointer to where it was on function call
    fseeko (self->fp, fpos, SEEK_SET);
    return -1;
}

static int 
internal_peek_next_frame_info (Camlog *self, camlog_frame_info_t *fmd)
{
    if (self->mode != CAMLOG_MODE_READ) return -1;

    off_t fpos = ftello (self->fp);
    int status = read_next_frame_info (self, fmd);
    fseeko (self->fp, fpos, SEEK_SET);
    return status;
}

int 
camlog_peek_next_frame_info (Camlog *self, camlog_frame_info_t *fmd)
{
    if (self->mode != CAMLOG_MODE_READ) return -1;
    memcpy (fmd, &self->next_frame_info, sizeof (self->next_frame_info));
    return (self->next_frame_info.timestamp > 0) ? 0 : -1;
}

int 
camlog_read_next_frame (Camlog *self, camlog_frame_info_t *fmd,
        uint8_t *buf, int buf_size)
{
    if (self->mode != CAMLOG_MODE_READ) return -1;

    dbgl ("reading frame %d (%.3f)\n", self->next_frame_info.frameno,
            timestamp_to_offset_s (self, self->next_frame_info.timestamp));

    off_t fpos = ftello (self->fp);
    int status = read_next_frame_info (self, fmd);
    if (0 != status) goto fail;

    if (fmd->datalen < buf_size)
        buf_size = fmd->datalen;

    if (1 != fread (buf, buf_size, 1, self->fp)) goto fail;

    if (buf_size < fmd->datalen)
        fseeko (self->fp, fmd->datalen - buf_size, SEEK_CUR);

    self->prev_frame_info = self->next_frame_info;
    internal_peek_next_frame_info (self, &self->next_frame_info);

    return 0;

fail:
    fseeko (self->fp, fpos, SEEK_SET);
    return -1;
}

int 
camlog_skip_next_frame (Camlog *self)
{
    if (self->mode != CAMLOG_MODE_READ) return -1;
    off_t fpos = ftello (self->fp);

    camlog_frame_info_t fmd;
    if (0 != read_next_frame_info (self, &fmd)) goto fail;
    if (0 != fseeko (self->fp, fmd.datalen, SEEK_CUR)) goto fail;

    self->prev_frame_info = self->next_frame_info;
    internal_peek_next_frame_info (self, &self->next_frame_info);

    return 0;
fail:
    fseeko (self->fp, fpos, SEEK_SET);
    return -1;
}

int 
camlog_seek_to_offset (Camlog *self, int64_t offset)
{
    if (self->mode != CAMLOG_MODE_READ) return -1;

    off_t fpos = ftello (self->fp);
    if (0 != fseeko (self->fp, offset, SEEK_SET)) goto fail;
    if (0 != log_resync (self->fp)) goto fail;

    // advance to the next frame format field
    uint16_t ftype;
    uint32_t flen;
    while (!log_get_next_field (&ftype, &flen, self->fp)) {
        if (ftype == LOG_TYPE_FRAME_FORMAT ||
            ftype == LOG_TYPE_FRAME_INFO_0) {
            fseeko (self->fp, -LOG_HEADER_SIZE, SEEK_CUR);
            off_t curpos = ftello (self->fp);
            
            internal_peek_next_frame_info (self, &self->next_frame_info);

            fseeko (self->fp, self->next_frame_info.prev_frame_offset, 
                    SEEK_SET);
            internal_peek_next_frame_info (self, &self->prev_frame_info);

            fseeko (self->fp, curpos, SEEK_SET);

            return 0;
        }
        fseeko(self->fp, flen, SEEK_CUR);
    }

fail:
    fseeko (self->fp, fpos, SEEK_SET);
    return -1;
}

int 
camlog_write_frame (Camlog *self, int width, int height, int stride,
        int pixelformat, uint64_t timestamp, int bus_timestamp,
        uint64_t source_uid,
        const uint8_t *data, int datalen, int64_t * file_offset)
{
    if (self->mode != CAMLOG_MODE_WRITE) return -1;
    int status;

    int64_t frame_start_offset = ftello (self->fp);
    if (file_offset) *file_offset = frame_start_offset;

    // write frame info
    log_put_field (LOG_TYPE_FRAME_INFO_0, LOG_FRAME_INFO_0_SIZE, 
            self->fp);
    log_put_uint16 (width, self->fp);
    log_put_uint16 (height, self->fp);
    log_put_uint16 (stride, self->fp);
    log_put_uint32 (pixelformat, self->fp);
    log_put_uint64 ((uint64_t) timestamp, self->fp);
    log_put_uint32 (bus_timestamp, self->fp);
    log_put_uint64 (source_uid, self->fp);
    log_put_uint32 (self->frameno, self->fp);
    log_put_uint64 (self->prev_frame_offset, self->fp);

    self->frameno ++;
    self->prev_frame_offset = frame_start_offset;

    // write frame data
    log_put_field (LOG_TYPE_FRAME_DATA, datalen, self->fp);
    status = fwrite (data, 1, datalen, self->fp);
    self->file_size = ftello (self->fp);

    return status != datalen;
}

int 
camlog_count_frames (Camlog *self)
{
    if (self->mode != CAMLOG_MODE_READ || 
        self->log_version == CAMLOG_VERSION_INVALID) return -1;

    if (self->log_version == CAMLOG_VERSION_LEGACY) {
        if (self->frame_spacing == 0) return -1;
        return self->file_size / self->frame_spacing;
    } else {
        assert (self->log_version == CAMLOG_VERSION_0);

        return self->last_frame_info.frameno;
    }
}

static int
do_seek_to_frame (Camlog *self, camlog_frame_info_t *low_frame,
        camlog_frame_info_t *high_frame, int desired_frameno)
{
    dbgl (" --- %d, %d, %d (%d) ---\n", 
            low_frame->frameno, desired_frameno, high_frame->frameno,
            self->next_frame_info.frameno);

    assert (low_frame->frame_offset >= 0 && 
            low_frame->frame_offset <= high_frame->frame_offset && 
            low_frame->frameno <= high_frame->frameno &&
            desired_frameno <= high_frame->frameno &&
            desired_frameno >= low_frame->frameno &&
            high_frame->frame_offset <= self->last_frame_info.frame_offset &&
            low_frame->frameno >= self->first_frame_info.frameno &&
            high_frame->frameno <= self->last_frame_info.frameno);

    // if we're within a few frames, just manually iterate to avoid the
    // resyncing process.
    while (desired_frameno > self->next_frame_info.frameno &&
           desired_frameno - self->next_frame_info.frameno < 20) {
        dbgl ("skip fwd\n");
        if (0 != camlog_skip_next_frame (self)) return -1;
    }
    while (self->next_frame_info.frameno > desired_frameno &&
           self->next_frame_info.frameno - desired_frameno < 20) {
        dbgl ("skip back\n");
        if (0 != camlog_seek_to_offset (self, 
                    self->next_frame_info.prev_frame_offset))
            return -1;
    }
    if (self->next_frame_info.frameno == desired_frameno) return 0;

    // make a best guess as to where the frame starts
    int nframes_spanned = high_frame->frameno - low_frame->frameno;
    int64_t nbytes_spanned = high_frame->frame_offset - low_frame->frame_offset;
    int64_t average_bytes_per_frame = nbytes_spanned / (int64_t)nframes_spanned;
    int64_t offset_guess = 
        (desired_frameno - low_frame->frameno) * average_bytes_per_frame + 
        low_frame->frame_offset;

    camlog_frame_info_t seeked_frame_info;
    camlog_seek_to_offset (self, offset_guess);
    internal_peek_next_frame_info (self, &seeked_frame_info);

    if (seeked_frame_info.frameno == desired_frameno) {
        return 0;
    }

    if (seeked_frame_info.frameno > desired_frameno) {
        return do_seek_to_frame (self, 
                low_frame, &seeked_frame_info, desired_frameno);
    } else if (desired_frameno > seeked_frame_info.frameno) {
        return do_seek_to_frame (self, 
                &seeked_frame_info, high_frame, desired_frameno);
    } 
   
    return 0;
}

int 
camlog_seek_to_frame (Camlog *self, int frameno)
{
    if (self->mode != CAMLOG_MODE_READ) return -1;

    if (self->log_version == CAMLOG_VERSION_LEGACY) {
        int64_t offset = (int64_t)self->frame_spacing * frameno;
        dbgl ("seeking to %"PRId64"\n", offset);
        return camlog_seek_to_offset (self, offset);
    } else {
        if (frameno < 0 || frameno > self->last_frame_info.frameno)
            return -1;

        return do_seek_to_frame (self, 
                &self->first_frame_info, &self->last_frame_info, frameno);
    }
}

static int
do_seek_to_timestamp (Camlog *self, camlog_frame_info_t *low_frame,
        camlog_frame_info_t *high_frame, int64_t desired_timestamp)
{
    if (self->next_frame_info.timestamp == desired_timestamp) return 0;

#ifdef USE_DBG
    double low_ts = timestamp_to_offset_s (self, low_frame->timestamp);
    double high_ts = timestamp_to_offset_s (self, high_frame->timestamp);
    double des_ts = timestamp_to_offset_s (self, desired_timestamp);
    double cur_ts = timestamp_to_offset_s (self, 
            self->next_frame_info.timestamp);
#endif

    dbgl (" --- %.3f, %.3f, %.3f (%.3f) ---\n", low_ts, des_ts, high_ts, 
            cur_ts);

    assert (low_frame->frame_offset >= 0 && 
            low_frame->frame_offset <= high_frame->frame_offset && 
            low_frame->timestamp <= high_frame->timestamp &&
            desired_timestamp <= high_frame->timestamp &&
            desired_timestamp >= low_frame->timestamp &&
            high_frame->frame_offset <= self->last_frame_info.frame_offset &&
            low_frame->timestamp >= self->first_frame_info.timestamp &&
            high_frame->timestamp <= self->last_frame_info.timestamp);

    int64_t spanned_usec = high_frame->timestamp - low_frame->timestamp;
    int spanned_frames = high_frame->frameno - low_frame->frameno;
    double average_usec_per_frame = spanned_usec / (double)spanned_frames;
    double usec_20frames = 20 * average_usec_per_frame;

    // if we're within a few frames, just manually iterate to avoid the
    // resyncing process.
    if (llabs (self->next_frame_info.timestamp - desired_timestamp) < 
            usec_20frames) {
        while (self->next_frame_info.timestamp > desired_timestamp) {
            dbgl ("skip back\n");
            if (0 != camlog_seek_to_offset (self, 
                        self->next_frame_info.prev_frame_offset))
                return -1;
        }
        dbgl ("  %.3f %.3f %.3f\n", 
                timestamp_to_offset_s (self, self->prev_frame_info.timestamp),
                timestamp_to_offset_s (self, desired_timestamp), 
                timestamp_to_offset_s (self, self->next_frame_info.timestamp));

        while (self->next_frame_info.timestamp < desired_timestamp) {
            dbgl ("skip fwd\n");
            if (0 != camlog_skip_next_frame (self)) return -1;
        }
        dbgl ("  %.3f %.3f %.3f\n", 
                timestamp_to_offset_s (self, self->prev_frame_info.timestamp),
                timestamp_to_offset_s (self, desired_timestamp), 
                timestamp_to_offset_s (self, self->next_frame_info.timestamp));

        assert (desired_timestamp > self->prev_frame_info.timestamp &&
                desired_timestamp <= self->next_frame_info.timestamp);
        return 0;
    }
    if (self->next_frame_info.timestamp == desired_timestamp) return 0;

    // make a best guess as to where the frame starts
    int64_t spanned_nbytes = high_frame->frame_offset - low_frame->frame_offset;
    double average_bytes_per_usec = spanned_nbytes / (double)spanned_usec;
    int64_t offset_guess = 
        (desired_timestamp - low_frame->timestamp) * average_bytes_per_usec + 
        low_frame->frame_offset;

    camlog_frame_info_t seeked_frame_info;
    camlog_seek_to_offset (self, offset_guess);
    internal_peek_next_frame_info (self, &seeked_frame_info);

    if (seeked_frame_info.timestamp == desired_timestamp) {
        return 0;
    }

    if (seeked_frame_info.timestamp > desired_timestamp) {
        return do_seek_to_timestamp (self, 
                low_frame, &seeked_frame_info, desired_timestamp);
    } else if (desired_timestamp > seeked_frame_info.timestamp) {
        return do_seek_to_timestamp (self, 
                &seeked_frame_info, high_frame, desired_timestamp);
    } 
   
    return 0;
}

int 
camlog_seek_to_timestamp (Camlog *self, int64_t timestamp)
{
    if (self->mode != CAMLOG_MODE_READ) return -1;

    if (timestamp < self->first_frame_info.timestamp || 
        timestamp > self->last_frame_info.timestamp)
        return -1;

    if (timestamp < self->next_frame_info.timestamp &&
        timestamp > self->prev_frame_info.timestamp) {
        dbgl ("already at %.3f (%"PRId64")\n", 
                timestamp_to_offset_s (self, timestamp), timestamp);
        return 0;
    } else {
        dbgl ("seek to %.3f (was %.3f, %3f)\n", 
                timestamp_to_offset_s (self, timestamp), 
                timestamp_to_offset_s (self, self->prev_frame_info.timestamp), 
                timestamp_to_offset_s (self, self->next_frame_info.timestamp)
                );
    }

    int status = do_seek_to_timestamp (self, 
            &self->first_frame_info, &self->last_frame_info, timestamp);

    if (0 == status) {
        dbgl ("seeked to frame %d time offset %.3f\n",
                self->next_frame_info.frameno,
                timestamp_to_offset_s (self, self->next_frame_info.timestamp));
    } else {
        dbgl ("couldn't seek to %.3f\n",
                timestamp_to_offset_s (self, self->next_frame_info.timestamp));
    }

    return status;
}

static void
find_last_frame_info (Camlog *self)
{
    int64_t search_inc = 5000000;

    for (int i=1; i < (self->file_size / search_inc) + 1 ; i++) {
        off_t offset = MAX (0, self->file_size - i * search_inc);

        if (0 == camlog_seek_to_offset (self, offset)) {
            camlog_frame_info_t fmd;
            while (0 == internal_peek_next_frame_info (self, &fmd)) {
                memcpy (&self->last_frame_info, &fmd, sizeof (fmd));
                camlog_skip_next_frame (self);
            }
            break;
        }
        if (0 == offset) break;
    }
    dbgl ("last frame offset: %"PRId64" timestamp: %"PRId64"\n",
            self->last_frame_info.frame_offset, 
            self->last_frame_info.timestamp);
    dbgl ("total frames: %d\n", self->last_frame_info.frameno);
}

// ========== legacy methods ==========

static void
legacy_compute_frame_spacing (Camlog *self)
{
    self->computed_frame_spacing = 1;
    self->frame_spacing = -1;
    off_t fpos = ftello (self->fp);
    fseeko (self->fp, 0, SEEK_SET);
    camlog_skip_next_frame (self);

    camlog_frame_info_t fmd;
    if (0 != internal_peek_next_frame_info (self, &fmd)) goto done;

    off_t fpos_first_frame = ftello (self->fp);

    if (! pixel_format_stride_meaningful (fmd.pixelformat)) {
        goto done;
    } else {
        camlog_skip_next_frame (self);
        if (0 != internal_peek_next_frame_info (self, &fmd)) goto done;

        off_t fpos_second_frame = ftello (self->fp);
        self->frame_spacing = fpos_second_frame - fpos_first_frame;
    }

done:
    fseeko (self->fp, fpos, SEEK_SET);
}
