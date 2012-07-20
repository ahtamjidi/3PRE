#ifndef _GRIDMAP_UTIL_H
#define _GRIDMAP_UTIL_H

#include <stdint.h>

#include <common/gridmap.h>

#include <lcmtypes/lcmtypes_gridmap_tile_t.h>

/** Given a current gridmap, import the data and realign the gridmap
    so that pos[] is near the center.  If the provided gridmap is NULL
    or is not compatible with the tile, a new gridmap is created (and
    the old destroyed). The gridmap is returned. 
**/
gridmap_t *gridmap_util_import_tile(gridmap_t *gm, const lcmtypes_gridmap_tile_t *tile, const double pos[3]);

void gridmap_util_publish(const gridmap_t *gm, lcm_t *lc, const char *channel, int64_t *generation, int64_t utime);

gridmap_u16_t *gridmap_util_import_tile_u16 (gridmap_u16_t *gm, 
        const lcmtypes_gridmap_tile_t *tile, const double pos[3]);

void gridmap_util_publish_u16 (const gridmap_u16_t *gm, lcm_t *lc, 
        const char *channel, int64_t *generation, int64_t utime);

#endif
