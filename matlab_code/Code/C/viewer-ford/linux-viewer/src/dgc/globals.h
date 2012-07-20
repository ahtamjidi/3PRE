#ifndef __globals_manager_h__
#define __globals_manager_h__

#include <glib.h>
#include <lcm/lcm.h>
#include <common/config.h>
#include <common/rndf.h>
#include "ctrans.h"
#include "lcgl.h"

#ifdef __cplusplus
extern "C" {
#endif

Config * globals_get_config ();
void globals_release_config (Config *config);

lcm_t * globals_get_lcm ();
void globals_release_lcm (lcm_t *lcm);

CTrans * globals_get_ctrans ();
void globals_release_ctrans (CTrans *ctrans);

RndfRouteNetwork * globals_get_rndf ();
void globals_release_rndf (RndfRouteNetwork *rndf);

lcgl_t * globals_get_lcgl (const char *name, const int create_if_missing);
//lcgl_t * globals_get_lcgl (const char *name);
// needed to retrieve all lcgls for switch_buffer
GHashTable *globals_get_lcgl_hashtable ();


#ifdef __cplusplus
}
#endif

#endif
