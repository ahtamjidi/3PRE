#ifndef __dgc_ephemeris_h__
#define __dgc_ephemeris_h__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Calculates the 3D vector to the sun in east/north/up coordinates.
 * latitude and longitude are specified in degrees.
 * timestamp is a typical dgc timestamp (in usec).
 */
int solar_ephemeris_get_sun_vector (const double lat, const double lon,
        const uint64_t utime, double direction[3]);

#ifdef __cplusplus
}
#endif
#endif
