#include "ephemeris.h"

#include <common/timestamp.h>
#include <math.h>
#include <stdio.h>

#define ADJUST -5.2487   // new adjustment from m.b.

#define DAYS_PER_SECOND (1 / 86400.0)

// algorithm taken from Paul Schlyter
// http://stjarnhimlen.se/comp/ppcomp.html

typedef struct _ephemeris_t {
    // Input
    double time;           // from get_time function
    double latitude;       // degrees north
    double longitude;      // degrees east

    // Output
    double right_ascension; // radians
    double declination;     // radians
    double azimuth;         // radians east of north
    double altitude;        // radians above horizon
    double sun_position[3]; // unit vector pointing to sun
} ephemeris_t;

static int
get_ephemeris (ephemeris_t *ephem)
{
    double w, e, M, ecl;
    double E, xv, yv, v, r, lonsun, xs, ys, xe, ye, ze;
    double L, LST, GMST0, LHA, xhor, yhor, zhor, x, y, z;
    double sinAz, cosAz, sinAlt, cosAlt;

    ecl = 23.4393 - 3.563e-7*ephem->time;
    w = 282.9404 + 4.70935e-5*ephem->time;
    e = 0.016709 - 1.151e-9*ephem->time;
    M = 356.0470 + 0.9856002585*ephem->time;

    ecl *= M_PI/180.0;
    w *= M_PI/180.0;
    M *= M_PI/180.0;

    E = M + e*sin(M)*(1+e*cos(M));
    xv = cos(E) - e;
    yv = sqrt(1.0-e*e)*sin(E);
    v = atan2(yv, xv);
    r = sqrt(xv*xv + yv*yv);

    lonsun = v + w;
    xs = r*cos(lonsun);
    ys = r*sin(lonsun);

    xe = xs;
    ye = ys*cos(ecl);
    ze = ys*sin(ecl);

    ephem->right_ascension = atan2(ye, xe);
    ephem->declination = atan2(ze, sqrt(xe*xe + ye*ye));

    L = M + w;
    GMST0 = L + M_PI;
    LST = GMST0 + ephem->time*2*M_PI + ephem->longitude*M_PI/180;
    LHA = LST - ephem->right_ascension;

    x = cos(LHA)*cos(ephem->declination);
    y = sin(LHA)*cos(ephem->declination);
    z = sin(ephem->declination);

    xhor = x*sin(ephem->latitude*M_PI/180) - z*cos(ephem->latitude*M_PI/180);
    yhor = y;
    zhor = x*cos(ephem->latitude*M_PI/180) + z*sin(ephem->latitude*M_PI/180);

    ephem->azimuth = atan2(yhor, xhor) + M_PI;
    ephem->altitude = asin(zhor);

    // TODO: is this adjustment really necessary?
    //  ephem->azimuth += ADJUST*M_PI/180;

    sinAz = sin(ephem->azimuth);
    cosAz = cos(ephem->azimuth);
    sinAlt = sin(ephem->altitude);
    cosAlt = cos(ephem->altitude);
    ephem->sun_position[0] = cosAlt*cosAz;
    ephem->sun_position[1] = -cosAlt*sinAz;
    ephem->sun_position[2] = sinAlt;

    return 0;
}

int solar_ephemeris_get_sun_vector (const double lat, const double lon,
        const uint64_t utime, double direction[3])
{
    // reference time is 00:00 Jan 1, 2000 UTC
    int64_t ref_time = 946684800;

    // Get solar ephemeris
    ephemeris_t ephem;
    ephem.latitude = lat;
    ephem.longitude = lon;
    ephem.time = (utime * 1e-6 - ref_time) * DAYS_PER_SECOND;
    get_ephemeris (&ephem);

    // Convert from NEU to ENU
    direction[0] = -ephem.sun_position[1];
    direction[1] = ephem.sun_position[0];
    direction[2] = ephem.sun_position[2];

    return 0;
}
