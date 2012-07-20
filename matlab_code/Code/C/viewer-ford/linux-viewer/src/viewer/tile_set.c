#include <glib.h>

#include <lcmtypes/lcmtypes_gridmap_tile_t.h>
#include <common/gridmap.h>
#include <common/glib_util.h>
#include <common/math_util.h>

#include "tile_set.h"

struct tile_texture *tile_texture_create(const lcmtypes_gridmap_tile_t *tile)
{
    struct tile_texture *tt = (struct tile_texture*) calloc(1, sizeof(struct tile_texture));

    tt->tile = lcmtypes_gridmap_tile_t_copy(tile);
    tt->texture_valid = 0;
    return tt;
}

void tile_texture_destroy(struct tile_texture *tt)
{
    if (tt->texture_valid)
        glDeleteTextures(1, &tt->texture_id);

    lcmtypes_gridmap_tile_t_destroy(tt->tile);
    free(tt);
}

void tile_set_process_new_tile(struct tile_set *ts, const lcmtypes_gridmap_tile_t *tile)
{
    struct tile_texture *tt = tile_texture_create(tile);
    
    if (ts->receiving_generation != tt->tile->generation) {
        ts->drawing_generation = ts->receiving_generation;
        ts->receiving_generation = tt->tile->generation;
    }

    for (int i = 0; i < g_ptr_array_size(ts->tiles); i++) {
        struct tile_texture *tt0 = g_ptr_array_index(ts->tiles, i);

        if (tt0->tile->generation != ts->receiving_generation && 
            tt0->tile->generation != ts->drawing_generation) {
            tile_texture_destroy(tt0);
            g_ptr_array_remove_index_fast(ts->tiles, i);
            i--;
        }
    }

    g_ptr_array_add(ts->tiles, tt);
}

void tile_set_draw_tile(struct tile_set *ts, struct tile_texture *tt)
{
    if (!tt->texture_valid) {
        glGenTextures( 1, &tt->texture_id );
        tt->texture_valid = 1;

        glPixelTransferi(GL_INDEX_SHIFT, 0);
        glPixelTransferi(GL_INDEX_OFFSET, 0);
        glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
      
        uint8_t *data = (uint8_t*) malloc(tt->tile->width * tt->tile->height);
        
        gridmap_decode_base(data, tt->tile->width, tt->tile->height, tt->tile->data, tt->tile->datalen);
        
        glPixelStorei (GL_UNPACK_ROW_LENGTH, tt->tile->width);
        glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
        glBindTexture (GL_TEXTURE_2D, tt->texture_id);
        glTexImage2D (GL_TEXTURE_2D, 
                      0, 
                      GL_RGBA8, 
                      tt->tile->width, tt->tile->height, 
                      0, 
                      GL_COLOR_INDEX, 
                      GL_UNSIGNED_BYTE,
                      data);
        
        glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);
        glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
        glBindTexture (GL_TEXTURE_2D, 0);

        free(data);
    }

    glPushAttrib (GL_ENABLE_BIT);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable (GL_DEPTH_TEST);
    glEnable (GL_TEXTURE_2D);
    
    if (ts->r && ts->g && ts->b && ts->a) {
        glPixelTransferi(GL_INDEX_SHIFT, 0);
        glPixelTransferi(GL_INDEX_OFFSET, 0);
        glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
        
        glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 256, ts->r);
        glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 256, ts->g);
        glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 256, ts->b);
        glPixelMapfv(GL_PIXEL_MAP_I_TO_A, 256, ts->a);
    }

    glPixelStorei (GL_UNPACK_ROW_LENGTH, tt->tile->width);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glBindTexture (GL_TEXTURE_2D, tt->texture_id);
    
    glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                    GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
                    GL_NEAREST);
    
    double x0 = tt->tile->x0;
    double x1 = tt->tile->x0 + tt->tile->width * tt->tile->meters_per_pixel;
    double y0 = tt->tile->y0;
    double y1 = tt->tile->y0 + tt->tile->height * tt->tile->meters_per_pixel;
    
    int d = 1;
    glBegin (GL_QUADS);
    glTexCoord2i (0,0);
    glVertex2d (x0, y0);
    glTexCoord2i (0, d);
    glVertex2d (x0, y1);
    glTexCoord2i (d, d);
    glVertex2d (x1, y1);
    glTexCoord2i (d, 0);
    glVertex2d (x1, y0);
    glEnd ();

    glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
    glBindTexture (GL_TEXTURE_2D, 0);
    glPopAttrib ();
}

struct tile_set *tile_set_create()
{
    return tile_set_create_colormap(NULL, NULL, NULL, NULL, 0);
}

struct tile_set *tile_set_create_colormap(float *r, float *g, float *b, float *a, int ncolors)
{
    struct tile_set *ts = (struct tile_set*) calloc(1, sizeof(struct tile_set));
    ts->tiles = g_ptr_array_new();
    ts->drawing_generation = -100;
    ts->ncolors = ncolors;
    ts->r = r;
    ts->g = g;
    ts->b = b;
    ts->a = a;

    return ts;
}

void tile_set_draw(struct tile_set *ts)
{
    if (!ts)
        return;

    for (int tidx = 0; tidx < g_ptr_array_size(ts->tiles); tidx++) {
        struct tile_texture *tt = g_ptr_array_index(ts->tiles, tidx);

        if (tt->tile->generation == ts->drawing_generation)
            tile_set_draw_tile(ts, tt);
    }
}

