#ifndef _TILE_SET_H
#define _TILE_SET_H

#include <GL/gl.h>

struct tile_set
{
    const char *channel;
    GPtrArray  *tiles;
    int64_t     receiving_generation;
    int64_t     drawing_generation;

    // for color mapping
    float      *r, *g, *b, *a;
    int        ncolors;
};

struct tile_texture 
{ 
    lcmtypes_gridmap_tile_t *tile;
    GLuint          texture_id;
    int             texture_valid;
};

struct tile_set *tile_set_create();
struct tile_set *tile_set_create_colormap(float *r, float *g, float *b, float *a, int ncolors);
void tile_set_process_new_tile(struct tile_set *ts, const lcmtypes_gridmap_tile_t *tile);
void tile_set_draw(struct tile_set *ts);

#endif

