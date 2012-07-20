#ifndef __camlog_h__
#define __camlog_h__

#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Camlog Camlog;

typedef struct _camlog_frame_info {
    uint16_t width;
    uint16_t height;
    uint16_t stride;
    uint32_t pixelformat;

    uint64_t timestamp;
    uint32_t bus_timestamp;

    uint64_t source_uid;

    uint32_t datalen;

    uint32_t frameno;

    uint64_t frame_offset;
    uint64_t prev_frame_offset;
} camlog_frame_info_t;

/**
 * camlog_new:
 * @fname: the file to read or create
 * @mode:  either "r" or "w"
 *
 * constructor
 */
Camlog* camlog_new (const char *fname, const char *mode);

void camlog_destroy (Camlog *self);

/**
 * camlog_peek_next_frame_info:
 *
 * Retrieves the metadata associated with the next frame, without actually
 * advancing the current position within the log.
 *
 * Read-mode only.
 *
 * Returns: 0 on success, -1 on failure.  In either case, the file pointer of
 * the log file is not changed.
 */
int camlog_peek_next_frame_info (Camlog *self, 
        camlog_frame_info_t *frame_info);

/**
 * camlog_read_next_frame:
 *
 * Retrieves the next frame and associated metadata.  Advances the file pointer
 * of the logfile.
 *
 * Read-mode only.
 *
 * Returns: 0 on success, -1 on file error, EOF, or if the buffer %buf is not
 * large enough to hold the entire frame.
 */
int camlog_read_next_frame (Camlog *self, camlog_frame_info_t *frame_info,
        uint8_t *buf, int buf_size);

/**
 * camlog_skip_next_frame:
 *
 * Advances the file pointer past the next frame.
 *
 * Read-mode only.
 *
 * Returns: 0 on success, -1 on file error or EOF.
 */
int camlog_skip_next_frame (Camlog *self);

/**
 * camlog_write_frame:
 * @width:          width of the image
 * @height:         size of the image
 * @stride:         distance, in bytes, between rows of the image
 * @pixelformat:    see pixels.h
 * @timestamp:      time, in microseconds since the epoch, of the frame
 * @bus_timestamp:  only useful if you're david
 * @source_uid:     identifier for the source of the image
 * @data:           pointer to the actual image data
 * @datalen:        size, in bytes, of the image data
 * @file_offset:    output parameter.  The file offset of the start of the
 *                  frame is stored here.
 *
 * Writes a frame to disk.
 *
 * Returns: 0 on success, -1 on failure
 */
int camlog_write_frame (Camlog *self, int width, int height, int stride,
        int pixelformat, uint64_t timestamp, int bus_timestamp,
        uint64_t source_uid,
        const uint8_t *data, int datalen, int64_t * file_offset);

/**
 * camlog_count_frames:
 *
 * Returns: the total number of frames in the logfile
 */
int camlog_count_frames (Camlog *self);

/**
 * camlog_seek_to_frame:
 *
 * Positions the file pointer of the camlog such that the next call to
 * camlog_read_next_frame reads frame number %frameno
 *
 * Read-mode only.
 *
 * Returns: 0 on success, -1 on failure
 */
int camlog_seek_to_frame (Camlog *self, int frameno);

/**
 * camlog_seek_to_offset:
 *
 * Positions the file pointer of the camlog at the start of the first frame
 * with file offset greater than or equal to %file_offset
 *
 * Read-mode only.
 *
 * Returns: 0 on success, -1 on failure
 */
int camlog_seek_to_offset (Camlog *self, int64_t file_offset);

/**
 * camlog_seek_to_timestamp:
 *
 * Positions the file pointer of the camlog at the start of the first frame
 * with timestamp greater or equal to than %timestamp
 *
 * Read-mode only.
 *
 * Returns: 0 on success, -1 on failure
 */
int camlog_seek_to_timestamp (Camlog *self, int64_t timestamp);

/**
 * camlog_get_file_size:
 *
 * Returns: the size of the camera log file.
 */
int64_t camlog_get_file_size (const Camlog *self);

#ifdef __cplusplus
}
#endif

#endif
