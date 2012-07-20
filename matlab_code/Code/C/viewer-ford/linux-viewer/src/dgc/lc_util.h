#ifndef __lc_util_h__
#define __lc_util_h__

#include <glib.h>

#include <lc/lc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * lcu_mainloop_attach_lc (lc_t *lc)
 * attaches/detaches LC to/from the glib mainloop
 * When attached, lc_handle() is invoked "automatically" when a message is 
 * received over LC.
 *
 * only one instance of lc_t can be attached per process
 *
 * returns 0 on success, -1 on failure
 */
int lcu_glib_mainloop_attach_lc (lc_t *lc);

int lcu_glib_mainloop_detach_lc (lc_t *lc);

#ifdef __cplusplus
}
#endif

#endif
