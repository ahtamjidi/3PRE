#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "gridmap.h"
#include "geometry.h"

#include "math_util.h"

#define MAX_RUN_LENGTH 0xffff
#define EOF_LEN 6

gridmap_lut_t *gridmap_lut_create_constant(int table_size, double max_dist, int inside_value, int outside_value)
{
    gridmap_lut_t *lut = (gridmap_lut_t*) calloc(1, sizeof(gridmap_lut_t));
    lut->table = malloc(table_size * sizeof(int));
    lut->size = table_size;
    lut->max_dist = max_dist;
    lut->max_dist_sq = sq(max_dist);
    lut->max_dist_sq_inv = 1.0 / lut->max_dist_sq;
    
    for (int i = 0; i < table_size; i++) {
        lut->table[i] = inside_value;
    }
    lut->table[table_size-1] = outside_value;

    return lut;
}

void gridmap_nearest_bucket_center(gridmap_t *gm, double x, double y, double *ox, double *oy)
{
    int ix, iy;
    gridmap_to_ix_iy(gm, x, y, &ix, &iy);
    *ox = gm->x0 + (ix+0.5)*gm->meters_per_pixel;
    *oy = gm->y0 + (iy+0.5)*gm->meters_per_pixel;
}

gridmap_lut_t *gridmap_lut_create_cliff(int table_size, double max_dist, double cliff_distance, double decay, int max_value)
{
    gridmap_lut_t *lut = (gridmap_lut_t*) calloc(1, sizeof(gridmap_lut_t));
    lut->table = malloc(table_size * sizeof(int));
    lut->size = table_size;
    lut->max_dist = max_dist;
    lut->max_dist_sq = sq(max_dist);
    lut->max_dist_sq_inv = 1.0 / lut->max_dist_sq;
    assert(decay <= 0);

    //  idx = dist^2 * table_size / table_max_dist^2;
    for (int i = 0; i < table_size; i++) {
        double dist = sqrt( i * lut->max_dist_sq / table_size);
        int v;

        if (dist < cliff_distance)
            v = max_value;
        else if (dist <= max_dist)
            v = max_value*exp(decay*(dist - cliff_distance));
        else
            v = 0;

        lut->table[i] = v;
    }
    // clamp end of table to zero in case expotential doesn't quite get there.
    // (you get low cost regions out to the bounding boxes otherwise)
    if (table_size&&(lut->table[table_size-1]<8)) {
        lut->table[table_size-1]=0;
    }
    return lut;
}

// like the above, except that distances less than restricted_distance will have the restricted bit set.
gridmap_lut_t *gridmap_lut_create_cliff_restricted(int table_size, double max_dist, double cliff_distance, double restricted_distance, double decay, int max_value, int outside_value)
{
    gridmap_lut_t *lut = (gridmap_lut_t*) calloc(1, sizeof(gridmap_lut_t));
    lut->table = malloc(table_size * sizeof(int));
    lut->size = table_size;
    lut->max_dist = max_dist;
    lut->max_dist_sq = sq(max_dist);
    lut->max_dist_sq_inv = 1.0 / lut->max_dist_sq;
    assert(decay <= 0);

    //  idx = dist^2 * table_size / table_max_dist^2;
    for (int i = 0; i < table_size; i++) {
        double dist = sqrt( i * lut->max_dist_sq / table_size);
        int v;

        if (dist < cliff_distance)
            v = max_value;
        else if (dist <= max_dist)
            v = max_value*exp(decay*(dist - cliff_distance));
        else
            v = 0;

        v &= 0xfe; // clear low bit
        if (dist < restricted_distance)
            v |= 1;

        lut->table[i] = v;
    }
    // clamp end of table to zero in case expotential doesn't quite get there.
    // (you get low cost regions out to the bounding boxes otherwise).
    lut->table[table_size-1]=outside_value;

    return lut;
}

// like the above, except that distances less than restricted_distance will have the restricted bit set.
gridmap_lut_t *gridmap_lut_create_cliff_restricted_linear(int table_size, double max_dist, double cliff_distance, double restricted_distance, int max_value, int outside_value)
{
    gridmap_lut_t *lut = (gridmap_lut_t*) calloc(1, sizeof(gridmap_lut_t));
    lut->table = malloc(table_size * sizeof(int));
    lut->size = table_size;
    lut->max_dist = max_dist;
    lut->max_dist_sq = sq(max_dist);
    lut->max_dist_sq_inv = 1.0 / lut->max_dist_sq;

    //  idx = dist^2 * table_size / table_max_dist^2;
    for (int i = 0; i < table_size; i++) {
        double dist = sqrt( i * lut->max_dist_sq / table_size);
        int v;

        if (dist < cliff_distance)
            v = max_value;
        else if (dist <= max_dist)
            v = max_value * (1 - (dist-cliff_distance)/(max_dist-cliff_distance));
        else
            v = 0;

        v &= 0xfe; // clear low bit
        if (dist < restricted_distance)
            v |= 1;

        lut->table[i] = v;
    }
    // clamp end of table to zero in case expotential doesn't quite get there.
    // (you get low cost regions out to the bounding boxes otherwise).
    lut->table[table_size-1]=outside_value;

    return lut;
}

gridmap_lut_t *gridmap_lut_create_cliff_restricted_cos(int table_size, double max_dist, double cliff_distance, double restricted_distance, 
                                                       double max_phase, int max_value, int outside_value)
{
    gridmap_lut_t *lut = (gridmap_lut_t*) calloc(1, sizeof(gridmap_lut_t));
    lut->table = malloc(table_size * sizeof(int));
    lut->size = table_size;
    lut->max_dist = max_dist;
    lut->max_dist_sq = sq(max_dist);
    lut->max_dist_sq_inv = 1.0 / lut->max_dist_sq;

    //  idx = dist^2 * table_size / table_max_dist^2;
    for (int i = 0; i < table_size; i++) {
        double dist = sqrt( i * lut->max_dist_sq / table_size);
        int v;

        if (dist < cliff_distance)
            v = max_value;
        else if (dist <= max_dist) {
            double pdist = dist - cliff_distance;
            double ptot = max_dist - cliff_distance;
            v = max_value * cos(max_phase * pdist / ptot);
        } else
            v = 0;

        v &= 0xfe; // clear low bit
        if (dist < restricted_distance)
            v |= 1;

        lut->table[i] = v;
    }
    // clamp end of table to zero in case expotential doesn't quite get there.
    // (you get low cost regions out to the bounding boxes otherwise).
    lut->table[table_size-1]=outside_value;

    return lut;
}

// cost is zero for < cliff_distance, increases afterwards.
// 
gridmap_lut_t *gridmap_lut_create_cliff_restricted_increasing(int table_size, double max_dist, double cliff_distance, double restricted_distance, double decay, 
                                                              int min_value, int max_value, int outside_value)
{
    gridmap_lut_t *lut = (gridmap_lut_t*) calloc(1, sizeof(gridmap_lut_t));
    lut->table = malloc(table_size * sizeof(int));
    lut->size = table_size;
    lut->max_dist = max_dist;
    lut->max_dist_sq = sq(max_dist);
    lut->max_dist_sq_inv = 1.0 / lut->max_dist_sq;
    assert(decay <= 0);

    //  idx = dist^2 * table_size / table_max_dist^2;
    for (int i = 0; i < table_size; i++) {
        double dist = sqrt( i * lut->max_dist_sq / table_size);
        int v;

        if (dist < cliff_distance)
            v = min_value;
        else if (dist <= max_dist)
            v = min_value + (max_value-min_value)*(dist - cliff_distance)/(max_dist - cliff_distance);
        else
            v = max_value;

        v = imin(max_value, v);

        v &= 0xfe; // clear low bit
        if (dist > restricted_distance)
            v |= 1;

        lut->table[i] = v;
    }

    lut->table[table_size-1] = outside_value;
    return lut;
}

void gridmap_lut_destroy(gridmap_lut_t *lut)
{
    free(lut->table);
    lut->table = NULL;
    free(lut);
}

#define DEFINE_GRIDMAP_ALIGN(suffix, maptype)               \
    static void gridmap_align_xy_ ## suffix (maptype *gm)   \
    {                                                       \
        /* round cx,cy to multiple of meters_per_pixel */   \
        int64_t tmp;                                        \
        tmp = round(gm->x0 / gm->meters_per_pixel);         \
        gm->x0 = tmp * gm->meters_per_pixel;                \
                                                            \
        tmp = round(gm->y0 / gm->meters_per_pixel);         \
        gm->y0 = tmp * gm->meters_per_pixel;                \
                                                            \
        /* recompute cx correctly. */                       \
        gm->cx = gm->x0 + gm->sizex/2;                      \
        gm->cy = gm->y0 + gm->sizey/2;                      \
    }

DEFINE_GRIDMAP_ALIGN (u8, gridmap_t)
DEFINE_GRIDMAP_ALIGN (u16, gridmap_u16_t)

int gridmap_render_rectangle_aligned(gridmap_t *gm, 
                                     double cx, double cy, 
                                     double x_size, double y_size, 
                                     const gridmap_lut_t *lut);

// The cell at (0,0) represents the space from (cx,cy) to (cx+meters_per_pixel, cy+meters_per_pixel) 
// i.e., the center of the cell is at (cx+meters_per_pixel/2, cy+meters_per_pixel/2)
//
// Note: we move (x0,y0) to the nearest multiple of meters_per_pixel. This ensures
// that as we import/export tiles, that different gridmaps with the same resolution can
// import/export without any interpolation.
//
gridmap_t *gridmap_create(double cx, double cy, double sizex, double sizey, double meters_per_pixel)
{
    return gridmap_create_fill(cx, cy, sizex, sizey, meters_per_pixel, 0);
}

gridmap_t *gridmap_create_fill(double cx, double cy, double sizex, double sizey, double meters_per_pixel, int fill)
{
    gridmap_t *gm = (gridmap_t*) calloc(1, sizeof(gridmap_t));

    gm->meters_per_pixel = meters_per_pixel;
    gm->pixels_per_meter = 1.0/gm->meters_per_pixel;

    // compute pixel dimensions and allocate
    gm->width = sizex/meters_per_pixel;
    gm->height = sizey/meters_per_pixel;
    gm->data = (uint8_t*) calloc(1, gm->width * gm->height);

    // recompute sizex/sizey using exact numbers (due to rounding above)
    gm->sizex = gm->width * meters_per_pixel;
    gm->sizey = gm->height * meters_per_pixel;

    gm->x0 = cx - sizex/2;
    gm->y0 = cy - sizey/2;

    gridmap_align_xy_u8 (gm);

    gridmap_fill(gm, fill);

    return gm;
}

void gridmap_fill(gridmap_t *gm, int fill)
{
    memset(gm->data, fill, gm->width * gm->height);
}

gridmap_t *gridmap_create_compatible(const gridmap_t *gm)
{
    return gridmap_create(gm->cx, gm->cy, gm->sizex, gm->sizey, gm->meters_per_pixel);
}

gridmap_t *
gridmap_new_copy (const gridmap_t *gm)
{
    gridmap_t *self = gridmap_create_compatible (gm);
    memcpy (self->data, gm->data, gm->width * gm->height);
    return self;
}

void gridmap_reset(gridmap_t *gm, double cx, double cy)
{
    gm->x0 = cx - gm->sizex/2;
    gm->y0 = cy - gm->sizey/2;

    gridmap_align_xy_u8 (gm);

    memset(gm->data, 0, gm->width*gm->height);
}

void gridmap_destroy(gridmap_t *gm)
{
    free(gm->data);
    free(gm);
}

// find the worst cost and the most restrictive value of the
// restricted flag along the given line. The distance is NOT
// integrated.
void gridmap_test_line_restricted_worst(gridmap_t *gm, double x0, double y0, double x1, double y1,
                                        int *cost, int *restricted)
{
    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
 
    int ix_len = ix1-ix0;
    int iy_len = iy1-iy0;
    int steps  = abs(ix_len);
    if (steps<abs(iy_len))
        steps = abs(iy_len);

    // default values if we're off the map...
    *cost = 253;
    *restricted = 0;

    if (ix0 < 0 || ix0 >= gm->width)
        return;
    if (ix1 < 0 || ix1 >= gm->width)
        return;

    if (iy0 < 0 || iy0 >= gm->height)
        return;
    if (iy1 < 0 || iy1 >= gm->height)
        return;
    
    // how far (in indices) we move at each sample point.
    if (steps == 0) {
        printf("WARNING: %s:%s: steps == 0. Critical Error!!!\n", __FILE__, 
               __FUNCTION__);
        return;
    }
    double dx = (ix_len)/((double)steps);
    double dy = (iy_len)/((double)steps);
 
    double x = ix0; // our array indices, in floating precision...
    double y = iy0;
    int cost_max = 0, restricted_max = 0;
    
    for (int i = 0; i <= steps; i++, x+=dx, y+=dy) {
        int ix = (int) (x+0.5); // compute integer indices again
        int iy = (int) (y+0.5);

        uint8_t color   = gm->data[iy*gm->width + ix];
        cost_max        = imax(cost_max, (color&0xfe));
        restricted_max |= (color&1);
    }

    *cost = cost_max;
    *restricted = restricted_max;
}

// returns cost < 0 for infeasible.  cost_fill/restrict_fill used if
// the query is out of range of the gridmap (but this isn't well
// defined anyway?). 
void gridmap_test_line_restricted(gridmap_t *gm, double x0, double y0, double x1, double y1,
                                  double *cost, double *restricted)
{
    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
 
    int ix_len = ix1-ix0;
    int iy_len = iy1-iy0;
    int steps  = abs(ix_len);
    if (steps<abs(iy_len))
        steps = abs(iy_len);

    // total distance travelled
    double dist = sqrt(sq(x1-x0) + sq(y1-y0));

    // default values if we're off the map...
    *cost = 253 * dist;
    //*restricted = 1 * dist;
    *restricted = 0;

    if (ix0 < 0 || ix0 >= gm->width)
        return;
    if (ix1 < 0 || ix1 >= gm->width)
        return;

    if (iy0 < 0 || iy0 >= gm->height)
        return;
    if (iy1 < 0 || iy1 >= gm->height)
        return;
    
    // how far (in indices) we move at each sample point.
    if (steps == 0) {
        printf("WARNING: %s:%s: steps == 0. Critical Error!!!\n", __FILE__, 
               __FUNCTION__);
        return;
    }
    double dx = (ix_len)/((double)steps);
    double dy = (iy_len)/((double)steps);
 
    double x = ix0; // our array indices, in floating precision...
    double y = iy0;
    int cost_acc = 0, restricted_acc = 0;
    
    for (int i = 0; i <= steps; i++, x+=dx, y+=dy) {
        int ix = (int) (x+0.5); // compute integer indices again
        int iy = (int) (y+0.5);

        uint8_t color = gm->data[iy*gm->width + ix];
        cost_acc += (color&0xfe);
        restricted_acc += (color&1);

        if (color >= 254) {
            *cost = -1;
            *restricted = 0;
            return;
        }
    }

    *cost = dist * cost_acc / steps;
    *restricted = dist * restricted_acc / steps;
}

double gridmap_test_line(gridmap_t *gm, double x0, double y0, double x1, double y1)
{
    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
 
    int ix_len = ix1-ix0;
    int iy_len = iy1-iy0;
    int steps  = abs(ix_len);
    if (steps<abs(iy_len))
        steps = abs(iy_len);

    // how far (in indices) we move at each sample point.
    double dx = (ix_len)/((double)steps);
    double dy = (iy_len)/((double)steps);

    if (ix0 < 0 || ix0 >= gm->width)
        return 0;
    if (ix1 < 0 || ix1 >= gm->width)
        return 0;

    if (iy0 < 0 || iy0 >= gm->height)
        return 0;
    if (iy1 < 0 || iy1 >= gm->height)
        return 0;
    
    double x = ix0; // our array indices, in floating precision...
    double y = iy0;
    int acc = 0;

    for (int i = 0; i <= steps; i++, x+=dx, y+=dy) {
        int ix = (int) (x+0.5); // compute integer indices again
        int iy = (int) (y+0.5);

        uint8_t color = gm->data[iy*gm->width + ix];
        acc += color;

        if (color == 255)
            return -1;
    }

    double dist = sqrt(sq(x1-x0) + sq(y1-y0));

    return dist * acc / steps;
}

int gridmap_test_line2(gridmap_t *gm, double *integral, double *average, uint8_t *max, double x0, double y0, double x1, double y1)
{
    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
 
    double dist = sqrt(sq(x1-x0) + sq(y1-y0));
    int steps = dist * gm->pixels_per_meter + 1;
    uint8_t max_so_far=0;

    // how far (in indices) we move at each sample point.
    double dx = (ix1-ix0)/steps;
    double dy = (iy1-iy0)/steps;

    if (ix0 < 0 || ix0 >= gm->width)
        return 0;
    if (ix1 < 0 || ix1 >= gm->width)
        return 0;

    if (iy0 < 0 || iy0 >= gm->height)
        return 0;
    if (iy1 < 0 || iy1 >= gm->height)
        return 0;
    
    double x = ix0; // our array indices, in floating precision...
    double y = iy0;
    int acc = 0;

    for (int i = 0; i < steps; i++, x+=dx, y+=dy) {
        int ix = (int) x; // compute integer indices again
        int iy = (int) y;

        uint8_t color = gm->data[iy*gm->width + ix];
        acc += color;

        if (color == 255) {
            if (max)
                *max=color;
            return -1;
        }
        if (max_so_far<color)
            max_so_far=color;

    }
    double my_average = acc / ((double)steps);
    if (average)
        *average =  my_average;

    if (integral)
        *integral =  my_average*dist;
    if (max)
        *max =  max_so_far;
    return 0;
}

void
gridmap_line_segment_stats (gridmap_t *gm, double *integral, double *average,
                            uint8_t *max, uint8_t *min, double x0, double y0, double x1, double y1)
{
    double dist = sqrt(sq(x1-x0) + sq(y1-y0));
    int steps = dist * gm->pixels_per_meter + 1;

    double gx0 = (x0 - gm->x0) * gm->pixels_per_meter;
    double gx1 = (x1 - gm->x0) * gm->pixels_per_meter;
    double gy0 = (y0 - gm->y0) * gm->pixels_per_meter;
    double gy1 = (y1 - gm->y0) * gm->pixels_per_meter;

    assert (gx0 >= gm->x0 && gx1 >= gm->x0 && 
            gx0 < gm->width-1 && gx1 < gm->width-1 &&
            gy0 >= gm->y0 && gy1 >= gm->y0 && 
            gy0 < gm->height-1 && gy1 < gm->height-1);

    double dx = (gx1 - gx0) / steps;
    double dy = (gy1 - gy0) / steps;

    double acc = 0;
    double max_so_far = 0;
    double min_so_far = 255;

    for (int i=0; i<steps; i++) {
        double x = gx0 + i * dx;
        double y = gy0 + i * dy;

        int ix = (int)x;
        int iy = (int)y;

        double v0 = gm->data[iy*gm->width+ix];
        double v1 = gm->data[iy*gm->width+(ix+1)];
        double v2 = gm->data[(iy+1)*gm->width+ix];
        double v3 = gm->data[(iy+1)*gm->width+(ix+1)];

        double ax = x - floor(x);
        double ay = y - floor(y);

        double v = 
            (1-ax)*(1-ay)*v0 + 
            (ax)*(1-ay)*v1 + 
            (1-ax)*  (ay)*v2 +
            (ax)*  (ay)*v3;

        acc += v;
        max_so_far = fmax(max_so_far, v);
        min_so_far = fmin(min_so_far, v);
    }

    double my_average = acc / steps;
    if (average) *average = my_average;
    if (integral) *integral = my_average*dist;
    if (max) *max = max_so_far;
    if (min) *min = min_so_far;
}

double gridmap_test_line_old(gridmap_t *gm, double x0, double y0, double x1, double y1)
{
    double dist = sqrt(sq(x1-x0) + sq(y1-y0));
    int steps = dist * gm->pixels_per_meter + 1;

    double dx = (x1-x0)/steps;
    double dy = (y1-y0)/steps;

    double x = x0;
    double y = y0;

    // XXX Very slow line search.
    int acc = 0;
    
    for (int i = 0; i < steps; i++) {
        int ix, iy;
        gridmap_to_ix_iy(gm, x, y, &ix, &iy);

        if (ix>=0 && ix < gm->width && iy>=0 && iy < gm->height) {
            uint8_t color = gm->data[iy*gm->width + ix];
            if (color == 255)
                return -1;
            else
                acc += color;
        }

        x+=dx;
        y+=dy;
    }

    return acc * dist / steps;
}

int gridmap_search_non_zero(gridmap_t *gm, double x0, double y0, double x1, double y1)
{
    double dist = sqrt(sq(x1-x0) + sq(y1-y0));
    int steps = dist * gm->pixels_per_meter + 1;

    double dx = (x1-x0)/steps;
    double dy = (y1-y0)/steps;

    double x = x0;
    double y = y0;

    // XXX Very slow line search.

    for (int i = 0; i < steps; i++) {
        int ix, iy;
        gridmap_to_ix_iy(gm, x, y, &ix, &iy);

        if (ix>=0 && ix < gm->width && iy>=0 && iy < gm->height) {
            uint8_t color = gm->data[iy*gm->width + ix];
            if (color)
                return 1;
        }

        x+=dx;
        y+=dy;
    }

    return 0;
}

// Set the LSB of each cell within the rectangle returns 1 if any
// rectangle was previously 0 (bits 7:1).
int gridmap_mark_rectangle(gridmap_t *gm, double x0, double y0, double sizex, double sizey)
{
    int xlo, xhi, ylo, yhi;

    gridmap_to_ix_iy(gm, x0, y0, &xlo, &ylo);
    gridmap_to_ix_iy(gm, x0+sizex, y0+sizey, &xhi, &yhi);

    if (xlo < 0) xlo = 0;
    if (ylo < 0) ylo = 0;
    if (xhi >= gm->width) xhi = gm->width - 1;
    if (yhi >= gm->height) yhi = gm->height -1;

    int ret = 0;

    for (int y = ylo; y <= yhi; y++)
        for (int x = xlo; x <= xhi; x++) {
            int v = gm->data[y*gm->width + x];
            if (v <= 1)
                ret = 1;
            gm->data[y*gm->width + x] = v | 1;
        }

    return ret;
}

// For each cell, if the low bit is set, increment bits (7-1), and
// clear the low bit. If the low bit was not set, the cell is cleared.
void gridmap_mark_update(gridmap_t *gm, int increment, int decrement)
{
    for (int y = 0; y < gm->height; y++) {
        for (int x = 0; x < gm->width; x++) {
            if (gm->data[y*gm->width + x]&1) {
                int v = gm->data[y*gm->width + x] + (increment<<1);
                if (v>254)
                    v=254;
                gm->data[y*gm->width + x] = v&0xfe;
            } else {
                int v = gm->data[y*gm->width + x] - (decrement<<1);
                if (v < 0)
                    v = 0;
                gm->data[y*gm->width + x] = (v&0xfe);
            }
        }
    }
}


// fill all of the cells that the rectangle touches
void gridmap_draw_rectangle(gridmap_t *gm, double x0, double y0, double sizex, double sizey, uint8_t color)
{
    int xlo, xhi, ylo, yhi;

    gridmap_to_ix_iy(gm, x0, y0, &xlo, &ylo);
    gridmap_to_ix_iy(gm, x0+sizex, y0+sizey, &xhi, &yhi);

    if (xlo < 0) xlo = 0;
    if (ylo < 0) ylo = 0;
    if (xhi >= gm->width) xhi = gm->width - 1;
    if (yhi >= gm->height) yhi = gm->height -1;

    for (int y = ylo; y <= yhi; y++)
        for (int x = xlo; x <= xhi; x++)
            gm->data[y*gm->width + x] = color;
}

// fill all of the cells that the rectangle touches
void gridmap_draw_rectangle_max(gridmap_t *gm, double x0, double y0, double sizex, double sizey, uint8_t color)
{
    int xlo, xhi, ylo, yhi;

    gridmap_to_ix_iy(gm, x0, y0, &xlo, &ylo);
    gridmap_to_ix_iy(gm, x0+sizex, y0+sizey, &xhi, &yhi);

    if (xlo < 0) xlo = 0;
    if (ylo < 0) ylo = 0;
    if (xhi >= gm->width) xhi = gm->width - 1;
    if (yhi >= gm->height) yhi = gm->height -1;

    for (int y = ylo; y <= yhi; y++)
        for (int x = xlo; x <= xhi; x++)
            gm->data[y*gm->width + x] = imax(gm->data[y*gm->width+x], color);
}

// fill all of the cells that the rectangle touches. a value of 1
// takes precedence over any other value.
void gridmap_draw_rectangle_max_special(gridmap_t *gm, double x0, double y0, double sizex, double sizey, uint8_t color)
{
    int xlo, xhi, ylo, yhi;

    gridmap_to_ix_iy(gm, x0, y0, &xlo, &ylo);
    gridmap_to_ix_iy(gm, x0+sizex, y0+sizey, &xhi, &yhi);

    if (xlo < 0) xlo = 0;
    if (ylo < 0) ylo = 0;
    if (xhi >= gm->width) xhi = gm->width - 1;
    if (yhi >= gm->height) yhi = gm->height -1;

    for (int y = ylo; y <= yhi; y++)
        for (int x = xlo; x <= xhi; x++) {
            int v = gm->data[y*gm->width + x];
            if (v==1 || color==1)
                v = 1;
            else
                v = imax(v, color);

            gm->data[y*gm->width + x] = v;
        }
}

#define ONEBYTE_LEN_BITS  1
#define ONEBYTE_COLOR_BITS (8-ONEBYTE_LEN_BITS)

static inline int decode_run(const uint8_t *data, int *run_color, int *run_length, int last_color)
{
    if (data[0] != 0) {
        int v = data[0];
        *run_length = (v & ((1 << ONEBYTE_LEN_BITS) - 1)) + 1;

        int color_code = v >> ONEBYTE_LEN_BITS;
        if (color_code == 1)
            *run_color = 0;
        else if (color_code == 2)
            *run_color = 255;
        else {
            *run_color = (color_code - 3) + last_color - (1 << (ONEBYTE_COLOR_BITS - 1));
        }
        return 1;
    }

    *run_length = (data[1]<<8) + data[2];
    *run_color = data[3];
    return 4;
}

static inline int encode_run(uint8_t *data, int maxlen, int run_color, int run_length, int last_color)
{
    if (maxlen < 3) {
        printf("gridmap_encode: not enough buffer space\n");
        return 0;
    }

    if (run_length > 0 && run_length <= (1 << ONEBYTE_LEN_BITS))
    {
        // one-byte encoding:
        // 7 6 5 4 3 2 1 0
        //  dcolor   len

        int dcolor = (run_color - last_color + (1 << (ONEBYTE_COLOR_BITS - 1))) & 0xff;
        if (dcolor < ((1 << ONEBYTE_COLOR_BITS) - 3) || run_color == 0 || run_color == 255) {

            int color_code;

            // never emit color_code == 0
            if (run_color == 0)
                color_code = 1;
            else if (run_color == 255)
                color_code = 2;
            else
                color_code = dcolor + 3;

            data[0] = (run_length - 1) | (color_code << ONEBYTE_LEN_BITS);
            return 1;
        }
    }

    data[0] = 0;
    data[1] = (run_length >> 8);
    data[2] = (run_length & 0xff);
    data[3] = run_color;

    return 4;
}

// dimensions must be fully contained within the gridmap.
int gridmap_encode(const gridmap_t *gm, int xoff, int yoff, int width, int height, uint8_t *data, int maxlen, int *datalen)
{
    assert(xoff >= 0 && xoff+width <= gm->width);
    assert(yoff >= 0 && yoff+height <= gm->height);
    assert(maxlen >= EOF_LEN);

    int last_color = 0;
    int run_color = -1;
    int run_length = 0;
    int outpos = 0;

    for (int y = yoff; y < yoff+height; y++) {
        for (int x = xoff; x < xoff+width; x++) {

            
            if (run_color == 0 && (xoff+width-x >=8) && (run_length + 8) <= MAX_RUN_LENGTH) {
                int64_t *p = (int64_t*) &gm->data[y*gm->width+x];
                if (*p==0) {
                    run_length+=8;
                    x+=7;
                    continue;
                }
            }

            int color = gm->data[y*gm->width + x];

            // add to existing run?
            if (color == run_color && (run_length + 1 ) <= MAX_RUN_LENGTH) {
                run_length++;
                continue;
            }

            // emit previous run (save room for EOF)
            if (run_length) {
                outpos += encode_run(&data[outpos], 
                                     maxlen - outpos - EOF_LEN, run_color, run_length, 
                                     last_color);
                last_color = run_color;
            }

            // start a new run
            run_color = color;
            run_length = 1;
        }
    }

    if (run_length) {
        outpos += encode_run(&data[outpos], maxlen - outpos - EOF_LEN, run_color, run_length, last_color);
        last_color = run_color;
    }

    // EOF
    outpos += encode_run(&data[outpos], maxlen - outpos, 0, 0, 0);

    *datalen = outpos;

    return 0;
}

// The tile can have any amount of overlap with the gridmap.
int gridmap_decode(gridmap_t *gm, int xoff, int yoff, int width, int height, uint8_t *data, int datalen)
{
    int x = xoff, y = yoff;
    int inpos = 0;
    int last_color = 0;

    // none of the tile overlaps?
    if (xoff + width < 0 || yoff + height < 0 || xoff >= gm->width || yoff >= gm->height)
        return 0;

    while (inpos < datalen) {
        int run_color, run_length;

        inpos += decode_run(&data[inpos], &run_color, &run_length, last_color);
        last_color = run_color;

        if (run_length == 0) { // EOF?
            return 0;
        }

        while (run_length) {

            // no more data, exit.
            if (y >= gm->height)
                return 0;

            int use_length = 0;

            if (y < 0) {
                // drop (up to) the rest of the row.
                use_length = imin(run_length, xoff + width - x);
            } else if (x < 0) {
                // drop (up to) the beginning of the row.
                use_length = imin(run_length, -x);
            } else if (x >= gm->width) {
                // drop (up to) the rest of the row.
                use_length = imin(run_length, xoff + width - x);
            } else {
                // data is *in bounds*. Copy up to the remainder of the row.
                use_length = imin(run_length, xoff + width - x);
                use_length = imin(use_length, gm->width - x);

                memset(&gm->data[y*gm->width + x], run_color, use_length);
            }

            run_length -= use_length;
            x+= use_length;

            if (x == xoff + width) {
                x = xoff;
                y++;
                if (y >= yoff + height && run_length) {
                    printf("gridmap_decode: run-on decode\n");
                    return -1;
                }
            }
        }
    }

    return 0;
}

// The tile can have any amount of overlap with the gridmap.
int gridmap_decode_base(uint8_t *dest, int width, int height, uint8_t *src, int datalen)
{
    int x = 0, y = 0;
    int inpos = 0;
    int last_color = 0;

    while (inpos < datalen) {
        int run_color, run_length;

        inpos += decode_run(&src[inpos], &run_color, &run_length, last_color);
        last_color = run_color;

        if (run_length == 0) { // EOF?
            return 0;
        }

        while (run_length) {

            // no more data, exit.
            if (y >= height)
                return 0;

            int use_length = 0;

            if (y < 0) {
                // drop (up to) the rest of the row.
                use_length = imin(run_length, width - x);
            } else if (x < 0) {
                // drop (up to) the beginning of the row.
                use_length = imin(run_length, -x);
            } else if (x >= width) {
                // drop (up to) the rest of the row.
                use_length = imin(run_length, width - x);
            } else {
                // data is *in bounds*. Copy up to the remainder of the row.
                use_length = imin(run_length, width - x);
                use_length = imin(use_length, width - x);

                memset(&dest[y*width + x], run_color, use_length);
            }

            run_length -= use_length;
            x+= use_length;

            if (x == width) {
                x = 0;
                y++;
                if (y >= height && run_length) {
                    printf("gridmap_decode: run-on decode\n");
                    return -1;
                }
            }
        }
    }

    return 0;
}

// Copy pixels out of the gridmap. xoff,yoff must be inside the grid, though the width and height
// may extend outside the allocated gridmap. The result is put in data
int gridmap_get_pixels_aligned(gridmap_t *gm, int xoff, int yoff, int width, int height, uint8_t *data, uint8_t fill)
{
    int outidx = 0;

    int max_x = xoff + width;

    // how much of the request falls outside the gridmap? (x component)
    int out_size = max_x - gm->width;
    if (out_size < 0)
        out_size = 0;

    // how much of the request falls inside the gridmap? (x component)
    int in_size = width - out_size;

    // iterate over rows...
    for (int y = yoff; y < height + yoff; y++) {

        if (y < 0 || y >= gm->height) {
            // this whole row is out of bounds
            memset(&data[outidx], fill, width);
            outidx += width;
        } else {
            // the row is in bounds
            memcpy(&data[outidx], &gm->data[y*gm->width + xoff], in_size);
            outidx += in_size;
            memset(&data[outidx], fill, out_size);
            outidx += out_size;
        }
    }

    return 0;
}

int gridmap_recenter(gridmap_t *gm, double cx, double cy, double max_distance, uint8_t fill)
{
    double distance_sq = sq(cx - gm->cx) + sq(cy-gm->cy);
    if (distance_sq < sq(max_distance))
        return 0;

    // how far (in pixels) to move. We round to an exact pixel boundary.
    // e.g., dx = source - dest
    int dx = (cx - gm->cx) * gm->pixels_per_meter;
    int dy = (cy - gm->cy) * gm->pixels_per_meter;

//    printf("RECENTER %15f %5d %5d\n", distance_sq, dx, dy);

    if (dy >= 0) {
        for (int dest = 0; dest < gm->height; dest++) {
            
            int src = dest + dy;

            if (src < 0 || src >= gm->height) {
                memset(&gm->data[dest*gm->width], fill, gm->width);
                continue;
            } 
            
            if (dx >= 0) {
                int sz = imax(0, gm->width - dx);
                memmove(&gm->data[dest*gm->width], &gm->data[src*gm->width + dx], sz);
                memset(&gm->data[dest*gm->width + sz], fill, gm->width - sz);
            } else {
                int sz = imax(0, gm->width + dx);
                memmove(&gm->data[dest*gm->width - dx], &gm->data[src*gm->width], sz);
                memset(&gm->data[dest*gm->width], fill, gm->width - sz);
            }
        }
    } else {
        for (int dest = gm->height-1; dest >= 0; dest--) {
             
            int src = dest + dy;

            if (src < 0 || src >= gm->height) {
                memset(&gm->data[dest*gm->width], fill, gm->width);
                continue;
            } 
                      
            if (dx >= 0) {
                int sz = imax(0, gm->width - dx);
                memmove(&gm->data[dest*gm->width], &gm->data[src*gm->width + dx], sz);
                memset(&gm->data[dest*gm->width + sz], fill, gm->width - sz);
            } else {
                int sz = imax(0, gm->width + dx);
                memmove(&gm->data[dest*gm->width - dx], &gm->data[src*gm->width], sz);
                memset(&gm->data[dest*gm->width], fill, gm->width - sz);
            }
        }
    }

    gm->cx += dx * gm->meters_per_pixel;
    gm->cy += dy * gm->meters_per_pixel;
    gm->x0 += dx * gm->meters_per_pixel;
    gm->y0 += dy * gm->meters_per_pixel;

    return 0;
}

int gridmap_expand_circle(gridmap_t *dst, const gridmap_t *src, double diameter_meters)
{
    return gridmap_expand_circle_ex(dst, src, diameter_meters, 0, 0, src->width, src->height);
}

int gridmap_expand_circle_ex(gridmap_t *dst, const gridmap_t *src, double diameter_meters,
                             int xidx, int yidx, int xwidth, int ywidth)
{
    assert(dst->width == src->width);
    assert(dst->height == src->height);

    dst->cx = src->cx;
    dst->cy = src->cy;
    dst->x0 = src->x0;
    dst->y0 = src->y0;
    dst->meters_per_pixel = src->meters_per_pixel;
    dst->pixels_per_meter = src->pixels_per_meter;

    int diam = diameter_meters*dst->pixels_per_meter + .5;
    int radius = diam/2;

    // no work to do?
    if (radius == 0) {
        memcpy(dst->data, src->data, src->width*src->height);
        return 0;
    }

    memset(dst->data, 0, dst->width*dst->height);
    //memcpy(dst->data, src->data, dst->width*dst->height);

    // Consider this diam=4 circle approximation
    // row             offset   width       
    //  0    .XX         1        2
    //  1    XXXX        0        4
    //  2    XXXX        0        4
    //  3     XX         1        2
    //
    // (. = origin, x0,y0)

    uint8_t bitmask[diam][diam];
    memset(bitmask, 0, sizeof(bitmask));
    
    // rasterize a circle (brute force)
    for (int x = 0; x < diam; x++) {
        for (int y = 0; y < diam; y++) {
            double dsq = sq(y + .5 - diam/2.0) + sq(x + .5 - diam/2.0);
            bitmask[x][y] = (dsq <= sq(diam/2)) ? 1 : 0;
        }
    }

    int offsets[diam];
    int widths[diam];

    for (int y = 0; y < diam; y++) {
        offsets[y] = 0;
        widths[y] = 0;

        for (int x = diam-1; x>=0; x--) {
            if (bitmask[x][y]) {
                offsets[y] = x;
                widths[y]++;
            }
        }
    }

    // compute the index of the right-most pixel in every row
    int right_xs[diam];
    for (int y = 0; y < diam; y++) {
        right_xs[y] = 0;
        for (int x = 0 ; x < diam; x++) {
            if (bitmask[x][y])
                right_xs[y] = x;
        }
    }

    // compute the index of the bottom-most pixel in every column.
    int bottom_ys[diam];
    for (int x = 0; x < diam; x++) {
        bottom_ys[x] = 0;
        for (int y = 0; y < diam; y++) {
            if (bitmask[x][y])
                bottom_ys[x] = y;
        }
    }

    /*
      for (int x = 0; x < diam; x++) {
      for (int y = 0; y < diam; y++) {
      printf("%c", bitmask[x][y] ? 'X' : '.');
      }
      printf("\n");
      }

      for (int i = 0; i < diam; i++) {
      printf("%4d:  %4d %4d %4d %4d\n", i, offsets[i], widths[i], right_xs[i], bottom_ys[i]);
      }
    */

    int ymin = imax(radius, yidx), ymax = imin(src->height - radius, yidx + ywidth);
    int xmin = imax(radius, xidx), xmax = imin(src->width - radius, xidx + xwidth);

    for (int y = ymin; y < ymax; y++) {
        for (int x = xmin; x < xmax; x++) {

            // optimization
            // if the next 4 pixels are all black, skip them in one go.
            uint32_t src_colors = *((uint32_t*) &src->data[y*src->width + x]);
            if (src_colors == 0) {
                x+=3;
                continue;
            }

            // handle this pixel.
            uint8_t src_color = src->data[y*src->width + x];
            if (src_color == 0)
                continue;

            // compute upper left coordinate of where we will plot our circle
            int x0 = x - diam/2;
            int y0 = y - diam/2;

            uint8_t src_color_up   = src->data[(y-1)*src->width + x];
            uint8_t src_color_left = src->data[y*src->width + x - 1];
//            uint8_t enable_opts = (y!=ymin) && (x!=xmin);

            if (src_color_left) {
                // the column to our left was set. we only need to
                // fill in the right most pixel of every row.
                int dy0 = src_color_up ? diam/2 : 0;
                for (int dy = dy0; dy < diam; dy++) {
                    dst->data[(y0 + dy)*dst->width + x0 + right_xs[dy]] = src_color;
                }
                continue;
            }

            if (src_color_up) {
                // the row above us was set. we only need to fill in
                // the bottom pixel of every column.
                for (int dx = 0; dx < diam; dx++) {
                    dst->data[(y0 + bottom_ys[dx])*dst->width + x0 + dx] = src_color;
                }
                continue;
            }

            // common case: brute force write.
            for (int i = 0; i < diam; i++)
                memset(&dst->data[(y0+i)*dst->width + x0 + offsets[i]], src_color, widths[i]);
        }
    }
    
    return 0;
}

int gridmap_expand_max_circle(gridmap_t *dst, const gridmap_t *src, double diameter_meters)
{
    return gridmap_expand_max_circle_ex(dst, src, diameter_meters, 0, 0, src->width, src->height);
}

// SLOW go get coffee 
int gridmap_expand_max_circle_ex(gridmap_t *dst, const gridmap_t *src, double diameter_meters,
                             int xidx, int yidx, int xwidth, int ywidth)
{
    assert(dst->width == src->width);
    assert(dst->height == src->height);

    dst->cx = src->cx;
    dst->cy = src->cy;
    dst->x0 = src->x0;
    dst->y0 = src->y0;
    dst->meters_per_pixel = src->meters_per_pixel;
    dst->pixels_per_meter = src->pixels_per_meter;

    int diam = diameter_meters*dst->pixels_per_meter + .5;
    int radius = diam/2;

    // no work to do?
    if (radius == 0) {
        memcpy(dst->data, src->data, src->width*src->height);
        return 0;
    }

    memset(dst->data, 0, dst->width*dst->height);

    // Consider this diam=4 circle approximation
    // row             offset   width       
    //  0    .XX         1        2
    //  1    XXXX        0        4
    //  2    XXXX        0        4
    //  3     XX         1        2
    //
    // (. = origin, x0,y0)

    uint8_t bitmask[diam][diam];
    memset(bitmask, 0, sizeof(bitmask));
    
    // rasterize a circle (brute force)
    for (int x = 0; x < diam; x++) {
        for (int y = 0; y < diam; y++) {
            double dsq = sq(y + .5 - diam/2.0) + sq(x + .5 - diam/2.0);
            bitmask[x][y] = (dsq <= sq(diam/2)) ? 1 : 0;
        }
    }

    int offsets[diam];
    int widths[diam];

    for (int y = 0; y < diam; y++) {
        offsets[y] = 0;
        widths[y] = 0;

        for (int x = diam-1; x>=0; x--) {
            if (bitmask[x][y]) {
                offsets[y] = x;
                widths[y]++;
            }
        }
    }

    // compute the index of the right-most pixel in every row
    int right_xs[diam];
    for (int y = 0; y < diam; y++) {
        right_xs[y] = 0;
        for (int x = 0 ; x < diam; x++) {
            if (bitmask[x][y])
                right_xs[y] = x;
        }
    }

    // compute the index of the bottom-most pixel in every column.
    int bottom_ys[diam];
    for (int x = 0; x < diam; x++) {
        bottom_ys[x] = 0;
        for (int y = 0; y < diam; y++) {
            if (bitmask[x][y])
                bottom_ys[x] = y;
        }
    }

    int ymin = imax(radius, yidx), ymax = imin(src->height - radius, yidx + ywidth);
    int xmin = imax(radius, xidx), xmax = imin(src->width - radius, xidx + xwidth);
    
    for (int y = ymin; y < ymax; y++) {
        for (int x = xmin; x < xmax; x++) {
            // optimization
            // if the next 4 pixels are all black, skip them in one go.
            uint32_t src_colors = *((uint32_t*) &src->data[y*src->width + x]);
            if (src_colors == 0) {
                x+=3;
                continue;
            }
            
            int x0 = x - diam/2;
            int y0 = y - diam/2;
            // common case: brute force write.
            uint8_t max_val=0;
            for (int i = 0; i < diam; i++) {
                for (int j = 0; j < widths[i]; j++) {
                    uint8_t val=src->data[(y0+i)*dst->width + x0 + offsets[i]+j];
                    if (val>max_val)
                        max_val=val;
                }
            }
            for (int i = 0; i < diam; i++) {
                memset(&dst->data[(y0+i)*dst->width + x0 + offsets[i]], max_val, widths[i]);
            }
        }
    }   
    return 0;
}

static inline double max_d(double a, double b)
{
    return (a > b) ? a : b;
}

/*
  int gridmap_render_rectangle(gridmap_t *gm, 
  double cx, double cy, 
  double x_size, double y_size, 
  double theta,
  int *table, double table_max_dist_sq, int table_size)
  {
  double ux, uy;
  sincos(theta, &uy, &ux);

  double d = sqrt(table_max_dist_sq);

  double x_bound = (x_size / 2.0 * fabs(ux) + y_size / 2.0 * fabs(uy)) + d;
  double y_bound = (x_size / 2.0 * fabs(uy) + y_size / 2.0 * fabs(ux)) + d;

  double x0 = cx - x_bound, x1 = cx + x_bound;
  double y0 = cy - y_bound, y1 = cy + y_bound;

  int ix0, iy0, ix1, iy1;
  gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
  gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
    
  ix0 = iclamp(ix0, 0, gm->width-1);
  ix1 = iclamp(ix1, 0, gm->width-1);
  iy0 = iclamp(iy0, 0, gm->height-1);
  iy1 = iclamp(iy1, 0, gm->height-1);

  gridmap_to_x_y(gm, ix0, iy0, &x0, &y0);
  gridmap_to_x_y(gm, ix1, iy1, &x1, &y1);

  double table_max_dist_sq_inv = 1.0 / table_max_dist_sq;

  // offset so that we test the middle of every pixel
  x0 += gm->meters_per_pixel * .5;
  y0 += gm->meters_per_pixel * .5;

  double y = y0;
  for (int iy = iy0; iy <= iy1; iy++, y+= gm->meters_per_pixel) {
  double x = x0;
  for (int ix = ix0; ix <= ix1; ix++, x += gm->meters_per_pixel) {
            
  // distances from query point to center of rectangle
  double dx = x - cx, dy = y - cy;

  // how long are the projections of the vector (dx,dy) onto the two principle
  // components of the rectangle? How much longer are they than the dimensions
  // of the rectangle?
  double c1 = fabs(dx * ux + dy * uy) - (x_size / 2);
  double c2 = fabs(- dx * uy + dy * ux) - (y_size / 2);

  // if the projection length is < 0, we're *inside* the rectangle.
  double distsq = 0;
  c1 = max_d(0, c1);
  c2 = max_d(0, c2);
            
  distsq = c1*c1 + c2*c2;
  
  if (distsq == 0)
  gm->data[iy*gm->width+ix] = table[0];

  int idx = distsq * table_size * table_max_dist_sq_inv;
            
  if (idx < table_size) {
  int v = table[idx];
  int oldv = gm->data[iy*gm->width+ix];
  if (v > oldv)
  gm->data[iy*gm->width+ix] = v;
  } 
  }
  }

  return 0;
  }
*/
#include <emmintrin.h>

static inline __m128 _mm_abs_ps(__m128 v)
{
//    __m128 neg = _mm_set1_ps(-1);
//    __m128 negv = _mm_mul_ps(v, neg);
//    return _mm_max_ps(v, negv);    
    static const union { int i[4]; __m128 m; } mask = {{0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff}};
    return _mm_and_ps(mask.m, v);
}

// this could obviously be spe906cial cased to be substantially faster.
int gridmap_render_line(gridmap_t *gm, 
                        double xy0[2], double xy1[2],
                        const gridmap_lut_t *lut)
{
    double cx = (xy0[0] + xy1[0])/2;
    double cy = (xy0[1] + xy1[1])/2;
    double x_size = sqrt(sq(xy0[0] - xy1[0]) + sq(xy0[1] - xy1[1]));
    double theta = atan2(xy1[1] - xy0[1], xy1[0] - xy0[0]);

    double ux, uy;
    sincos(theta, &uy, &ux);

    double d = lut->max_dist;

    double x_bound = x_size / 2.0 * fabs(ux) + d;
    double y_bound = x_size / 2.0 * fabs(uy) + d;

    double x0 = cx - x_bound, x1 = cx + x_bound;
    double y0 = cy - y_bound, y1 = cy + y_bound;

    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
    
    ix0 = iclamp(ix0, 0, gm->width-1);
    ix1 = iclamp(ix1, 0, gm->width-4);
    iy0 = iclamp(iy0, 0, gm->height-1);
    iy1 = iclamp(iy1, 0, gm->height-1);

    gridmap_to_x_y(gm, ix0, iy0, &x0, &y0);
    gridmap_to_x_y(gm, ix1, iy1, &x1, &y1);

    double table_max_dist_sq_inv = 1.0 / lut->max_dist_sq;

    // offset so that we test the middle of every pixel
    x0 += gm->meters_per_pixel * .5;
    y0 += gm->meters_per_pixel * .5;

    // increment for x after each iteration
    __m128 dxinc4 = _mm_set1_ps(4 * gm->meters_per_pixel);

    // "zero".
    __m128 zero4 = _mm_setzero_ps();

    __m128 ux4 = _mm_set1_ps(ux);
    __m128 uy4 = _mm_set1_ps(uy);

    __m128 xsize4 = _mm_set1_ps(x_size/2);

    __m128 to_index4 = _mm_set1_ps(lut->size * table_max_dist_sq_inv);
    __m128 max_index4 = _mm_set1_ps(lut->size - 1);

    double y = y0;
    for (int iy = iy0; iy <= iy1; iy++, y+= gm->meters_per_pixel) {

        // x distance to center of box for the next four pixels
        __m128 dx4 = _mm_setr_ps(x0 - cx, 
                                 x0 - cx + gm->meters_per_pixel, 
                                 x0 - cx + 2*gm->meters_per_pixel, 
                                 x0 - cx + 3*gm->meters_per_pixel);
        
        __m128 dy4 = _mm_set1_ps(y - cy);
        __m128 dy_ux4 = _mm_mul_ps(dy4, ux4);
        __m128 dy_uy4 = _mm_mul_ps(dy4, uy4);

        for (int ix = ix0; ix <= ix1; ix+=4) {
            
            // how long are the projections of the vector (dx,dy) onto the two principle
            // components of the rectangle? How much longer are they than the dimensions
            // of the rectangle?
            __m128 dx_ux4 = _mm_mul_ps(dx4, ux4);
            __m128 ca4 = _mm_add_ps(dx_ux4, dy_uy4);
            ca4 = _mm_abs_ps(ca4);
            ca4 = _mm_sub_ps(ca4, xsize4);
            ca4 = _mm_max_ps(ca4, zero4);
            ca4 = _mm_mul_ps(ca4, ca4);

            __m128 dx_uy4 = _mm_mul_ps(dx4, uy4);
            __m128 cb4 = _mm_sub_ps(dy_ux4, dx_uy4);
            cb4 = _mm_abs_ps(cb4);
            cb4 = _mm_max_ps(cb4, zero4);
            cb4 = _mm_mul_ps(cb4, cb4);

            __m128 distsq4 = _mm_add_ps(ca4, cb4);

            __m128 idx4 = _mm_mul_ps(distsq4, to_index4);
            idx4 = _mm_min_ps(idx4, max_index4);

            float idx[4];
            _mm_store_ps(idx, idx4);

            for (int i = 0; i < 4; i++) {
                int oldv = gm->data[iy*gm->width + ix + i];
                int v = lut->table[(int) idx[i]];
                if (v > oldv)
                    gm->data[iy*gm->width + ix + i] = v;
            }

            dx4 = _mm_add_ps(dx4, dxinc4);
        }
    }

    return 0;


}

int gridmap_render_rectangle_aligned_2(gridmap_t *gm, 
                                       double cx, double cy, 
                                       double x_size, double y_size, 
                                       const gridmap_lut_t *lut)
{
    double d = lut->max_dist;

    double x_bound = (x_size / 2.0 ) + d;
    double y_bound = (y_size / 2.0 ) + d;

    double x0 = cx - x_bound, x1 = cx + x_bound;
    double y0 = cy - y_bound, y1 = cy + y_bound;

    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);

    ix0 = iclamp(ix0, 0, gm->width-1);
    ix1 = iclamp(ix1, 0, gm->width-1);
    iy0 = iclamp(iy0, 0, gm->height-1);
    iy1 = iclamp(iy1, 0, gm->height-1);

    gridmap_to_x_y(gm, ix0, iy0, &x0, &y0);
    
    double dist2idx = lut->size / lut->max_dist_sq;

    int nx = ix1 - ix0 + 1;
    double ex[nx];
    double dx = x0 + gm->meters_per_pixel * .5 - cx;
    for (int i = 0; i < nx; i++) {

        ex[i] = dist2idx * sq(fmax(0, fabs(dx) - x_size/2));
        dx += gm->meters_per_pixel;
    }

    double dy = y0 + gm->meters_per_pixel * .5 - cy;
    for (int iy = iy0; iy <= iy1; iy++) {
        
        double ey = dist2idx * sq(fmax(0, fabs(dy) - y_size/2));

        for (int ix = ix0; ix <= ix1; ix++) {

            int idx = ey + ex[ix - ix0];
            if (idx < lut->size) {
                int oldv = gm->data[iy*gm->width + ix];

                gm->data[iy*gm->width + ix] = imax(oldv, lut->table[idx]);
            }
        }

        dy += gm->meters_per_pixel;
    }

    return 0;
}

int gridmap_render_rectangle(gridmap_t *gm, 
                             double cx, double cy, 
                             double x_size, double y_size, 
                             double theta,
                             const gridmap_lut_t *lut)
{
    if (theta == 0)
        return gridmap_render_rectangle_aligned(gm, cx, cy, x_size, y_size, lut);

    double ux, uy;
    sincos(theta, &uy, &ux);

    double d = lut->max_dist;

    double x_bound = (x_size / 2.0 * fabs(ux) + y_size / 2.0 * fabs(uy)) + d;
    double y_bound = (x_size / 2.0 * fabs(uy) + y_size / 2.0 * fabs(ux)) + d;

    double x0 = cx - x_bound, x1 = cx + x_bound;
    double y0 = cy - y_bound, y1 = cy + y_bound;

    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
    
    ix0 = iclamp(ix0, 0, gm->width-1);
    ix1 = iclamp(ix1, 0, gm->width-4);
    iy0 = iclamp(iy0, 0, gm->height-1);
    iy1 = iclamp(iy1, 0, gm->height-1);

    gridmap_to_x_y(gm, ix0, iy0, &x0, &y0);
    gridmap_to_x_y(gm, ix1, iy1, &x1, &y1);

    // offset so that we test the middle of every pixel
    x0 += gm->meters_per_pixel * .5;
    y0 += gm->meters_per_pixel * .5;

    // increment for x after each iteration
    __m128 dxinc4 = _mm_set1_ps(4 * gm->meters_per_pixel);

    // "zero".
    __m128 zero4 = _mm_setzero_ps();

    __m128 ux4 = _mm_set1_ps(ux);
    __m128 uy4 = _mm_set1_ps(uy);

    __m128 xsize4 = _mm_set1_ps(x_size/2);
    __m128 ysize4 = _mm_set1_ps(y_size/2);

    __m128 to_index4 = _mm_set1_ps(lut->size * lut->max_dist_sq_inv);
    __m128 max_index4 = _mm_set1_ps(lut->size - 1);

    double y = y0;
    for (int iy = iy0; iy <= iy1; iy++, y+= gm->meters_per_pixel) {

        // x distance to center of box for the next four pixels
        __m128 dx4 = _mm_setr_ps(x0 - cx, 
                                 x0 - cx + gm->meters_per_pixel, 
                                 x0 - cx + 2*gm->meters_per_pixel, 
                                 x0 - cx + 3*gm->meters_per_pixel);
        
        __m128 dy4 = _mm_set1_ps(y - cy);
        __m128 dy_ux4 = _mm_mul_ps(dy4, ux4);
        __m128 dy_uy4 = _mm_mul_ps(dy4, uy4);

        for (int ix = ix0; ix <= ix1; ix+=4) {
            
            // how long are the projections of the vector (dx,dy) onto the two principle
            // components of the rectangle? How much longer are they than the dimensions
            // of the rectangle?
            __m128 dx_ux4 = _mm_mul_ps(dx4, ux4);
            __m128 ca4 = _mm_add_ps(dx_ux4, dy_uy4);
            ca4 = _mm_abs_ps(ca4);
            ca4 = _mm_sub_ps(ca4, xsize4);
            ca4 = _mm_max_ps(ca4, zero4);
            ca4 = _mm_mul_ps(ca4, ca4);

            __m128 dx_uy4 = _mm_mul_ps(dx4, uy4);
            __m128 cb4 = _mm_sub_ps(dy_ux4, dx_uy4);
            cb4 = _mm_abs_ps(cb4);
            cb4 = _mm_sub_ps(cb4, ysize4);
            cb4 = _mm_max_ps(cb4, zero4);
            cb4 = _mm_mul_ps(cb4, cb4);

            __m128 distsq4 = _mm_add_ps(ca4, cb4);

            __m128 idx4 = _mm_mul_ps(distsq4, to_index4);
            idx4 = _mm_min_ps(idx4, max_index4);

            float idx[4];
            _mm_store_ps(idx, idx4);

            for (int i = 0; i < 4; i++) {
                int oldv = gm->data[iy*gm->width + ix + i];
                int v = lut->table[(int) idx[i]];
                if (v > oldv)
                    gm->data[iy*gm->width + ix + i] = v;
            }

            dx4 = _mm_add_ps(dx4, dxinc4);
        }
    }

    return 0;
}

int gridmap_render_line_min(gridmap_t *gm, 
                            double xy0[2], double xy1[2],
                            const gridmap_lut_t *lut)
{
    double cx = (xy0[0] + xy1[0])/2;
    double cy = (xy0[1] + xy1[1])/2;
    double x_size = sqrt(sq(xy0[0] - xy1[0]) + sq(xy0[1] - xy1[1]));
    double theta = atan2(xy1[1] - xy0[1], xy1[0] - xy0[0]);
    return gridmap_render_rectangle_min(gm, cx, cy, x_size, 0.0, theta, lut);
}

int gridmap_render_line_min_keep_infeasible(gridmap_t *gm, 
                            double xy0[2], double xy1[2],
                            const gridmap_lut_t *lut)
{
    double cx = (xy0[0] + xy1[0])/2;
    double cy = (xy0[1] + xy1[1])/2;
    double x_size = sqrt(sq(xy0[0] - xy1[0]) + sq(xy0[1] - xy1[1]));
    double theta = atan2(xy1[1] - xy0[1], xy1[0] - xy0[0]);
    return gridmap_render_rectangle_min_keep_infeasible(gm, cx, cy, x_size, 0.0, theta, lut);
}

// the least restrictive value is used when painting for both cost and
// restricted-ness.
int gridmap_render_rectangle_min(gridmap_t *gm, 
                                 double cx, double cy, 
                                 double x_size, double y_size, 
                                 double theta,
                                 const gridmap_lut_t *lut)
{
    double ux, uy;
    sincos(theta, &uy, &ux);

    double d = lut->max_dist;

    double x_bound = (x_size / 2.0 * fabs(ux) + y_size / 2.0 * fabs(uy)) + d;
    double y_bound = (x_size / 2.0 * fabs(uy) + y_size / 2.0 * fabs(ux)) + d;

    double x0 = cx - x_bound, x1 = cx + x_bound;
    double y0 = cy - y_bound, y1 = cy + y_bound;

    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
    
    ix0 = iclamp(ix0, 0, gm->width-1);
    ix1 = iclamp(ix1, 0, gm->width-4);
    iy0 = iclamp(iy0, 0, gm->height-1);
    iy1 = iclamp(iy1, 0, gm->height-1);

    gridmap_to_x_y(gm, ix0, iy0, &x0, &y0);
    gridmap_to_x_y(gm, ix1, iy1, &x1, &y1);

    // offset so that we test the middle of every pixel
    x0 += gm->meters_per_pixel * .5;
    y0 += gm->meters_per_pixel * .5;

    // increment for x after each iteration
    __m128 dxinc4 = _mm_set1_ps(4 * gm->meters_per_pixel);

    // "zero".
    __m128 zero4 = _mm_setzero_ps();

    __m128 ux4 = _mm_set1_ps(ux);
    __m128 uy4 = _mm_set1_ps(uy);

    __m128 xsize4 = _mm_set1_ps(x_size/2);
    __m128 ysize4 = _mm_set1_ps(y_size/2);

    __m128 to_index4 = _mm_set1_ps(lut->size * lut->max_dist_sq_inv);
    __m128 max_index4 = _mm_set1_ps(lut->size - 1);

    double y = y0;
    for (int iy = iy0; iy <= iy1; iy++, y+= gm->meters_per_pixel) {

        // x distance to center of box for the next four pixels
        __m128 dx4 = _mm_setr_ps(x0 - cx, 
                                 x0 - cx + gm->meters_per_pixel, 
                                 x0 - cx + 2*gm->meters_per_pixel, 
                                 x0 - cx + 3*gm->meters_per_pixel);
        
        __m128 dy4 = _mm_set1_ps(y - cy);
        __m128 dy_ux4 = _mm_mul_ps(dy4, ux4);
        __m128 dy_uy4 = _mm_mul_ps(dy4, uy4);

        for (int ix = ix0; ix <= ix1; ix+=4) {
            
            // how long are the projections of the vector (dx,dy) onto the two principle
            // components of the rectangle? How much longer are they than the dimensions
            // of the rectangle?
            __m128 dx_ux4 = _mm_mul_ps(dx4, ux4);
            __m128 ca4 = _mm_add_ps(dx_ux4, dy_uy4);
            ca4 = _mm_abs_ps(ca4);
            ca4 = _mm_sub_ps(ca4, xsize4);
            ca4 = _mm_max_ps(ca4, zero4);
            ca4 = _mm_mul_ps(ca4, ca4);

            __m128 dx_uy4 = _mm_mul_ps(dx4, uy4);
            __m128 cb4 = _mm_sub_ps(dy_ux4, dx_uy4);
            cb4 = _mm_abs_ps(cb4);
            cb4 = _mm_sub_ps(cb4, ysize4);
            cb4 = _mm_max_ps(cb4, zero4);
            cb4 = _mm_mul_ps(cb4, cb4);

            __m128 distsq4 = _mm_add_ps(ca4, cb4);

            __m128 idx4 = _mm_mul_ps(distsq4, to_index4);
            idx4 = _mm_min_ps(idx4, max_index4);

            float idx[4];
            _mm_store_ps(idx, idx4);

            for (int i = 0; i < 4; i++) {
                int oldv = gm->data[iy*gm->width + ix + i];
                int v = lut->table[(int) idx[i]];

                int newv = imin(oldv, v) & 0xfe;
                newv |= (oldv&v&1);

                gm->data[iy*gm->width + ix + i] = newv;
            }

            dx4 = _mm_add_ps(dx4, dxinc4);
        }
    }

    return 0;
}

// the least restrictive value is used when painting for both cost and
// restricted-ness.
int gridmap_render_rectangle_min_keep_infeasible(gridmap_t *gm, 
                                                 double cx, double cy, 
                                                 double x_size, double y_size, 
                                                 double theta,
                                                 const gridmap_lut_t *lut)
{
    double ux, uy;
    sincos(theta, &uy, &ux);

    double d = lut->max_dist;

    double x_bound = (x_size / 2.0 * fabs(ux) + y_size / 2.0 * fabs(uy)) + d;
    double y_bound = (x_size / 2.0 * fabs(uy) + y_size / 2.0 * fabs(ux)) + d;

    double x0 = cx - x_bound, x1 = cx + x_bound;
    double y0 = cy - y_bound, y1 = cy + y_bound;

    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
    
    ix0 = iclamp(ix0, 0, gm->width-1);
    ix1 = iclamp(ix1, 0, gm->width-4);
    iy0 = iclamp(iy0, 0, gm->height-1);
    iy1 = iclamp(iy1, 0, gm->height-1);

    gridmap_to_x_y(gm, ix0, iy0, &x0, &y0);
    gridmap_to_x_y(gm, ix1, iy1, &x1, &y1);

    // offset so that we test the middle of every pixel
    x0 += gm->meters_per_pixel * .5;
    y0 += gm->meters_per_pixel * .5;

    // increment for x after each iteration
    __m128 dxinc4 = _mm_set1_ps(4 * gm->meters_per_pixel);

    // "zero".
    __m128 zero4 = _mm_setzero_ps();

    __m128 ux4 = _mm_set1_ps(ux);
    __m128 uy4 = _mm_set1_ps(uy);

    __m128 xsize4 = _mm_set1_ps(x_size/2);
    __m128 ysize4 = _mm_set1_ps(y_size/2);

    __m128 to_index4 = _mm_set1_ps(lut->size * lut->max_dist_sq_inv);
    __m128 max_index4 = _mm_set1_ps(lut->size - 1);

    double y = y0;
    for (int iy = iy0; iy <= iy1; iy++, y+= gm->meters_per_pixel) {

        // x distance to center of box for the next four pixels
        __m128 dx4 = _mm_setr_ps(x0 - cx, 
                                 x0 - cx + gm->meters_per_pixel, 
                                 x0 - cx + 2*gm->meters_per_pixel, 
                                 x0 - cx + 3*gm->meters_per_pixel);
        
        __m128 dy4 = _mm_set1_ps(y - cy);
        __m128 dy_ux4 = _mm_mul_ps(dy4, ux4);
        __m128 dy_uy4 = _mm_mul_ps(dy4, uy4);

        for (int ix = ix0; ix <= ix1; ix+=4) {
            
            // how long are the projections of the vector (dx,dy) onto the two principle
            // components of the rectangle? How much longer are they than the dimensions
            // of the rectangle?
            __m128 dx_ux4 = _mm_mul_ps(dx4, ux4);
            __m128 ca4 = _mm_add_ps(dx_ux4, dy_uy4);
            ca4 = _mm_abs_ps(ca4);
            ca4 = _mm_sub_ps(ca4, xsize4);
            ca4 = _mm_max_ps(ca4, zero4);
            ca4 = _mm_mul_ps(ca4, ca4);

            __m128 dx_uy4 = _mm_mul_ps(dx4, uy4);
            __m128 cb4 = _mm_sub_ps(dy_ux4, dx_uy4);
            cb4 = _mm_abs_ps(cb4);
            cb4 = _mm_sub_ps(cb4, ysize4);
            cb4 = _mm_max_ps(cb4, zero4);
            cb4 = _mm_mul_ps(cb4, cb4);

            __m128 distsq4 = _mm_add_ps(ca4, cb4);

            __m128 idx4 = _mm_mul_ps(distsq4, to_index4);
            idx4 = _mm_min_ps(idx4, max_index4);

            float idx[4];
            _mm_store_ps(idx, idx4);

            for (int i = 0; i < 4; i++) {
                int oldv = gm->data[iy*gm->width + ix + i];
                int v = lut->table[(int) idx[i]];

                int newv = imin(oldv, v) & 0xfe;
                newv |= (oldv&v&1);

                if (oldv < 254)
                    gm->data[iy*gm->width + ix + i] = newv;
            }

            dx4 = _mm_add_ps(dx4, dxinc4);
        }
    }

    return 0;
}

int gridmap_render_rectangle_restricted(gridmap_t *gm, 
                                        double cx, double cy, 
                                        double x_size, double y_size, 
                                        double theta,
                                        const gridmap_lut_t *lut)
{
// need to special case restricted...
//    if (theta == 0)
//        return gridmap_render_rectangle_aligned(gm, cx, cy, x_size, y_size, lut);

    double ux, uy;
    sincos(theta, &uy, &ux);

    double d = lut->max_dist;

    double x_bound = (x_size / 2.0 * fabs(ux) + y_size / 2.0 * fabs(uy)) + d;
    double y_bound = (x_size / 2.0 * fabs(uy) + y_size / 2.0 * fabs(ux)) + d;

    double x0 = cx - x_bound, x1 = cx + x_bound;
    double y0 = cy - y_bound, y1 = cy + y_bound;

    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
    
    ix0 = iclamp(ix0, 0, gm->width-1);
    ix1 = iclamp(ix1, 0, gm->width-4);
    iy0 = iclamp(iy0, 0, gm->height-1);
    iy1 = iclamp(iy1, 0, gm->height-1);

    gridmap_to_x_y(gm, ix0, iy0, &x0, &y0);
    gridmap_to_x_y(gm, ix1, iy1, &x1, &y1);

    // offset so that we test the middle of every pixel
    x0 += gm->meters_per_pixel * .5;
    y0 += gm->meters_per_pixel * .5;

    // increment for x after each iteration
    __m128 dxinc4 = _mm_set1_ps(4 * gm->meters_per_pixel);

    // "zero".
    __m128 zero4 = _mm_setzero_ps();

    __m128 ux4 = _mm_set1_ps(ux);
    __m128 uy4 = _mm_set1_ps(uy);

    __m128 xsize4 = _mm_set1_ps(x_size/2);
    __m128 ysize4 = _mm_set1_ps(y_size/2);

    __m128 to_index4 = _mm_set1_ps(lut->size * lut->max_dist_sq_inv);
    __m128 max_index4 = _mm_set1_ps(lut->size - 1);

    double y = y0;
    for (int iy = iy0; iy <= iy1; iy++, y+= gm->meters_per_pixel) {

        // x distance to center of box for the next four pixels
        __m128 dx4 = _mm_setr_ps(x0 - cx, 
                                 x0 - cx + gm->meters_per_pixel, 
                                 x0 - cx + 2*gm->meters_per_pixel, 
                                 x0 - cx + 3*gm->meters_per_pixel);
        
        __m128 dy4 = _mm_set1_ps(y - cy);
        __m128 dy_ux4 = _mm_mul_ps(dy4, ux4);
        __m128 dy_uy4 = _mm_mul_ps(dy4, uy4);

        for (int ix = ix0; ix <= ix1; ix+=4) {
            
            // how long are the projections of the vector (dx,dy) onto the two principle
            // components of the rectangle? How much longer are they than the dimensions
            // of the rectangle?
            __m128 dx_ux4 = _mm_mul_ps(dx4, ux4);
            __m128 ca4 = _mm_add_ps(dx_ux4, dy_uy4);
            ca4 = _mm_abs_ps(ca4);
            ca4 = _mm_sub_ps(ca4, xsize4);
            ca4 = _mm_max_ps(ca4, zero4);
            ca4 = _mm_mul_ps(ca4, ca4);

            __m128 dx_uy4 = _mm_mul_ps(dx4, uy4);
            __m128 cb4 = _mm_sub_ps(dy_ux4, dx_uy4);
            cb4 = _mm_abs_ps(cb4);
            cb4 = _mm_sub_ps(cb4, ysize4);
            cb4 = _mm_max_ps(cb4, zero4);
            cb4 = _mm_mul_ps(cb4, cb4);

            __m128 distsq4 = _mm_add_ps(ca4, cb4);

            __m128 idx4 = _mm_mul_ps(distsq4, to_index4);
            idx4 = _mm_min_ps(idx4, max_index4);

            float idx[4];
            _mm_store_ps(idx, idx4);

            for (int i = 0; i < 4; i++) {
                int oldv = gm->data[iy*gm->width + ix + i];
                int v = lut->table[(int) idx[i]];

                int newv = imax(oldv, v) | ((oldv | v) & 1);
                gm->data[iy*gm->width + ix + i] = newv;
            }

            dx4 = _mm_add_ps(dx4, dxinc4);
        }
    }

    return 0;
}

int gridmap_render_rectangle_aligned(gridmap_t *gm, 
                                     double cx, double cy, 
                                     double x_size, double y_size, 
                                     const gridmap_lut_t *lut)
{
    double d = lut->max_dist;
    double x_bound = (x_size / 2.0 ) + d;
    double y_bound = (y_size / 2.0 ) + d;

    double x0 = cx - x_bound, x1 = cx + x_bound;
    double y0 = cy - y_bound, y1 = cy + y_bound;

    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
    
    ix0 = iclamp(ix0, 0, gm->width-1);
    ix1 = iclamp(ix1, 0, gm->width-4);
    iy0 = iclamp(iy0, 0, gm->height-1);
    iy1 = iclamp(iy1, 0, gm->height-1);

    gridmap_to_x_y(gm, ix0, iy0, &x0, &y0);
    gridmap_to_x_y(gm, ix1, iy1, &x1, &y1);

    // offset so that we test the middle of every pixel
    x0 += gm->meters_per_pixel * .5;
    y0 += gm->meters_per_pixel * .5;

    // increment for x after each iteration
    __m128 dxinc4 = _mm_set1_ps(4 * gm->meters_per_pixel);

    // "zero".
    __m128 zero4 = _mm_setzero_ps();

    __m128 xsize4 = _mm_set1_ps(x_size/2);
    __m128 ysize4 = _mm_set1_ps(y_size/2);

    __m128 to_index4 = _mm_set1_ps(lut->size * lut->max_dist_sq_inv);
    __m128 max_index4 = _mm_set1_ps(lut->size - 1);

    double y = y0;
    for (int iy = iy0; iy <= iy1; iy++, y+= gm->meters_per_pixel) {

        // x distance to center of box for the next four pixels
        __m128 dx4 = _mm_setr_ps(x0 - cx, 
                                 x0 - cx + gm->meters_per_pixel, 
                                 x0 - cx + 2*gm->meters_per_pixel, 
                                 x0 - cx + 3*gm->meters_per_pixel);
        
        __m128 dy4 = _mm_set1_ps(y - cy);

        // squared distance in y direction
        __m128 cb4 = _mm_abs_ps(dy4);
        cb4 = _mm_sub_ps(cb4, ysize4);
        cb4 = _mm_max_ps(cb4, zero4);
        cb4 = _mm_mul_ps(cb4, cb4);

        for (int ix = ix0; ix <= ix1; ix+=4) {

            // squared distance in x direction
            __m128 ca4 = _mm_abs_ps(dx4);
            ca4 = _mm_sub_ps(ca4, xsize4);
            ca4 = _mm_max_ps(ca4, zero4);
            ca4 = _mm_mul_ps(ca4, ca4);

            __m128 distsq4 = _mm_add_ps(ca4, cb4);

            __m128 idx4 = _mm_mul_ps(distsq4, to_index4);
            idx4 = _mm_min_ps(idx4, max_index4);

            float idx[4];
            _mm_store_ps(idx, idx4);

            for (int i = 0; i < 4; i++) {
                int oldv = gm->data[iy*gm->width + ix + i];
                int v = lut->table[(int) idx[i]];
                if (v > oldv)
                    gm->data[iy*gm->width + ix + i] = v;
            }

            dx4 = _mm_add_ps(dx4, dxinc4);
        }
    }

    return 0;
}

// this could obviously be special cased to be substantially faster.
int gridmap_render_line_additive(gridmap_t *gm, 
                                 double xy0[2], double xy1[2],
                                 const gridmap_lut_t *lut, 
                                 double lut_scale)
{
    double cx = (xy0[0] + xy1[0])/2;
    double cy = (xy0[1] + xy1[1])/2;
    double x_size = sqrt(sq(xy0[0] - xy1[0]) + sq(xy0[1] - xy1[1]));
    double y_size = 0;
    double theta = atan2(xy1[1] - xy0[1], xy1[0] - xy0[0]);

    return gridmap_render_rectangle_additive(gm, cx, cy, x_size, y_size, theta,
                                             lut, lut_scale);
}

int gridmap_render_rectangle_additive(gridmap_t *gm, 
                                      double cx, double cy, 
                                      double x_size, double y_size, 
                                      double theta,
                                      const gridmap_lut_t *lut,
                                      double lut_scale)
{
    double ux, uy;
    sincos(theta, &uy, &ux);

    double d = lut->max_dist;

    double x_bound = (x_size / 2.0 * fabs(ux) + y_size / 2.0 * fabs(uy)) + d;
    double y_bound = (x_size / 2.0 * fabs(uy) + y_size / 2.0 * fabs(ux)) + d;

    double x0 = cx - x_bound, x1 = cx + x_bound;
    double y0 = cy - y_bound, y1 = cy + y_bound;

    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
    
    ix0 = iclamp(ix0, 0, gm->width-1);
    ix1 = iclamp(ix1, 0, gm->width-4);
    iy0 = iclamp(iy0, 0, gm->height-1);
    iy1 = iclamp(iy1, 0, gm->height-1);

    gridmap_to_x_y(gm, ix0, iy0, &x0, &y0);
    gridmap_to_x_y(gm, ix1, iy1, &x1, &y1);

    // offset so that we test the middle of every pixel
    x0 += gm->meters_per_pixel * .5;
    y0 += gm->meters_per_pixel * .5;

    // increment for x after each iteration
    __m128 dxinc4 = _mm_set1_ps(4 * gm->meters_per_pixel);

    // "zero".
    __m128 zero4 = _mm_setzero_ps();

    __m128 ux4 = _mm_set1_ps(ux);
    __m128 uy4 = _mm_set1_ps(uy);

    __m128 xsize4 = _mm_set1_ps(x_size/2);
    __m128 ysize4 = _mm_set1_ps(y_size/2);

    __m128 to_index4 = _mm_set1_ps(lut->size * lut->max_dist_sq_inv);
    __m128 max_index4 = _mm_set1_ps(lut->size - 1);

    double y = y0;
    for (int iy = iy0; iy <= iy1; iy++, y+= gm->meters_per_pixel) {

        // x distance to center of box for the next four pixels
        __m128 dx4 = _mm_setr_ps(x0 - cx, 
                                 x0 - cx + gm->meters_per_pixel, 
                                 x0 - cx + 2*gm->meters_per_pixel, 
                                 x0 - cx + 3*gm->meters_per_pixel);
        
        __m128 dy4 = _mm_set1_ps(y - cy);
        __m128 dy_ux4 = _mm_mul_ps(dy4, ux4);
        __m128 dy_uy4 = _mm_mul_ps(dy4, uy4);

        for (int ix = ix0; ix <= ix1; ix+=4) {
            
            // how long are the projections of the vector (dx,dy) onto the two
            // principle components of the rectangle? How much longer are they
            // than the dimensions of the rectangle?
            __m128 dx_ux4 = _mm_mul_ps(dx4, ux4);
            __m128 ca4 = _mm_add_ps(dx_ux4, dy_uy4);
            ca4 = _mm_abs_ps(ca4);
            ca4 = _mm_sub_ps(ca4, xsize4);
            ca4 = _mm_max_ps(ca4, zero4);
            ca4 = _mm_mul_ps(ca4, ca4);

            __m128 dx_uy4 = _mm_mul_ps(dx4, uy4);
            __m128 cb4 = _mm_sub_ps(dy_ux4, dx_uy4);
            cb4 = _mm_abs_ps(cb4);
            cb4 = _mm_sub_ps(cb4, ysize4);
            cb4 = _mm_max_ps(cb4, zero4);
            cb4 = _mm_mul_ps(cb4, cb4);

            __m128 distsq4 = _mm_add_ps(ca4, cb4);

            __m128 idx4 = _mm_mul_ps(distsq4, to_index4);
            idx4 = _mm_min_ps(idx4, max_index4);

            float idx[4];
            _mm_store_ps(idx, idx4);

            for (int i = 0; i < 4; i++) {
                int oldv = gm->data[iy*gm->width + ix + i];
                int v = lut_scale * lut->table[(int) idx[i]] + oldv;
                if (v > 255) {
                    gm->data[iy*gm->width + ix + i] = 255;
                } else {
                    gm->data[iy*gm->width + ix + i] = v;
                }
            }

            dx4 = _mm_add_ps(dx4, dxinc4);
        }    }

    return 0;
}

// compute the rate of change in the x and y direction.
void gridmap_gradient(gridmap_t *gm, const double xy[2], double grad[2])
{
    int ix, iy;

    // offset us to the left by half a pixel
    gridmap_to_ix_iy(gm, xy[0] - gm->meters_per_pixel/2, xy[1], &ix, &iy);
    // now: for x direction, sample (ix,iy) and (ix+1, iy)
    if (ix < 0 || ix+1 >= gm->width || iy < 0 || iy >= gm->height) {
        grad[0] = 0;
    } else {
        grad[0] = gm->data[iy*gm->width + ix+1] - gm->data[iy*gm->width + ix];
        grad[0] /= gm->meters_per_pixel;
    }

    // offset us to the left by half a pixel
    gridmap_to_ix_iy(gm, xy[0], xy[1] - gm->meters_per_pixel/2, &ix, &iy);
    // now: for y direction, sample (ix,iy) and (ix, iy+1)
    if (ix < 0 || ix >= gm->width || iy < 0 || iy+1 >= gm->height) {
        grad[1] = 0;
    } else {
        grad[1] = gm->data[(iy+1)*gm->width + ix] - gm->data[iy*gm->width + ix];
        grad[1] /= gm->meters_per_pixel;
    }

}

int gridmap_get_value(const gridmap_t *gm, double x, double y, int fill)
{
    int ix, iy;

    gridmap_to_ix_iy(gm, x, y, &ix, &iy);
    if (ix < 0 || ix >= gm->width || iy < 0 || iy >= gm->width)
        return fill;
    return gm->data[iy*gm->width + ix];
}

void gridmap_set_value(gridmap_t *gm, double x, double y, int v)
{
    int ix, iy;

    gridmap_to_ix_iy(gm, x, y, &ix, &iy);
    if (ix < 0 || ix >= gm->width || iy < 0 || iy >= gm->width)
        return;
    gm->data[iy*gm->width + ix] = v;
}

// Return the minimum value at the corners of the rectangle.
int gridmap_rectangle_corner_min(gridmap_t *gm, double cx, double cy, double sx, double sy)
{
    int xlo, xhi, ylo, yhi;

    gridmap_to_ix_iy(gm, cx-sx/2, cy-sy/2, &xlo, &ylo);
    gridmap_to_ix_iy(gm, cx+sx/2, cy+sy/2, &xhi, &yhi);

    int v00 = gm->data[ylo*gm->width + xlo];
    int v01 = gm->data[ylo*gm->width + xhi];
    int v10 = gm->data[yhi*gm->width + xlo];
    int v11 = gm->data[yhi*gm->width + xhi];

    return imin(imin(v00,v01), imin(v10, v11));
}

int gridmap_set_rectangle(gridmap_t *gm, 
                          double cx, double cy, 
                          double x_size, double y_size, 
                          double theta,
                          uint8_t value)
{
    double ux, uy;
    sincos(theta, &uy, &ux);

    double x_bound = (x_size / 2.0 * fabs(ux) + y_size / 2.0 * fabs(uy));
    double y_bound = (x_size / 2.0 * fabs(uy) + y_size / 2.0 * fabs(ux));

    double x0 = cx - x_bound, x1 = cx + x_bound;
    double y0 = cy - y_bound, y1 = cy + y_bound;

    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
    
    ix0 = iclamp(ix0, 0, gm->width-1);
    ix1 = iclamp(ix1, 0, gm->width-4);
    iy0 = iclamp(iy0, 0, gm->height-1);
    iy1 = iclamp(iy1, 0, gm->height-1);

    gridmap_to_x_y(gm, ix0, iy0, &x0, &y0);
    gridmap_to_x_y(gm, ix1, iy1, &x1, &y1);

    // offset so that we test the middle of every pixel
    x0 += gm->meters_per_pixel * .5;
    y0 += gm->meters_per_pixel * .5;

    // increment for x after each iteration
    __m128 dxinc4 = _mm_set1_ps(4 * gm->meters_per_pixel);

    // "zero".
    __m128 zero4 = _mm_setzero_ps();

    __m128 ux4 = _mm_set1_ps(ux);
    __m128 uy4 = _mm_set1_ps(uy);

    __m128 xsize4 = _mm_set1_ps(x_size/2);
    __m128 ysize4 = _mm_set1_ps(y_size/2);

    double y = y0;
    for (int iy = iy0; iy <= iy1; iy++, y+= gm->meters_per_pixel) {

        // x distance to center of box for the next four pixels
        __m128 dx4 = _mm_setr_ps(x0 - cx, 
                                 x0 - cx + gm->meters_per_pixel, 
                                 x0 - cx + 2*gm->meters_per_pixel, 
                                 x0 - cx + 3*gm->meters_per_pixel);
        
        __m128 dy4 = _mm_set1_ps(y - cy);
        __m128 dy_ux4 = _mm_mul_ps(dy4, ux4);
        __m128 dy_uy4 = _mm_mul_ps(dy4, uy4);

        for (int ix = ix0; ix <= ix1; ix+=4) {
            
            // how long are the projections of the vector (dx,dy) onto the two principle
            // components of the rectangle? How much longer are they than the dimensions
            // of the rectangle?
            __m128 dx_ux4 = _mm_mul_ps(dx4, ux4);
            __m128 ca4 = _mm_add_ps(dx_ux4, dy_uy4);
            ca4 = _mm_abs_ps(ca4);
            ca4 = _mm_sub_ps(ca4, xsize4);
            ca4 = _mm_max_ps(ca4, zero4);
  
            __m128 dx_uy4 = _mm_mul_ps(dx4, uy4);
            __m128 cb4 = _mm_sub_ps(dy_ux4, dx_uy4);
            cb4 = _mm_abs_ps(cb4);
            cb4 = _mm_sub_ps(cb4, ysize4);
            cb4 = _mm_max_ps(cb4, zero4);

            ca4 = _mm_add_ps(ca4, cb4);

            float c[4];
            _mm_store_ps(c, ca4);

            for (int i = 0; i < 4; i++) {
                if (!c[i])
                    gm->data[iy*gm->width + ix + i] = value;
            }

            dx4 = _mm_add_ps(dx4, dxinc4);
        }
    }

    return 0;
}

int gridmap_mask_rectangle(gridmap_t *gm, 
                          double cx, double cy, 
                          double x_size, double y_size, 
                          double theta,
                          uint8_t mask)
{
    double ux, uy;
    sincos(theta, &uy, &ux);

    double x_bound = (x_size / 2.0 * fabs(ux) + y_size / 2.0 * fabs(uy));
    double y_bound = (x_size / 2.0 * fabs(uy) + y_size / 2.0 * fabs(ux));

    double x0 = cx - x_bound, x1 = cx + x_bound;
    double y0 = cy - y_bound, y1 = cy + y_bound;

    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
    
    ix0 = iclamp(ix0, 0, gm->width-1);
    ix1 = iclamp(ix1, 0, gm->width-4);
    iy0 = iclamp(iy0, 0, gm->height-1);
    iy1 = iclamp(iy1, 0, gm->height-1);

    gridmap_to_x_y(gm, ix0, iy0, &x0, &y0);
    gridmap_to_x_y(gm, ix1, iy1, &x1, &y1);

    // offset so that we test the middle of every pixel
    x0 += gm->meters_per_pixel * .5;
    y0 += gm->meters_per_pixel * .5;

    // increment for x after each iteration
    __m128 dxinc4 = _mm_set1_ps(4 * gm->meters_per_pixel);

    // "zero".
    __m128 zero4 = _mm_setzero_ps();

    __m128 ux4 = _mm_set1_ps(ux);
    __m128 uy4 = _mm_set1_ps(uy);

    __m128 xsize4 = _mm_set1_ps(x_size/2);
    __m128 ysize4 = _mm_set1_ps(y_size/2);

    double y = y0;
    for (int iy = iy0; iy <= iy1; iy++, y+= gm->meters_per_pixel) {

        // x distance to center of box for the next four pixels
        __m128 dx4 = _mm_setr_ps(x0 - cx, 
                                 x0 - cx + gm->meters_per_pixel, 
                                 x0 - cx + 2*gm->meters_per_pixel, 
                                 x0 - cx + 3*gm->meters_per_pixel);
        
        __m128 dy4 = _mm_set1_ps(y - cy);
        __m128 dy_ux4 = _mm_mul_ps(dy4, ux4);
        __m128 dy_uy4 = _mm_mul_ps(dy4, uy4);

        for (int ix = ix0; ix <= ix1; ix+=4) {
            
            // how long are the projections of the vector (dx,dy) onto the two principle
            // components of the rectangle? How much longer are they than the dimensions
            // of the rectangle?
            __m128 dx_ux4 = _mm_mul_ps(dx4, ux4);
            __m128 ca4 = _mm_add_ps(dx_ux4, dy_uy4);
            ca4 = _mm_abs_ps(ca4);
            ca4 = _mm_sub_ps(ca4, xsize4);
            ca4 = _mm_max_ps(ca4, zero4);
  
            __m128 dx_uy4 = _mm_mul_ps(dx4, uy4);
            __m128 cb4 = _mm_sub_ps(dy_ux4, dx_uy4);
            cb4 = _mm_abs_ps(cb4);
            cb4 = _mm_sub_ps(cb4, ysize4);
            cb4 = _mm_max_ps(cb4, zero4);

            ca4 = _mm_add_ps(ca4, cb4);

            float c[4];
            _mm_store_ps(c, ca4);

            for (int i = 0; i < 4; i++) {
                if (!c[i])
                    gm->data[iy*gm->width + ix + i]&= mask;
            }

            dx4 = _mm_add_ps(dx4, dxinc4);
        }
    }

    return 0;
}

int gridmap_sum_rectangle (gridmap_t *gm, 
                           double cx, double cy, 
                           double x_size, double y_size, 
                           double theta,
                           int64_t * sum, int64_t * ncells)
{
    double ux, uy;
    sincos(theta, &uy, &ux);

    *sum = 0;
    *ncells = 0;

    double x_bound = (x_size / 2.0 * fabs(ux) + y_size / 2.0 * fabs(uy));
    double y_bound = (x_size / 2.0 * fabs(uy) + y_size / 2.0 * fabs(ux));

    double x0 = cx - x_bound, x1 = cx + x_bound;
    double y0 = cy - y_bound, y1 = cy + y_bound;

    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
    
    ix0 = iclamp(ix0, 0, gm->width-1);
    ix1 = iclamp(ix1, 0, gm->width-4);
    iy0 = iclamp(iy0, 0, gm->height-1);
    iy1 = iclamp(iy1, 0, gm->height-1);

    gridmap_to_x_y(gm, ix0, iy0, &x0, &y0);
    gridmap_to_x_y(gm, ix1, iy1, &x1, &y1);

    // offset so that we test the middle of every pixel
    x0 += gm->meters_per_pixel * .5;
    y0 += gm->meters_per_pixel * .5;

    // increment for x after each iteration
    __m128 dxinc4 = _mm_set1_ps(4 * gm->meters_per_pixel);

    // "zero".
    __m128 zero4 = _mm_setzero_ps();

    __m128 ux4 = _mm_set1_ps(ux);
    __m128 uy4 = _mm_set1_ps(uy);

    __m128 xsize4 = _mm_set1_ps(x_size/2);
    __m128 ysize4 = _mm_set1_ps(y_size/2);

    double y = y0;
    for (int iy = iy0; iy <= iy1; iy++, y+= gm->meters_per_pixel) {

        // x distance to center of box for the next four pixels
        __m128 dx4 = _mm_setr_ps(x0 - cx, 
                                 x0 - cx + gm->meters_per_pixel, 
                                 x0 - cx + 2*gm->meters_per_pixel, 
                                 x0 - cx + 3*gm->meters_per_pixel);
        
        __m128 dy4 = _mm_set1_ps(y - cy);
        __m128 dy_ux4 = _mm_mul_ps(dy4, ux4);
        __m128 dy_uy4 = _mm_mul_ps(dy4, uy4);

        for (int ix = ix0; ix <= ix1; ix+=4) {
            
            // how long are the projections of the vector (dx,dy) onto the two principle
            // components of the rectangle? How much longer are they than the dimensions
            // of the rectangle?
            __m128 dx_ux4 = _mm_mul_ps(dx4, ux4);
            __m128 ca4 = _mm_add_ps(dx_ux4, dy_uy4);
            ca4 = _mm_abs_ps(ca4);
            ca4 = _mm_sub_ps(ca4, xsize4);
            ca4 = _mm_max_ps(ca4, zero4);
  
            __m128 dx_uy4 = _mm_mul_ps(dx4, uy4);
            __m128 cb4 = _mm_sub_ps(dy_ux4, dx_uy4);
            cb4 = _mm_abs_ps(cb4);
            cb4 = _mm_sub_ps(cb4, ysize4);
            cb4 = _mm_max_ps(cb4, zero4);

            ca4 = _mm_add_ps(ca4, cb4);

            float c[4];
            _mm_store_ps(c, ca4);

            for (int i = 0; i < 4; i++) {
                if (!c[i]) {
                    *sum += gm->data[iy*gm->width + ix + i];
                    *ncells += 1;
                }
            }

            dx4 = _mm_add_ps(dx4, dxinc4);
        }
    }

    return 0;
}


/*
  int gridmap_set_rectangle(gridmap_t *gm, 
  double cx, double cy, 
  double x_size, double y_size, 
  double theta, uint8_t value)
  {
  double ux, uy;
  sincos(theta, &uy, &ux);

  double x_bound = (x_size / 2.0 * fabs(ux) + y_size / 2.0 * fabs(uy));
  double y_bound = (x_size / 2.0 * fabs(uy) + y_size / 2.0 * fabs(ux));

  double x0 = cx - x_bound, x1 = cx + x_bound;
  double y0 = cy - y_bound, y1 = cy + y_bound;

  int ix0, iy0, ix1, iy1;
  gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
  gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);
    
  ix0 = iclamp(ix0, 0, gm->width-1);
  ix1 = iclamp(ix1, 0, gm->width-1);
  iy0 = iclamp(iy0, 0, gm->height-1);
  iy1 = iclamp(iy1, 0, gm->height-1);

  gridmap_to_x_y(gm, ix0, iy0, &x0, &y0);
  gridmap_to_x_y(gm, ix1, iy1, &x1, &y1);


  // offset so that we test the middle of every pixel
  x0 += gm->meters_per_pixel * .5;
  y0 += gm->meters_per_pixel * .5;

  double y = y0;
  for (int iy = iy0; iy <= iy1; iy++, y+= gm->meters_per_pixel) {
  double x = x0;
  for (int ix = ix0; ix <= ix1; ix++, x += gm->meters_per_pixel) {
            
  // distances from query point to center of rectangle
  double dx = x - cx, dy = y - cy;

  // how long are the projections of the vector (dx,dy) onto the two principle
  // components of the rectangle? How much longer are they than the dimensions
  // of the rectangle?
  double c1 = fabs(dx * ux + dy * uy) - (x_size / 2);
  double c2 = fabs(- dx * uy + dy * ux) - (y_size / 2);

  // if the projection length is < 0, we're *inside* the rectangle.
  double distsq = 0;
  c1 = max_d(0, c1);
  c2 = max_d(0, c2);
  if (!c1&&!c2)
  gm->data[iy*gm->width+ix] = value;
  }
  }

  return 0;
  }
*/

// ======================= unsigned 16-bit ============================

gridmap_u16_t * 
gridmap_create_u16 (double cx, double cy, double sizex, 
                    double sizey, double meters_per_pixel, uint16_t fill)
{
    gridmap_u16_t *gm = (gridmap_u16_t*) calloc(1, sizeof(gridmap_u16_t));

    gm->meters_per_pixel = meters_per_pixel;
    gm->pixels_per_meter = 1.0/gm->meters_per_pixel;

    // compute pixel dimensions and allocate
    gm->width = sizex/meters_per_pixel;
    gm->height = sizey/meters_per_pixel;
    int datasize = sizeof (uint16_t) * gm->width * gm->height;
    void *data = NULL;
    posix_memalign (&data, 16, datasize);
    gm->data = data;
    for (int i=0; i<gm->height; i++) {
        for (int j=0; j<gm->width; j++) {
            gm->data[i*gm->width+j] = fill;
        }
    }

    // recompute sizex/sizey using exact numbers (due to rounding above)
    gm->sizex = gm->width * meters_per_pixel;
    gm->sizey = gm->height * meters_per_pixel;

    gm->x0 = cx - sizex/2;
    gm->y0 = cy - sizey/2;

    gridmap_align_xy_u16 (gm);

    return gm;
}
void gridmap_destroy_u16 (gridmap_u16_t *gm)
{
    free(gm->data);
    free(gm);
}

#define TWOBYTE_LEN_BITS_U16  1
#define TWOBYTE_COLOR_BITS_U16 (16-TWOBYTE_LEN_BITS_U16)

static inline int 
decode_run_u16 (const uint8_t *data, int *run_color, int *run_length, 
                int last_color)
{
    if (data[0] != 0 || data[1] != 0) {
        int v = (data[0] << 8) | data[1];
        *run_length = (v & ((1 << TWOBYTE_LEN_BITS_U16) - 1)) + 1;
        assert (*run_length <= 2);

        int color_code = v >> TWOBYTE_LEN_BITS_U16;
        if (color_code == 1)
            *run_color = 0;
        else if (color_code == 2)
            *run_color = 65535;
        else {
            *run_color = (color_code - 3) + 
                last_color - (1 << (TWOBYTE_COLOR_BITS_U16 - 1));
        }
        return 2;
    }

    *run_length = (data[2]<<8) | data[3];
    *run_color = (uint16_t) ((data[4]<<8) | data[5]);
    return 6;
}

static inline int 
encode_run_u16 (uint8_t *data, int maxlen, int run_color, int run_length, 
                int last_color)
{
    if (maxlen < 6) {
        printf("%s:%s: not enough buffer space\n", __FILE__, __FUNCTION__);
        return 0;
    }
    assert (run_color >= 0 && run_color < 65536);
    if (!run_length) assert (!run_color);

    if (run_length > 0 && run_length <= (1 << TWOBYTE_LEN_BITS_U16))
    {
        assert (run_length <= 2);
        // two-byte encoding:
        // f e d c b a 9 8 7 6 5 4 3 2 1 0
        //  dcolor                       len

        int dcolor = (run_color - last_color + 
                      (1 << (TWOBYTE_COLOR_BITS_U16 - 1))) & 0xffff;
        if (dcolor < ((1 << TWOBYTE_COLOR_BITS_U16) - 3) || 
            run_color == 0 || run_color == 65535) {

            int color_code;

            // never emit color_code == 0
            if (run_color == 0)
                color_code = 1;
            else if (run_color == 65535)
                color_code = 2;
            else
                color_code = dcolor + 3;

            uint16_t v = 
                (run_length - 1) | (color_code << TWOBYTE_LEN_BITS_U16);
            data[0] = v >> 8;
            data[1] = v & 0xff;
            return 2;
        }
    }

    data[0] = 0;
    data[1] = 0;
    data[2] = run_length >> 8;
    data[3] = run_length & 0xff;
    data[4] = run_color >> 8;
    data[5] = run_color & 0xff;

    return 6;
}

// dimensions must be fully contained within the gridmap.
int gridmap_encode_u16 (const gridmap_u16_t *gm, int xoff, int yoff, 
                        int width, int height, uint8_t *data, int maxlen, int *datalen)
{
    assert(xoff >= 0 && xoff+width <= gm->width);
    assert(yoff >= 0 && yoff+height <= gm->height);
    assert(maxlen >= EOF_LEN);

    int last_color = 0;
    int run_color = -1;
    int run_length = 0;
    int outpos = 0;

    for (int y = yoff; y < yoff+height; y++) {
        for (int x = xoff; x < xoff+width; x++) {

            if (run_color == 0 && (xoff+width-x >=4) && 
                (run_length + 4) <= MAX_RUN_LENGTH) {
                int64_t *p = (int64_t*) &gm->data[y*gm->width+x];
                if (*p==0) {
                    run_length+=4;
                    x+=3;
                    continue;
                }
            }

            int color = gm->data[y*gm->width + x];

            // add to existing run?
            if (color == run_color && (run_length + 1 ) <= MAX_RUN_LENGTH) {
                run_length++;
                continue;
            }

            // emit previous run (save room for EOF)
            if (run_length) {
                outpos += encode_run_u16 (&data[outpos], 
                                          maxlen - outpos - EOF_LEN, run_color, run_length, 
                                          last_color);
                last_color = run_color;
            }

            // start a new run
            run_color = color;
            run_length = 1;
        }
    }

    if (run_length) {
        outpos += encode_run_u16 (&data[outpos], maxlen - outpos - EOF_LEN, 
                                  run_color, run_length, last_color);
        last_color = run_color;
    }

    // EOF
    outpos += encode_run_u16 (&data[outpos], maxlen - outpos, 0, 0, 0);

    *datalen = outpos;

    return 0;
}

// The tile can have any amount of overlap with the gridmap.
int gridmap_decode_u16 (gridmap_u16_t *gm, int xoff, int yoff, int width, 
                        int height, uint8_t *data, int datalen)
{
    int x = xoff, y = yoff;
    int inpos = 0;
    int last_color = 0;

    // none of the tile overlaps?
    if (xoff + width < 0 || yoff + height < 0 || 
        xoff >= gm->width || yoff >= gm->height)
        return 0;

    while (inpos < datalen) {
        int run_color, run_length;

        inpos += decode_run_u16 (&data[inpos], &run_color, &run_length, 
                                 last_color);
        last_color = run_color;

        if (run_length == 0) { // EOF?
            return 0;
        }

        while (run_length) {

            // no more data, exit.
            if (y >= gm->height)
                return 0;

            int use_length = 0;

            if (y < 0) {
                // drop (up to) the rest of the row.
                use_length = imin(run_length, xoff + width - x);
            } else if (x < 0) {
                // drop (up to) the beginning of the row.
                use_length = imin(run_length, -x);
            } else if (x >= gm->width) {
                // drop (up to) the rest of the row.
                use_length = imin(run_length, xoff + width - x);
            } else {
                // data is *in bounds*. Copy up to the remainder of the row.
                use_length = imin(run_length, xoff + width - x);
                use_length = imin(use_length, gm->width - x);

                for (int i=0; i<use_length; i++) {
                    gm->data[y*gm->width+x+i] = run_color;
                }
            }

            run_length -= use_length;
            x+= use_length;

            if (x == xoff + width) {
                x = xoff;
                y++;
                if (y >= yoff + height && run_length) {
                    printf("%s: run-on decode\n", __FUNCTION__);
                    return -1;
                }
            }
        }
    }

    return 0;
}

int 
gridmap_get_pixels_aligned_u16 (gridmap_u16_t *gm, int xoff, int yoff, 
                                int width, int height, uint16_t *data, uint16_t fill)
{
    int outidx = 0;

    int max_x = xoff + width;

    // how much of the request falls outside the gridmap? (x component)
    int out_size = max_x - gm->width;
    if (out_size < 0)
        out_size = 0;

    // how much of the request falls inside the gridmap? (x component)
    int in_size = width - out_size;

    // iterate over rows...
    for (int y = yoff; y < height + yoff; y++) {

        if (y < 0 || y >= gm->height) {
            // this whole row is out of bounds
            for (int i=0; i<width; i++) data[outidx + i] = fill;
            outidx += width;
        } else {
            // the row is in bounds
            memcpy(&data[outidx], &gm->data[y*gm->width + xoff], 
                   in_size * sizeof (uint16_t));
            outidx += in_size;
            for (int i=0; i<out_size; i++) data[outidx + i] = fill;
            outidx += out_size;
        }
    }

    return 0;
}

int 
gridmap_recenter_u16 (gridmap_u16_t *gm, double cx, double cy, 
                      double max_distance, uint16_t fill)
{
    double distance_sq = sq(cx - gm->cx) + sq(cy-gm->cy);
    if (distance_sq < sq(max_distance))
        return 0;

    // how far (in pixels) to move. We round to an exact pixel boundary.
    // e.g., dx = source - dest
    int dx = (cx - gm->cx) * gm->pixels_per_meter;
    int dy = (cy - gm->cy) * gm->pixels_per_meter;

//    printf("RECENTER %15f %5d %5d\n", distance_sq, dx, dy);

    if (dy >= 0) {
        for (int dest = 0; dest < gm->height; dest++) {
            
            int src = dest + dy;

            if (src < 0 || src >= gm->height) {
                int sz = gm->width * sizeof (uint16_t);
                memset(&gm->data[dest*gm->width], fill, sz);
                continue;
            } 
            
            if (dx >= 0) {
                int sz = imax(0, gm->width - dx);
                memmove(&gm->data[dest*gm->width], 
                        &gm->data[src*gm->width + dx], sz);
                for (int i=0; i<gm->width - sz; i++) 
                    gm->data[dest*gm->width + sz + i] = fill;
            } else {
                int sz = imax(0, gm->width + dx);
                memmove(&gm->data[dest*gm->width - dx], 
                        &gm->data[src*gm->width], sz);
                for (int i=0; i<gm->width - sz; i++) 
                    gm->data[dest*gm->width + i] = fill;
            }
        }
    } else {
        for (int dest = gm->height-1; dest >= 0; dest--) {
             
            int src = dest + dy;

            if (src < 0 || src >= gm->height) {
                int sz = gm->width * sizeof (uint16_t);
                memset(&gm->data[dest*gm->width], fill, sz);
                continue;
            } 
                      
            if (dx >= 0) {
                int sz = imax(0, gm->width - dx) * sizeof (uint16_t);
                memmove(&gm->data[dest*gm->width], 
                        &gm->data[src*gm->width + dx], sz);
                for (int i=0; i<gm->width - sz; i++) 
                    gm->data[dest*gm->width + sz + i] = fill;
            } else {
                int sz = imax(0, gm->width + dx) * sizeof (uint16_t);
                memmove(&gm->data[dest*gm->width - dx], 
                        &gm->data[src*gm->width], sz);
                for (int i=0; i<gm->width - sz; i++) 
                    gm->data[dest*gm->width + i] = fill;
            }
        }
    }

    gm->cx += dx * gm->meters_per_pixel;
    gm->cy += dy * gm->meters_per_pixel;
    gm->x0 += dx * gm->meters_per_pixel;
    gm->y0 += dy * gm->meters_per_pixel;

    return 0;
}


// Take the max of each point from two gridmaps
/*
// too many assumptions, don't use me.

int gridmap_combine_max(gridmap_t *dst, const gridmap_t *src1, const gridmap_t *src2) 
{
if ((src1->width!=src2->width)||(src1->height!=src2->height)||
(dst->width!=src1->width)||(dst->height!=src1->height)) {
printf("gridmap_combine_max(): ERROR: src and dst not same size!");
return -1;
}
#if 0
for (int j=0;j<dst->height; j++) {
for (int i=0;i<dst->width-16; i+=16) {
__m128i s1 = _mm_load_si128 ((__m128i *)(src1->data+j*src1->width+i));
__m128i s2 = _mm_load_si128 ((__m128i *)(src2->data+j*src2->width+i));
__m128i d =  _mm_max_epu8(s1, s2);
_mm_store_si128 ((__m128i *)(dst->data+j*dst->width+i), d);
}
for (int i=dst->width-16;i<dst->width; i++)
*(dst->data+j*dst->width+i)=imax(*(src1->data+j*src1->width+i),*(src2->data+j*src2->width+i));
}
#else
for (int i=0; i<dst->height; i++) {
uint8_t *src_row1 = &src1->data[i*src1->width];
uint8_t *src_row2 = &src2->data[i*src2->width];
uint8_t *dst_row = &dst->data[i*dst->width];
for (int j=0; i<dst->width; i++) {
dst_row[j] = imax (src_row1[j], src_row2[j]);
}
}
#endif
return 0;
}
*/

 /** combine the gridmap src into the current gridmap gm. Each
  * gridcell is the max(orig value, src_value * src_scale) 
  **/

 // naive reference implementation:
 /*
   int gridmap_max_scale(gridmap_t *gm, const gridmap_t *src, double src_scale)
   {
    
   int ix0 = 0, ix1 = gm->width - 1;
   int iy0 = 0, iy1 = gm->height - 1;
 
   for (int iy = iy0; iy <= iy1; iy++) {

   double x, y;
   gridmap_to_x_y(gm, ix0, iy, &x, &y);
        
   // sample from center of cell
   x += gm->meters_per_pixel / 2;
   y += gm->meters_per_pixel / 2;
        
   for (int ix = ix0; ix <= ix1; ix++) {

   int orig_value = gm->data[iy * gm->width + ix];
   int src_value = gridmap_get_value(src, x, y, 0);

   gm->data[iy * gm->width + ix] = imax(orig_value, src_value * src_scale);
   x += gm->meters_per_pixel;
   }
   }
   }
 */

int gridmap_max_scale(gridmap_t *gm, const gridmap_t *src, double src_scale)
{
    // process over the intersection of the two gridmaps. fudge upper
    // bound by a pixel 'cuz I don't feel like figuring out whether we
    // need to or not in order to avoid running past the buffer.

    double margin = 2*fmax(src->meters_per_pixel, gm->meters_per_pixel);

    double x0 = fmax(gm->x0, src->x0);
    double x1 = fmin(gm->x0 + gm->sizex, src->x0 + src->sizex) - margin;

    double y0 = fmax(gm->y0, src->y0);
    double y1 = fmin(gm->y0 + gm->sizey, src->y0 + src->sizey) - margin;

    // find the index coordinates for gm. we don't need to clamp, of
    // course.
    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);

    // if we step one pixel in gm, how many pixels (in floating precision) do we step in src?
    double src_step = gm->meters_per_pixel / src->meters_per_pixel;

    for (int iy = iy0; iy <= iy1; iy++) {

        double x, y;
        gridmap_to_x_y(gm, ix0, iy, &x, &y);
        
        // sample from center of cell
        x += gm->meters_per_pixel / 2;
        y += gm->meters_per_pixel / 2;

        // like gridmap_to_ix_iy, except we want floating point precision for the indices
        double src_x = (x - src->x0) * src->pixels_per_meter;
        double src_y = (y - src->y0) * src->pixels_per_meter;

        for (int ix = ix0; ix <= ix1; ix++) {

            int orig_value = gm->data[iy * gm->width + ix];
            int src_value = src->data[((int) src_y) * src->width + ((int) src_x)];

            gm->data[iy * gm->width + ix] = imax(orig_value, imin(255,src_value * src_scale));

            src_x += src_step;
        }
    }

    return 0;
}

// Compute the maximum value of src and gm, storing the result in
// gm. The restrictedness of gm is preserved.
int gridmap_max_scale_restricted(gridmap_t *gm, const gridmap_t *src, double src_scale)
{
    // process over the intersection of the two gridmaps. fudge upper
    // bound by a pixel 'cuz I don't feel like figuring out whether we
    // need to or not in order to avoid running past the buffer.

    double margin = 2*fmax(src->meters_per_pixel, gm->meters_per_pixel);

    double x0 = fmax(gm->x0, src->x0);
    double x1 = fmin(gm->x0 + gm->sizex, src->x0 + src->sizex) - margin;

    double y0 = fmax(gm->y0, src->y0);
    double y1 = fmin(gm->y0 + gm->sizey, src->y0 + src->sizey) - margin;

    // find the index coordinates for gm. we don't need to clamp, of
    // course.
    int ix0, iy0, ix1, iy1;
    gridmap_to_ix_iy(gm, x0, y0, &ix0, &iy0);
    gridmap_to_ix_iy(gm, x1, y1, &ix1, &iy1);

    // if we step one pixel in gm, how many pixels (in floating precision) do we step in src?
    double src_step = gm->meters_per_pixel / src->meters_per_pixel;

    for (int iy = iy0; iy <= iy1; iy++) {

        double x, y;
        gridmap_to_x_y(gm, ix0, iy, &x, &y);
        
        // sample from center of cell
        x += gm->meters_per_pixel / 2;
        y += gm->meters_per_pixel / 2;

        // like gridmap_to_ix_iy, except we want floating point precision for the indices
        double src_x = (x - src->x0) * src->pixels_per_meter;
        double src_y = (y - src->y0) * src->pixels_per_meter;

        for (int ix = ix0; ix <= ix1; ix++) {
            int orig_value = gm->data[iy * gm->width + ix];
            int src_value = src->data[((int) src_y) * src->width + ((int) src_x)];

            src_value = (int) (src_scale * src_value);
            int new_value = imax(orig_value&0xfe, imin(254,src_value) & 0xfe);
            int new_restricted = (orig_value&0x01); // | (src_value&0x01);
            gm->data[iy * gm->width + ix] = new_value | new_restricted;

            src_x += src_step;
        }
    }

    return 0;
}

void gridmap_scale(gridmap_t *gm, double s)
{
    for (int iy = 0 ; iy < gm->height; iy++) {
        for (int ix = 0; ix < gm->width; ix++) {
            int oldvalue = gm->data[iy*gm->width + ix];
            gm->data[iy*gm->width+ix] = oldvalue * s;
        }
    }
}

// this could obviously be special cased to be substantially faster.
int gridmap_render_line_restricted(gridmap_t *gm, 
                                   double xy0[2], double xy1[2],
                                   const gridmap_lut_t *lut)
{
    double cx = (xy0[0] + xy1[0])/2;
    double cy = (xy0[1] + xy1[1])/2;
    double x_size = sqrt(sq(xy0[0] - xy1[0]) + sq(xy0[1] - xy1[1]));
    double theta = atan2(xy1[1] - xy0[1], xy1[0] - xy0[0]);
    return gridmap_render_rectangle_restricted(gm, cx, cy, x_size, 0.0, theta, lut);
}

void gridmap_polygon_fill(gridmap_t *gm, polygon2d_t *polyd, int fill)
{
    polygon2i_t *polyi = polygon2i_new();

    for (int listidx = 0; listidx < polyd->nlists; listidx++) {
        pointlist2d_t *listd = &polyd->pointlists[listidx];
        pointlist2i_t *listi = pointlist2i_new(listd->npoints);
        
        for (int pidx = 0; pidx < listd->npoints; pidx++) {
            double x = listd->points[pidx].x;
            double y = listd->points[pidx].y;
            int ix, iy;
            gridmap_to_ix_iy(gm, x, y, &ix, &iy);
            listi->points[pidx].x = ix;
            listi->points[pidx].y = iy;
            
//            listi->points[pidx].x = iclamp(ix, 0, gm->width-1);
//            listi->points[pidx].y = iclamp(iy, 0, gm->height-1);
        }
        
        polygon2i_add_pointlist(polyi, listi);
        pointlist2i_free (listi);
    }
    
    pointlist2i_t *interior = geom_compute_polygon_covered_points_2i(polyi);
    polygon2i_free(polyi);
    
    if (!interior)
        return;
    
    for (int i = 0; i < interior->npoints; i++) {
        int ix = interior->points[i].x;
        int iy = interior->points[i].y;
        
        if (ix >= 0 && ix < gm->width && iy >= 0 && iy < gm->height) {
            gm->data[iy*gm->width + ix] = fill;
        }
    }

    pointlist2i_free(interior);
}

void gridmap_polygon_fill_min_restricted(gridmap_t *gm, polygon2d_t *polyd, int fill)
{
    polygon2i_t *polyi = polygon2i_new();

    for (int listidx = 0; listidx < polyd->nlists; listidx++) {
        pointlist2d_t *listd = &polyd->pointlists[listidx];
        pointlist2i_t *listi = pointlist2i_new(listd->npoints);
        
        for (int pidx = 0; pidx < listd->npoints; pidx++) {
            double x = listd->points[pidx].x;
            double y = listd->points[pidx].y;
            int ix, iy;
            gridmap_to_ix_iy(gm, x, y, &ix, &iy);
            listi->points[pidx].x = ix;
            listi->points[pidx].y = iy;
            
            listi->points[pidx].x = iclamp(ix, 0, gm->width-1);
            listi->points[pidx].y = iclamp(iy, 0, gm->height-1);
        }
        
        polygon2i_add_pointlist(polyi, listi);
        pointlist2i_free (listi);
    }
    
    pointlist2i_t *interior = geom_compute_polygon_covered_points_2i(polyi);
    polygon2i_free(polyi);
    
    if (!interior) 
        return;
    
    for (int i = 0; i < interior->npoints; i++) {
        int ix = interior->points[i].x;
        int iy = interior->points[i].y;
        
        if (ix >= 0 && ix < gm->width && iy >= 0 && iy < gm->height) {
            int oldv = gm->data[iy*gm->width + ix];
            int newv = imin(fill, oldv) & 0xfe;
            newv |= (oldv & fill & 1);
            gm->data[iy*gm->width + ix] = newv;
        }
    }

    pointlist2i_free(interior);
}

void gridmap_polygon_render_min(gridmap_t *gm, polygon2d_t *polyd, gridmap_lut_t *lut)
{
    for (int listidx = 0; listidx < polyd->nlists; listidx++) {
        pointlist2d_t *listd = &polyd->pointlists[listidx];

        for (int pidx = 0; pidx < listd->npoints; pidx++) {
            double xy0[] = { listd->points[pidx].x,
                             listd->points[pidx].y };
            double xy1[] = { listd->points[(pidx+1) % listd->npoints].x,
                             listd->points[(pidx+1) % listd->npoints].y };

            gridmap_render_line_min(gm, xy0, xy1, lut);
        }
    }
}

gridmap_t *gridmap_create_max_restricted(const gridmap_t *a, const gridmap_t *b)
{
    assert(a->width==b->width);
    assert(a->height == b->height);
    assert(a->meters_per_pixel == b->meters_per_pixel);
    assert(a->x0 == b->x0);
    assert(a->y0 == b->y0);

    gridmap_t *dst = gridmap_create_compatible(a);
/*
    assert(dst->x0 == a->x0);
    assert(dst->y0 == a->y0);
*/
    for (int iy = 0; iy < a->height; iy++) {
        for (int ix = 0; ix < a->width; ix++) {

            int av = a->data[iy*a->width + ix];
            int bv = b->data[iy*a->width + ix];

            int v = imax(av, bv);
            v |= ((av | bv) & 1);

            dst->data[iy*a->width + ix] = v;
        }
    }

    return dst;
}
