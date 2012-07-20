#ifndef _LCM_EVENTLOG_H_
#define _LCM_EVENTLOG_H_

#include <stdio.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _lcm_eventlog_t lcm_eventlog_t;
struct _lcm_eventlog_t
{
    FILE *f;
    int64_t eventcount;
};

typedef struct _lcm_eventlog_event_t lcm_eventlog_event_t;
struct _lcm_eventlog_event_t {
    int64_t eventnum, timestamp;
    int32_t channellen, datalen;

    char     *channel;
    void     *data;
};

// mode must be "r"
lcm_eventlog_t *lcm_eventlog_create(const char *path, const char *mode);

// when you're done with the log, clean up after yourself!
void lcm_eventlog_destroy(lcm_eventlog_t *l);

// read the next event.
lcm_eventlog_event_t *lcm_eventlog_read_next_event(lcm_eventlog_t *l);

// free the structure returned by lcm_eventlog_read_next_event
void lcm_eventlog_free_event(lcm_eventlog_event_t *le);

// seek (approximately) to particular timestamp
int lcm_eventlog_seek_to_timestamp(lcm_eventlog_t *l, int64_t ts);

#ifdef __cplusplus
}
#endif

#endif
