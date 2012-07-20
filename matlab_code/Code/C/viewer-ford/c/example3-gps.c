// file: example3-gps.c
//
// Given a log file, print out the GPS positions along with the timestamp at
// each position.  
//
// Each line of the output consists of four numbers:
//  timestamp latitude longitude elevation theta
//
// Timestamps are given in microseconds since 00:00:00 January 1, 1970 UTC.  
// Latitude and Longitude are referenced to WGS84.
// Elevation is given in meters above sea level.
// Theta is given in radians, and measures the angle between the vehicle's
// forward direction and magnetic north.


#include <stdio.h>
#include <string.h>

#include "eventlog.h"
#include "lcmtypes_gps_to_local_t.h"

int main(int argc, char **argv)
{
    if(argc < 2) {
        fprintf(stderr, "usage: example3-gps <logfile>\n");
        return 1;
    }

    lcm_eventlog_t *log = lcm_eventlog_create(argv[1], "r");
    if(!log) {
        fprintf(stderr, "error opening log file\n");
        return 1;
    }

    lcm_eventlog_event_t *event = lcm_eventlog_read_next_event(log);
    while(event) {
        if(!strcmp(event->channel, "GPS_TO_LOCAL")) {
            lcmtypes_gps_to_local_t gps;
            lcmtypes_gps_to_local_t_decode(event->data, 0, event->datalen, 
                    &gps);

            printf("%lld %.10f %.10f %f %f\n", 
                    (long long)gps.utime, 
                    gps.lat_lon_el_theta[0], 
                    gps.lat_lon_el_theta[1], 
                    gps.lat_lon_el_theta[2],
                    gps.lat_lon_el_theta[3]); 

            lcmtypes_gps_to_local_t_decode_cleanup(&gps);
        }

        lcm_eventlog_free_event(event);
        event = lcm_eventlog_read_next_event(log);
    }

    lcm_eventlog_destroy(log);

    return 0;
}
