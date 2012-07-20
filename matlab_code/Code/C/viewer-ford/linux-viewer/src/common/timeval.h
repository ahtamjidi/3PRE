#ifndef __lc_timeval_h__
#define __lc_timeval_h__

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// returns:    1      a > b
//            -1      a < b
//             0      a == b
static inline int
timeval_compare (const struct timeval *a, const struct timeval *b) {
    if (a->tv_sec == b->tv_sec && a->tv_usec == b->tv_usec) return 0;
    if (a->tv_sec > b->tv_sec || 
            (a->tv_sec == b->tv_sec && a->tv_usec > b->tv_usec)) 
        return 1;
    return -1;
}

static inline void
timeval_add (const struct timeval *a, const struct timeval *b, 
        struct timeval *dest) 
{
    dest->tv_sec = a->tv_sec + b->tv_sec;
    dest->tv_usec = a->tv_usec + b->tv_usec;
    if (dest->tv_usec > 999999) {
        dest->tv_usec -= 1000000;
        dest->tv_sec++;
    }
}

static inline void
timeval_subtract (const struct timeval *a, const struct timeval *b,
        struct timeval *dest)
{
    dest->tv_sec = a->tv_sec - b->tv_sec;
    dest->tv_usec = a->tv_usec - b->tv_usec;
    if (dest->tv_usec < 0) {
        dest->tv_usec += 1000000;
        dest->tv_sec--;
    }
}

#ifdef __cplusplus
}
#endif

#endif
