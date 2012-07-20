// file: example1-poses.c
//
// Given a log file, print out the 3D rotation rate, acceleration, velocity and gps data 
// coming from mtig along with the timestamp at each position.  Timestamps are given in 
// microseconds since 00:00:00 January 1, 1970 UTC.  Positions are given in meters.

#include <stdio.h>
#include <string.h>

#include "eventlog.h"
#include "lcmtypes_mtig_t.h"

int main(int argc, char **argv)
{
    if(argc < 2) {
        fprintf(stderr, "usage: example2-mtig-poses <logfile>\n");
        return 1;
    }

    lcm_eventlog_t *log = lcm_eventlog_create(argv[1], "r");
    if(!log) {
        fprintf(stderr, "error opening log file\n");
        return 1;
    }

    printf("# utime x y z x' y' z'\n");
    lcm_eventlog_event_t *event = lcm_eventlog_read_next_event(log);
    while(event) {
        if(!strcmp(event->channel, "MTIG")) {
            lcmtypes_mtig_t pose;
            lcmtypes_mtig_t_decode(event->data, 0, event->datalen, &pose);

            printf("%lld %f %f %f %f %f %f %f %f %f %f %f %f %f\n", 
                    (long long)pose.ts_unix, 
                    pose.eul_roll, 
                    pose.eul_pitch, 
                    pose.eul_yaw,
                    pose.acc_x,
                    pose.acc_y,
                    pose.acc_z,
                    pose.vel_n,
                    pose.vel_e,
                    pose.vel_d,
                    pose.pos_lat,
                    pose.pos_lon,
                    pose.pos_alt);

            lcmtypes_mtig_t_decode_cleanup(&pose);
        }

        lcm_eventlog_free_event(event);
        event = lcm_eventlog_read_next_event(log);
    }

    lcm_eventlog_destroy(log);

    return 0;
}
