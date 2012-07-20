#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

#include "nmea.h"

/** does string s2 begin with s1? **/
static
int strpcmp(const char *s1, const char *s2)
{
	return strncmp(s1, s2, strlen(s1));
}

//static double
//pdouble (char *tok, double prev)
//{
//    if (tok[0]==0)
//        return prev;
//
//    return strtod(tok, NULL);
//}

// returns 1 for north/east, -1 for south/west
static double
nsew (char a)
{
    char c = toupper(a);
    if (c=='W' || c=='S')
        return -1;
    return 1;
}

gps_lcmtypes_nmea_t *
nmea_new (void)
{
    gps_lcmtypes_nmea_t * gn;

    gn = malloc (sizeof (gps_lcmtypes_nmea_t));
    memset (gn, 0, sizeof (gps_lcmtypes_nmea_t));

    return gn;
}

void
nmea_free (gps_lcmtypes_nmea_t * gn)
{
    free (gn);
}

#define GPS_MESSAGE_MAXLEN 512

#if 0
void
nmea_state_clear (gps_state_t * gd)
{
    int i;
    gd->is_valid = 0;
    for (i = 0; i < GPS_MAX_SATS; i++)
        gd->sat_snr[i] = -1;
}
#endif

static int
nmea_handler (const char * msgtype, const lcmtypes_nmea_t * nmea, gps_lcmtypes_nmea_t * gn)
{
    if (nmea_parse (&gn->state_pending, nmea)) {
        memcpy (&gn->state, &gn->state_pending, sizeof (gps_state_t));
        if (gn->handler)
            gn->handler (gn, &gn->state, gn->userdata);
    }
    return 0;
}

int
nmea_subscribe (gps_lcmtypes_nmea_t * gn, lcm_t * lc, const char * channel,
        gps_state_handler_t handler, void * userdata)
{
    gn->handler = handler;
    gn->userdata = userdata;
    gn->nhid = lcmtypes_nmea_t_subscribe (lc, channel,
            (lcmtypes_nmea_t_handler_t) nmea_handler, gn);
    return gn->nhid ? 0 : -1;
}

int
nmea_unsubscribe (gps_lcmtypes_nmea_t * gn, lcm_t * lc, const char * channel,
        gps_state_handler_t handler, void * userdata)
{
    return lcmtypes_nmea_t_unsubscribe (lc, gn->nhid);
}

int
nmea_parse (gps_state_t *gd, const lcmtypes_nmea_t *_nmea)
{
    nmea_sentence_t n;
    memset( &n, 0, sizeof(n) );

    if( 0 != nmea_parse_sentence( _nmea->nmea, &n ) ) return 0;

    switch( n.type ) {
        case NMEA_TYPE_GPRMC:
            gd->recv_utime = _nmea->utime;
            gd->utc_hours   = n.data.gprmc.utc_hours;
            gd->utc_minutes = n.data.gprmc.utc_minutes;
            gd->utc_seconds = n.data.gprmc.utc_seconds;

            gd->lat = n.data.gprmc.lat;
            gd->lon = n.data.gprmc.lon;
            gd->elev = 0;

            switch (n.data.gprmc.status) {
                case NMEA_RMC_STATUS_VALID:
                    gd->status = GPS_STATUS_LOCK;
                    break;
                case NMEA_RMC_STATUS_VALID_DIFF:
                    gd->status = GPS_STATUS_DGPS_LOCK;
                    break;
                default:
                    gd->status = GPS_STATUS_NO_LOCK;
                    break;
            }

            double speed_ms = n.data.gprmc.speed_knots * 0.514444444;
            gd->vn = speed_ms * cos (n.data.gprmc.heading_deg * M_PI / 180);
            gd->ve = speed_ms * sin (n.data.gprmc.heading_deg * M_PI / 180);
            gd->vu = 0;

            return 1;

#if 0
        /* Don't use GGA anymore, it doesn't have velocity */
        case NMEA_TYPE_GPGGA:
            gd->recv_utime = _nmea->utime;
            gd->utc_hours   = n.data.gpgga.utc_hours;
            gd->utc_minutes = n.data.gpgga.utc_minutes;
            gd->utc_seconds = n.data.gpgga.utc_seconds;
            gd->lat = n.data.gpgga.lat;
            gd->lon = n.data.gpgga.lon;
            gd->elev = n.data.gpgga.antenna_height;
            switch (n.data.gpgga.fix) {
                case NMEA_GGA_FIX:
                    gd->status = GPS_STATUS_LOCK;
                    break;
                case NMEA_GGA_FIX_DIFFERENTIAL:
                    gd->status = GPS_STATUS_DGPS_LOCK;
                    break;
                default: 
                    gd->status = GPS_STATUS_NO_LOCK;
                    break;
            }
            return 1;
#endif
        default:
            break;
    }
    return 0;
}

int nmea_parse_sentence( const char *sentence, nmea_sentence_t *p )
{
    char nmea_cpy[NMEA_SENTENCE_MAXLEN+1];
    strncpy(nmea_cpy, sentence, NMEA_SENTENCE_MAXLEN);
    char *nmea;

    // get rid of any extra stuff at the beginning
    nmea = strchr(nmea_cpy, '$');
    if (nmea == NULL)
        return -1;

    // tokenize and verify checksum
    char *toks[100];
    int  ntoks = 0;
    int  pos = 0;
    int checksum = 0;
    while (nmea[pos]!=0 && ntoks < 100) {
        if (nmea[pos]=='*') {
            char *endptr = NULL;
            int tocheck = strtol( nmea+pos+1, &endptr, 16 );

            if( endptr == nmea+pos+1 || tocheck != checksum ) return -1;

            nmea[pos]=0;
            break;
        }

        if( pos > 0) checksum ^= nmea[pos];

        if (nmea[pos]==',') {
            nmea[pos] = 0;
            toks[ntoks++]=&nmea[pos+1];
        }
        pos++;
    }

    memset( p, 0, sizeof(nmea_sentence_t) );
    p->type = NMEA_TYPE_UNRECOGNIZED;

    if (!strpcmp("$GPRMC", nmea)) {
        p->type = NMEA_TYPE_GPRMC;
        nmea_gprmc_t *rmc = &p->data.gprmc;

        rmc->utc_hours   = (toks[0][0]-'0') * 10 + (toks[0][1]-'0') * 1;
        rmc->utc_minutes = (toks[0][2]-'0') * 10 + (toks[0][3]-'0') * 1;
        rmc->utc_seconds = strtod(toks[0] + 4, NULL);

        switch (toks[1][0]) {
            case 'A':
                rmc->status = NMEA_RMC_STATUS_VALID;
                break;
            case 'V':
                rmc->status = NMEA_RMC_STATUS_INVALID;
                break;
            default:
                fprintf (stderr, "Invalid status '%c' in NMEA string GPRMC\n",
                        toks[1][0]);
                rmc->status = NMEA_RMC_STATUS_INVALID;
                break;
        }

        rmc->lat = ( (toks[2][0] - '0') * 10 + (toks[2][1] - '0') + 
            strtod(toks[2] + 2, NULL) / 60.0 ) * nsew( toks[3][0] );
        rmc->lon = ( (toks[4][0] - '0') * 100 + 
                     (toks[4][1] - '0') * 10 + 
                     (toks[4][2] - '0') * 1 + 
            strtod(toks[4] + 3, NULL) / 60.0 ) * nsew( toks[5][0] );

        rmc->speed_knots = strtod (toks[6], NULL);
        rmc->heading_deg = strtod (toks[7], NULL);

        if (ntoks >= 12) {
            switch (toks[11][0]) {
                case 'A':
                    rmc->status = NMEA_RMC_STATUS_VALID;
                    break;
                case 'D':
                    rmc->status = NMEA_RMC_STATUS_VALID_DIFF;
                    break;
                case 'E':
                    rmc->status = NMEA_RMC_STATUS_ESTIMATED;
                    break;
                case 'N':
                case '\0':
                    rmc->status = NMEA_RMC_STATUS_INVALID;
                    break;
                default:
                    fprintf (stderr, "Invalid mode '%c' in NMEA string GPRMC\n",
                            toks[11][0]);
                    rmc->status = NMEA_RMC_STATUS_INVALID;
                    break;
            }
        }
    }
    else if (!strpcmp("$GPGGA", nmea) && ntoks==14) { // GPS system fix data
        p->type = NMEA_TYPE_GPGGA;
        nmea_gpgga_t *gga = &p->data.gpgga;

        gga->utc_hours   = (toks[0][0]-'0') * 10 + (toks[0][1]-'0') * 1;
        gga->utc_minutes = (toks[0][2]-'0') * 10 + (toks[0][3]-'0') * 1;
        gga->utc_seconds = strtod(toks[0] + 4, NULL);

        gga->lat = ( (toks[1][0] - '0') * 10 + (toks[1][1] - '0') + 
            strtod(toks[1] + 2, NULL) / 60.0 ) * nsew( toks[2][0] );
        gga->lon = ( (toks[3][0] - '0') * 100 + 
                     (toks[3][1] - '0') * 10 + 
                     (toks[3][2] - '0') * 1 + 
            strtod(toks[3] + 3, NULL) / 60.0 ) * nsew( toks[4][0] );

        int qual = atoi(toks[5]);
        switch (qual)
        {
            case 1: gga->fix = NMEA_GGA_FIX; break;
            case 2: gga->fix = NMEA_GGA_FIX_DIFFERENTIAL; break;
            case 6: gga->fix = NMEA_GGA_ESTIMATED; break;
            default: 
            case 0: gga->fix = NMEA_GGA_NO_FIX; break;
        }

        gga->num_satellites = atoi( toks[6] );
        gga->horizontal_dilution = strtod( toks[7], NULL );
        gga->antenna_height = strtod( toks[8], NULL );
        gga->geoidal_height = strtod( toks[10], NULL );
    } else if (!strpcmp("$GPGSV", nmea) && ntoks>2) { // satellites in view
        p->type = NMEA_TYPE_GPGSV; 
        nmea_gpgsv_t *gsv = &p->data.gpgsv;

        gsv->num_sentences = atoi(toks[0]);
        gsv->sentence = atoi(toks[1]);
        gsv->in_view = atoi(toks[2]);

        gsv->num_sats_here = (ntoks-3)/4;
        int i;
        for( i=0; i<gsv->num_sats_here; i++ ) {
            gsv->prn[i] = atoi( toks[4*i+3] );
            gsv->elevation[i] = strtod( toks[4*i+4], NULL );
            gsv->azimuth[i] = strtod( toks[4*i+5], NULL );
            gsv->snr[i] = strtod( toks[4*i+6], NULL );
        }
    } else if (!strpcmp("$PGRMV", nmea) && ntoks >= 3) { // 3D velocity info
        p->type = NMEA_TYPE_PGRMV;
        p->data.pgrmv.vel_east  = strtod(toks[0], NULL);
        p->data.pgrmv.vel_north = strtod(toks[1], NULL);
        p->data.pgrmv.vel_up    = strtod(toks[2], NULL);
    } else if (!strpcmp("$PGRME", nmea)) { // estimated error information
        p->type = NMEA_TYPE_PGRME;
        p->data.pgrme.err_horizontal = strtod(toks[0], NULL);
        p->data.pgrme.err_vertical   = strtod(toks[1], NULL);
        p->data.pgrme.err_position   = strtod(toks[2], NULL);
    }
#if 0
    else if (!strpcmp("$GPGLL", nmea)) { // geographic position
        p->type = NMEA_TYPE_GPGLL;
        nmea_gpgll_t *gll = &p->data.gpgll;

        gll->lat = ( (toks[0][0] - '0') * 10 + (toks[0][1] - '0') + 
            strtod(toks[0] + 2, NULL) / 60.0 ) * nsew( toks[1][0] );
        gll->lon = ( (toks[2][0] - '0') * 100 + 
                     (toks[2][1] - '0') * 10 + 
                     (toks[2][2] - '0') * 1 + 
            strtod(toks[2] + 3, NULL) / 60.0 ) * nsew( toks[3][0] );

        gll->utc_hours   = (toks[4][0]-'0') * 10 + (toks[4][1]-'0') * 1;
        gll->utc_minutes = (toks[4][2]-'0') * 10 + (toks[4][3]-'0') * 1;
        gll->utc_seconds = strtod(toks[4] + 4, NULL);

        //gll->status = toks[5][0];
        //gll->mode   = toks[6][0];
#endif
    else {
        p->type = NMEA_TYPE_UNRECOGNIZED;
        strncpy( p->data.raw, sentence, sizeof(p->data.raw) );
    }
//    else if (!strpcmp("$GPALM", nmea)) {  // almanac data
//    }
//    else if (!strpcmp("$GPGSA", nmea)) { // DOP and active satellites
//    }
//    else if (!strpcmp("$GPVTG", nmea)) { // track made good and ground speed
//    }
//    else if (!strpcmp("$PGRMF", nmea)) { // fix data 
//    }
//    else if (!strpcmp("$PGRMT", nmea)) { // sensor status information (GARMIN)
//    }
//    else if (!strpcmp("$PGRMB", nmea)) { // dgps beacon info
//    }
//    else if (!strpcmp("$PGRMM", nmea)) { // ??? e.g. "$PGRMM,WGS 84*06"
//    }
    return 0;
}
