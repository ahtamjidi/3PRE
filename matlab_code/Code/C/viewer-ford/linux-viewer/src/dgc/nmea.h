#ifndef _NMEA_H
#define _NMEA_H

#include <stdint.h>
#include <lcm/lcm.h>
#include <lcmtypes/lcmtypes_nmea_t.h>

#define GPS_MAX_SATS 52

#define GPS_STATUS_ERROR 0
#define GPS_STATUS_NO_LOCK 1
#define GPS_STATUS_LOCK 2
#define GPS_STATUS_DGPS_LOCK 3

typedef struct _gps_lcmtypes_nmea_t gps_lcmtypes_nmea_t;
typedef struct _gps_state_t gps_state_t;

struct _gps_state_t
{
    //uint8_t is_valid;

    uint8_t utc_hours;
    uint8_t utc_minutes;
    float utc_seconds;

    int64_t recv_utime;  // local utime at which the state was measured

    double lat;
    double lon;
    double elev;

    double ve, vn, vu; // velocities

#if 0
    double err_horiz;
    double err_vert;
    double err_pos;

    double sat_elevation[GPS_MAX_SATS], sat_elevation_b[GPS_MAX_SATS];
    double sat_azimuth[GPS_MAX_SATS], sat_azimuth_b[GPS_MAX_SATS];
    double sat_snr[GPS_MAX_SATS], sat_snr_b[GPS_MAX_SATS]; // set to -1 for satellite unseen
    int    sats_visible;
#endif

    int    status;
    //int    messagecount;
};

typedef int (*gps_state_handler_t)(gps_lcmtypes_nmea_t *, gps_state_t *, void *);

struct _gps_lcmtypes_nmea_t
{
    gps_state_t         state;
    gps_state_t         state_pending;
    gps_state_handler_t handler;
    lcmtypes_nmea_t_subscription_t    *nhid;
    void *              userdata;
};

gps_lcmtypes_nmea_t * nmea_new (void);
int nmea_subscribe (gps_lcmtypes_nmea_t * gn, lcm_t * lc, const char * channel,
        gps_state_handler_t handler, void * userdata);
int nmea_unsubscribe (gps_lcmtypes_nmea_t * gn, lcm_t * lc, const char * channel,
        gps_state_handler_t handler, void * userdata);
void nmea_free (gps_lcmtypes_nmea_t * gn);
void nmea_state_clear (gps_state_t * gd);
int nmea_parse (gps_state_t * gd, const lcmtypes_nmea_t *_nmea);




#define NMEA_SENTENCE_MAXLEN 512

//#define GPS_TIMEOUT -1
//#define GPS_INVALID -2


// ================== GGA ===========

typedef enum {
    NMEA_GGA_NO_FIX = 0,
    NMEA_GGA_FIX = 1,
    NMEA_GGA_FIX_DIFFERENTIAL = 2,
    NMEA_GGA_ESTIMATED = 6
} nmea_gga_fix_quality_t;

/*
 * nmea_gpgga_t encodes all the useful information presented in a GGA
 * sentence
 */
typedef struct nmea_gpgga {
    uint8_t utc_hours;
    uint8_t utc_minutes;
    float utc_seconds;

    double lat;                   // negative is south, positive is north
    double lon;                   // negative is west, positive is east
    nmea_gga_fix_quality_t fix;   // GPS quality indication
    uint8_t num_satellites;       // 0 - 12
    double horizontal_dilution;   // horiz dilution of precision 0.5 - 99.9
    double antenna_height;        // -9999.9 - 99999.9 meters
    double geoidal_height;        // -999.9 - 9999.9 meters
} nmea_gpgga_t;

// ================== RMC ===========

typedef enum {
    NMEA_RMC_STATUS_INVALID = 0,
    NMEA_RMC_STATUS_VALID = 1,
    NMEA_RMC_STATUS_VALID_DIFF = 2,
    NMEA_RMC_STATUS_ESTIMATED = 3,
} nmea_rmc_status_t;

/*
 * nmea_gprmc_t encodes all the useful information presented in a RMC
 * message
 */
typedef struct nmea_gprmc {
    uint8_t utc_hours;
    uint8_t utc_minutes;
    float utc_seconds;

    double lat;                   // negative is south, positive is north
    double lon;                   // negative is west, positive is east
    nmea_rmc_status_t status;     // GPS quality indication

    double speed_knots;
    double heading_deg;
} nmea_gprmc_t;

// ================  GSV ===============

typedef struct nmea_gpgsv {
    uint8_t num_sentences;        // total number of GSV sentences transmitting
    uint8_t sentence;             // number of current GSV sentence
    uint8_t in_view;              // 0-12

    uint8_t num_sats_here;        // 1-4  (number of satellites reported in 
                                  //       this particular instance)
    uint8_t prn[4];               // 1-32 (satellite ID)
    uint8_t elevation[4];         // 0-90 degrees
    uint8_t azimuth[4];           // 0-359 degrees
    uint8_t snr[4];               // signal to noise ratio (0-99 dB)
} nmea_gpgsv_t;

// ================ GLL ================

#define NMEA_GLL_STATUS_VALID 'A'
#define NMEA_GLL_STATUS_WARNING 'V'

#define NMEA_GLL_MODE_AUTONOMOUS 'A'
#define NMEA_GLL_MODE_DIFFERENTIAL 'D'
#define NMEA_GLL_MODE_ESTIMATED 'E'
#define NMEA_GLL_MODE_INVALID 'N'

typedef struct nmea_gll {
    double lat;             // degrees.  negative is south, positive is north
    double lon;             // degrees.  negative is west, positive is east
    uint8_t utc_hours;
    uint8_t utc_minutes;
    float utc_seconds;
    char status;
    char mode;
} nmea_gpgll_t;

// =============== PGRMV ================
// garmin-proprietary velocity data

typedef struct nmea_pgrmv {
    double vel_east;          // meters/second [ -514.4, 514.4 ]
    double vel_north;         // ''
    double vel_up;            // ''
} nmea_pgrmv_t;

// =============== PGRME ================
// garmin-proprietary estimated error

typedef struct nmea_pgrme {
    double err_horizontal;
    double err_vertical;
    double err_position;
} nmea_pgrme_t;

// =========

typedef enum {
    NMEA_TYPE_UNRECOGNIZED,

    NMEA_TYPE_GPRMC,             // GPS recommended minimum specific data
    NMEA_TYPE_GPGGA,             // GPS system fix data
    NMEA_TYPE_GPGSV,             // GPS satellites in view
    NMEA_TYPE_GPGLL,             // GPS geographic position

    NMEA_TYPE_PGRMV,             // garmin proprietary 3D velocity 
    NMEA_TYPE_PGRME,             // garmin proprietary estimated error
} nmea_sentence_type_t;

typedef struct nmea_sentence {
    union {
        // GPS standard sentences
        nmea_gpgga_t gpgga;
        nmea_gpgsv_t gpgsv;
        nmea_gpgll_t gpgll;
        nmea_gprmc_t gprmc;

        // garmin proprietary sentences
        nmea_pgrmv_t pgrmv;
        nmea_pgrme_t pgrme;

        // unrecognized
        char raw[NMEA_SENTENCE_MAXLEN];
    } data;

    nmea_sentence_type_t type;
} nmea_sentence_t;


// returns 0 if the sentence is parsed successfully.
//        -1 if the sentence is not a valid NMEA sentence
int nmea_parse_sentence( const char *sentence, nmea_sentence_t *p );

#endif
