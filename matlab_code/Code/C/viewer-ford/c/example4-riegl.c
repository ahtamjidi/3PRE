// file: example5-sick.c
//
// Given a log file, print out the laser range data from the SICK LIDAR
// sensors.

#include <stdio.h>
#include <string.h>

#include "eventlog.h"
#include "lcmtypes_laser_t.h"

int main(int argc, char **argv)
{
    if(argc < 2) {
        fprintf(stderr, "usage: example5-sick <logfile>\n");
        return 1;
    }

    lcm_eventlog_t *log = lcm_eventlog_create(argv[1], "r");
    if(!log) {
        fprintf(stderr, "error opening log file\n");
        return 1;
    }

    lcm_eventlog_event_t *event = lcm_eventlog_read_next_event(log);
    while(event) {
        if(!strncmp(event->channel, "BROOM_LEFT", 5) || 
           !strncmp(event->channel, "BROOM_RIGHT", 5)) {
            lcmtypes_laser_t msg;
            int i;
            lcmtypes_laser_t_decode(event->data, 0, event->datalen, &msg);

            printf("%lld %s %f %f %d %d", 
                    (long long)msg.utime, 
                    event->channel, 
                    msg.rad0,
                    msg.radstep,
                    msg.nranges, msg.nintensities);
            for(i=0; i<msg.nranges; i++) {
                printf(" %f", msg.ranges[i]);
            }
            for(i=0; i<msg.nintensities; i++) {
                printf(" %f", msg.intensities[i]);
            }
            printf("\n");

            lcmtypes_laser_t_decode_cleanup(&msg);
        }

        lcm_eventlog_free_event(event);
        event = lcm_eventlog_read_next_event(log);
    }

    lcm_eventlog_destroy(log);

    return 0;
}
