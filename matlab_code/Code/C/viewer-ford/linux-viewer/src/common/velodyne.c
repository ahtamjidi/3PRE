#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <common/math_util.h>

#include "velodyne.h"

#define UPPER_MAGIC 0xeeff
#define LOWER_MAGIC 0xddff

//#include "velodyne-uncalib.h"
#include "velodyne-newunit.h"

#define RADIANS_PER_LSB 0.00017453293
//#define METERS_PER_LSB 0.00194805
//#define METERS_PER_LSB 0.0019
#define METERS_PER_LSB 0.002

// return an upper bound on the # of samples in this message
int velodyne_decoder_estimate_samples(velodyne_calib_t *v, const void *_data, int datalen)
{
    return (datalen / 3) +1;
}

int velodyne_decoder_init(velodyne_calib_t *v, velodyne_decoder_t *vd, const void *_data, int datalen)
{
    vd->data    = (uint8_t*) _data;
    vd->data_remaining = datalen;
    vd->i       = 0;

    if (datalen != 1206) {
        printf("velodyne: bad len %i\n", datalen);
        vd->data_remaining = 0; // don't decode it.
        return -1;
    }

    // copy out the version string
    if (vd->data[1202]=='v') {
      vd->revolution_count = vd->data[1200] + (vd->data[1201]<<8);
      memcpy(vd->version_string, &vd->data[1202], 4);
      vd->version_string[4]=0;
    } else {
      vd->version_string[0]=0;
      vd->revolution_count=-1;
    }

    /*    printf("velodyne: ");
    for (int i = 1200; i < 1206; i++) {
      printf("%02x ", vd->data[i]);

    printf("\n");
      }*/

    return 0;
}

int velodyne_decoder_next(velodyne_calib_t *v, velodyne_decoder_t *vd, velodyne_sample_t *sample)
{
    // if we finished the last block, "consume" this block of data.
    if (vd->i == 32) {
        vd->data_remaining -= 100;
        vd->data += 100;
        vd->i = 0;
    }

    uint8_t *data = vd->data;

    // starting a new block? 
    if (vd->i == 0) {
        // enough data for another block?
        if (vd->data_remaining < 100)
            return -1;

        int magic = data[0] + (data[1]<<8);

        if (magic == UPPER_MAGIC) 
            vd->laser_offset = 32;
        else if (magic == LOWER_MAGIC)
            vd->laser_offset = 0;
        else {
            printf("Unknown velodyne magic %4x\n", magic);
            return -2;
        }
      
        // position of velodyne head, constant for all 32 measurements that follow
        vd->ctheta = 2*M_PI - (data[2] + (data[3]<<8)) * RADIANS_PER_LSB;
        if (vd->ctheta == 2*M_PI)
            vd->ctheta = 0;
        
        fasttrig_sincos(vd->ctheta, &vd->sin_ctheta, &vd->cos_ctheta);
        vd->i = 0;
    }

    // Decode the laser sample
    int      i       = vd->i;
    
    sample->physical = vd->laser_offset + i;
    sample->logical  = velodyne_physical_to_logical(v, sample->physical);

    struct velodyne_laser_calib *params = &v->lasers[sample->physical];

    sample->raw_range = (data[4 + i*3] + (data[5+i*3]<<8)) * METERS_PER_LSB;
    sample->range     = (sample->raw_range + params->range_offset) * (1.0 + params->range_scale_offset);
    sample->ctheta    = vd->ctheta;
    sample->theta     = mod2pi_ref(M_PI, vd->ctheta + params->rcf);
    sample->phi       = params->vcf;
    sample->intensity = data[6 + i*3]/255.0;

    double sin_theta, cos_theta;
    fasttrig_sincos(sample->theta, &sin_theta, &cos_theta);
    double sin_phi = v->sincos[sample->physical][0];
    double cos_phi = v->sincos[sample->physical][1];

    sample->xyz[0] = sample->range * cos_theta * cos_phi;
    sample->xyz[1] = sample->range * sin_theta * cos_phi;
    sample->xyz[2] = sample->range * sin_phi;
  
    // handle horizontal offset ("parallax")
    sample->xyz[0] -= params->hcf * vd->cos_ctheta;
    sample->xyz[1] -= params->hcf * vd->sin_ctheta;
    vd->i++;

    // successful decode
    return 0;
}

int velodyne_decode(velodyne_calib_t *v, const void *_data, int datalen, 
                    double *_theta0,
                    double *ranges, double *intensities, double *thetas, double *phis,
                    int *laserids, int *nsamples, int *badscans)
{
    if (datalen != 1206) {
        printf("velodyne: bad len %i\n", datalen);
        return -1;
    }

    int out_idx = 0;
    int laser_offset = 0;

    for (uint8_t *data = (uint8_t*) _data; datalen >= 100; datalen-=100, data += 100) {

        int magic = data[0] + (data[1]<<8);

        if (magic == UPPER_MAGIC) 
            laser_offset = 32;
        else if (magic == LOWER_MAGIC)
            laser_offset = 0;
        else {
            printf("Unknown velodyne magic %4x\n", magic);
            continue;
        }

        double theta0 = 2*M_PI - (data[2] + (data[3]<<8)) * RADIANS_PER_LSB;
	*_theta0 = theta0;

        for (int i = 0; i < 32; i++) {
            struct velodyne_laser_calib *params = &v->lasers[laser_offset + i];
            
            ranges[out_idx] = (data[4 + i*3] + (data[5+i*3]<<8)) * METERS_PER_LSB;
            ranges[out_idx] *= (1.0 + params->range_scale_offset);
            ranges[out_idx] -= params->range_offset;
            
            // skip illegally short ranges
            if (ranges[out_idx] < 0.5) {
                badscans[laser_offset+i]++;
                continue;
            }

            intensities[out_idx] = data[6 + i*3]/255.0;
            thetas[out_idx] = theta0 + params->rcf;
            phis[out_idx] = params->vcf;
            laserids[out_idx] = laser_offset + i;
            out_idx++;
        }
    }

    *nsamples = out_idx;
    // last six bytes are status. we don't know what to do with them.

    return 0;
}

void velodyne_calib_dump(velodyne_calib_t *v)
{
    printf("struct velodyne_laser_calib velodyne_NAME_ME_HERE[] = {\n");
    for (int i = 0; i < VELODYNE_NUM_LASERS; i++) {
        struct velodyne_laser_calib *params = &v->lasers[i];
        printf("   { %11.7f, %11.7f, %8.4f, %8.4f, %10.6f }, // laser %2d\n", 
	       params->rcf, params->vcf, params->hcf, params->range_offset, params->range_scale_offset, i);
    }
    printf("};\n\n");
}


static velodyne_calib_t *__v;
static int laser_phi_compare(const void *_a, const void *_b)
{
    int a = *((int*) _a);
    int b = *((int*) _b);

    if (__v->lasers[a].vcf < __v->lasers[b].vcf) 
        return -1;
    return 1;
}

// NOT REENTRANT
int velodyne_calib_precompute(velodyne_calib_t *v)
{
    assert (!__v); // check for reentrancy...

    __v = v;

    for (int i = 0; i < VELODYNE_NUM_LASERS; i++)
        v->logical2physical[i] = i;
    qsort(v->logical2physical, VELODYNE_NUM_LASERS, sizeof(int), laser_phi_compare);
    
    for (int logical = 0; logical < VELODYNE_NUM_LASERS; logical++) {
        v->physical2logical[v->logical2physical[logical]] = logical;
    }

    for (int physical = 0; physical < VELODYNE_NUM_LASERS; physical++) {
        sincos(v->lasers[physical].vcf, &v->sincos[physical][0], & v->sincos[physical][1]);
    }
    __v = NULL;

    return 0;
}

velodyne_calib_t *velodyne_calib_create()
{
    velodyne_calib_t *v = (velodyne_calib_t*) calloc(1, sizeof(velodyne_calib_t));
    memcpy(v->lasers, velodyne_uncalibrated, sizeof(struct velodyne_laser_calib) * VELODYNE_NUM_LASERS);
    velodyne_calib_precompute(v);

    return v;
}
