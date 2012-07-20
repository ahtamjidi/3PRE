#ifndef _VELODYNE_H
#define _VELODYNE_H

#include <stdint.h>
#include "common/fasttrig.h"

/** Lasers 0-31: lower lasers
          32-63: upper lasers 
**/

#define VELODYNE_NUM_LASERS 64

typedef struct velodyne_decoder velodyne_decoder_t;
struct velodyne_decoder
{
    uint8_t *data;
    uint8_t *p;
    int      data_remaining;

    int      i; // which laser within the current block are we processing?
    int      laser_offset;
    double   ctheta, sin_ctheta, cos_ctheta;

    int32_t revolution_count;
    char    version_string[16];
};

typedef struct velodyne_sample velodyne_sample_t;
struct velodyne_sample
{
    double  xyz[3];            // calibrated, projected into velodyne coordinate system
    double  raw_range;         // raw return directly from sensor
    double  range;             // corrected range
    double  ctheta;            // theta of sensor head at time of sample, **always [0, 2*PI)**
    double  theta;             // calibrated theta (horizontal/yaw angle) **always [0, 2*PI)**
    double  phi;               // calibrated phi (veritcle/pitch angle)
    double  intensity;         // normalized intensity [0, 1]
    int     physical;          // physical laser number (0-31 lower, 32-63 upper)
    int     logical;           // logical laser number (in order of increasing pitch)
};

struct velodyne_laser_calib
{
    double rcf;                // radians (rotational/yaw offset)
    double vcf;                // radians (vertical offset)
    double hcf;                // meters (horizontal off-axis offset)
    double range_offset;       // meters
    double range_scale_offset; // (scalar)
};

typedef struct 
{
    struct velodyne_laser_calib lasers[VELODYNE_NUM_LASERS]; // physical idx
    int physical2logical[VELODYNE_NUM_LASERS];
    int logical2physical[VELODYNE_NUM_LASERS];
    double sincos[VELODYNE_NUM_LASERS][2];
} velodyne_calib_t;

void velodyne_dump_calib(velodyne_calib_t *v);


// Velodyne lidars are numbered in two different ways:
// Physical: The order given by the hardware.
// Logical: In order of increasing phi

velodyne_calib_t *velodyne_calib_create();

// NOT REENTRANT. 
// Compute the logical laser numbers given the current phi angles
// (necessary before calls to p2l or l2p)
int velodyne_calib_precompute(velodyne_calib_t *v);

static inline int velodyne_physical_to_logical(velodyne_calib_t *v, int phys)
{
    return v->physical2logical[phys];
}

static inline int velodyne_logical_to_physical(velodyne_calib_t *v, int logical)
{
    return v->logical2physical[logical];
}

// return an upper bound on the # of samples in this message
int velodyne_decoder_estimate_samples(velodyne_calib_t *v, const void *_data, int datalen);

int velodyne_decoder_init(velodyne_calib_t *v, velodyne_decoder_t *vd, const void *_data, int datalen);
int velodyne_decoder_next(velodyne_calib_t *v, velodyne_decoder_t *vd, struct velodyne_sample *sample);

#endif
