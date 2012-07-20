#ifndef _GRIDMAP_H
#define _GRIDMAP_H

#include <stdint.h>

#include <common/geometry.h>

typedef struct
{
    int *table;
    int size;
    double max_dist;
    double max_dist_sq;
    double max_dist_sq_inv;
} gridmap_lut_t;

gridmap_lut_t *gridmap_lut_create_constant(int table_size, double max_dist, int inside_value, int outside_value);

gridmap_lut_t *gridmap_lut_create_cliff(int table_size, double max_dist, double cliff_distance, double decay, int max_value);
gridmap_lut_t *gridmap_lut_create_cliff_restricted_increasing(int table_size, double max_dist, double cliff_distance, double restricted_distance, 
                                                              double decay, int min_value, int max_value, int outside_value);
gridmap_lut_t *gridmap_lut_create_cliff_restricted_cos(int table_size, double max_dist, double cliff_distance, double restricted_distance, 
                                                       double max_phase, int max_value, int outside_value);


void gridmap_lut_destroy(gridmap_lut_t *lut);

typedef struct gridmap gridmap_t;
struct gridmap
{
    double cx, cy;              // center x,y
    double x0, y0;              // minimum x,y
    double sizex, sizey;
    double meters_per_pixel;
    double pixels_per_meter;

    int width, height;
    uint8_t *data;
};

gridmap_t *gridmap_create(double cx, double cy, double sizex, double sizey, 
                          double meters_per_pixel);

gridmap_t *gridmap_create_fill(double cx, double cy, double sizex, double sizey, 
                               double meters_per_pixel, int fill);

/**
 * gridmap_create_compatible:
 *
 * constructs a new gridmap with the same attributes as %src, but with data set 
 * to zero.
 */
gridmap_t *gridmap_create_compatible(const gridmap_t *src);

/**
 * gridmap_new_copy:
 *
 * constructs a copy of %src.
 */
gridmap_t *gridmap_new_copy (const gridmap_t *src);

void gridmap_nearest_bucket_center(gridmap_t *gm, double x, double y, double *ox, double *oy);

void gridmap_scale(gridmap_t *gm, double s);

void gridmap_destroy(gridmap_t *gm);

// Return the minimum value at the corners of the rectangle.
int gridmap_rectangle_corner_min(gridmap_t *gm, double cx, double cy, double sx, double sy);

void gridmap_draw_rectangle(gridmap_t *gm, double x0, double y0, double sizex, double sizey, uint8_t color);
void gridmap_draw_rectangle_max(gridmap_t *gm, double x0, double y0, double sizex, double sizey, uint8_t color);
// fill all of the cells that the rectangle touches. a value of 1
// takes precedence over any other value.
void gridmap_draw_rectangle_max_special(gridmap_t *gm, double x0, double y0, double sizex, double sizey, uint8_t color);

int gridmap_expand_circle(gridmap_t *dst, const gridmap_t *src, double diameter);

// WARNING: verrrry sllloooowww only use on small gridmaps
int gridmap_expand_max_circle(gridmap_t *dst, const gridmap_t *src, double diameter);
int gridmap_expand_max_circle_ex(gridmap_t *dst, const gridmap_t *src, double diameter_meters,
                                 int xidx, int yidx, int xwidth, int ywidth);

void gridmap_reset(gridmap_t *gm, double cx, double cy);
int gridmap_encode(const gridmap_t *gm, int xoff, int yoff, int width, int height, uint8_t *data, int maxlen, int *datalen);
int gridmap_decode(gridmap_t *gm, int xoff, int yoff, int width, int height, uint8_t *data, int datalen);
int gridmap_decode_base(uint8_t *dest, int width, int height, uint8_t *src, int datalen);
/** 
 * gridmap_recenter:
 *
 * recenter the gridmap if the new (cx,cy) is farther than max_distance from
 * the current center.  New pixels in the gridmap are set to value %fill
 */
int gridmap_recenter(gridmap_t *gm, double cx, double cy, double max_distance, 
        uint8_t fill);

int gridmap_expand_circle_ex(gridmap_t *dst, const gridmap_t *src, double diameter_meters,
                             int xidx, int yidx, int xwidth, int ywidth);

int gridmap_search_non_zero(gridmap_t *gm, double x0, double y0, double x1, double y1);
int gridmap_get_pixels_aligned(gridmap_t *gm, int xoff, int yoff, int width, int height, uint8_t *data, uint8_t fill);

// integrate the values along a line, returning -1 if a value of 255 was encountered. Returned value
// is normalized with respect to distance. (i.e., a path of 1 meter through a cost field of 37 yields 37.
double gridmap_test_line(gridmap_t *gm, double x0, double y0, double x1, double y1);



// Computes integral,average and max for test line. NULLs for integral average and max are okay.
// NOTE: Stops early and returns -1 if we see 255
// integral is same as above
// average is the integral / length
// max is max value encountered.
int gridmap_test_line2(gridmap_t *gm, double *integral, double *average, uint8_t *max, 
                       double x0, double y0, double x1, double y1);


void gridmap_line_segment_stats (gridmap_t *gm, double *integral, 
        double *average, uint8_t *max, uint8_t *min, 
        double x0, double y0, double x1, double y1);

static inline void gridmap_to_ix_iy(const gridmap_t *gm, double x, double y, 
        int *ix, int *iy)
{
    *ix = (x - gm->x0) * gm->pixels_per_meter;
    *iy = (y - gm->y0) * gm->pixels_per_meter;
}

static inline void gridmap_to_x_y(const gridmap_t *gm, int ix, int iy, 
        double *x, double *y)
{
    *x = ix * gm->meters_per_pixel + gm->x0;
    *y = iy * gm->meters_per_pixel + gm->y0;
}

int gridmap_render_rectangle(gridmap_t *gm, 
                             double cx, double cy, 
                             double x_size, double y_size, 
                             double theta,
                             const gridmap_lut_t *lut);


int gridmap_render_line(gridmap_t *gm, 
                        double xy0[2], double xy1[2],
                        const gridmap_lut_t *lut);

int gridmap_render_line_min(gridmap_t *gm, 
                            double xy0[2], double xy1[2],
                            const gridmap_lut_t *lut);

int gridmap_render_line_min_keep_infeasible(gridmap_t *gm, 
                                            double xy0[2], double xy1[2],
                                            const gridmap_lut_t *lut);

int gridmap_render_rectangle_min_keep_infeasible(gridmap_t *gm, 
                                                 double cx, double cy, 
                                                 double x_size, double y_size, 
                                                 double theta,
                                                 const gridmap_lut_t *lut);

int gridmap_render_rectangle_min(gridmap_t *gm, 
                                 double cx, double cy, 
                                 double x_size, double y_size, 
                                 double theta,
                                 const gridmap_lut_t *lut);


// same as gridmap_render_rectangle, but the new value at a pixel of the
// gridmap is added to the previous value instead of taking the max.
int gridmap_render_rectangle_additive (gridmap_t *gm, 
                                       double cx, double cy, 
                                       double x_size, double y_size, 
                                       double theta,
                                       const gridmap_lut_t *lut,
                                       double lut_scale);

// same as gridmap_render_line, but the new value at a pixel of the
// gridmap is added to the previous value instead of taking the max.
int gridmap_render_line_additive(gridmap_t *gm,
                                 double xy0[2], double xy1[2],
                                 const gridmap_lut_t *lut,
                                 double lut_scale);

void gridmap_gradient(gridmap_t *gm, const double xy[2], double grad[2]);

int gridmap_get_value(const gridmap_t *gm, double x, double y, int fill);
void gridmap_set_value(gridmap_t *gm, double x, double y, int v);

int gridmap_set_rectangle(gridmap_t *gm, 
                          double cx, double cy, 
                          double x_size, double y_size, 
                          double theta, uint8_t value);

/* Returns the sum of the contents of the gridmap inside a rectangle */
int gridmap_sum_rectangle (gridmap_t *gm, 
                           double cx, double cy, 
                           double x_size, double y_size, 
                           double theta,
                           int64_t * sum, int64_t * ncells);

// mark-related functions help implement an history occupancy grid. The value of the
// grid is stored in bits 7-1; bit 0 is used internally.
// "mark" registers that there was a hit by setting bit 0.
// "update" visits all buckets: if bit 0 is set, the value is increased by the given amount, else it is
// decreased by the given amount. The low bit is then cleared.

// Set the LSB of each cell within the rectangle
// returns 1 if any part of the rectangle was previously 0.
int gridmap_mark_rectangle(gridmap_t *gm, double x0, double y0, double sizex, double sizey);


void gridmap_mark_update(gridmap_t *gm, int increment, int decrement);

// because sometimes, 8 bits of precision just isn't enough...

typedef struct gridmap_u16 gridmap_u16_t;
struct gridmap_u16
{
    double cx, cy;              // center x,y
    double x0, y0;              // minimum x,y
    double sizex, sizey;
    double meters_per_pixel;
    double pixels_per_meter;

    int width, height;
    uint16_t *data;
};

gridmap_u16_t * gridmap_create_u16 (double cx, double cy, double sizex, 
        double sizey, double meters_per_pixel, uint16_t fill);
void gridmap_destroy_u16 (gridmap_u16_t *gm);
int gridmap_encode_u16 (const gridmap_u16_t *gm, int xoff, int yoff, int width, 
        int height, uint8_t *data, int maxlen, int *datalen);
int gridmap_decode_u16 (gridmap_u16_t *gm, int xoff, int yoff, int width, 
        int height, uint8_t *data, int datalen);

static inline void gridmap_to_ix_iy_u16 (const gridmap_u16_t *gm, 
        double x, double y, int *ix, int *iy)
{
    *ix = (x - gm->x0) * gm->pixels_per_meter;
    *iy = (y - gm->y0) * gm->pixels_per_meter;
}

static inline void gridmap_to_x_y_u16 (const gridmap_u16_t *gm, int ix, int iy, 
        double *x, double *y)
{
    *x = ix * gm->meters_per_pixel + gm->x0;
    *y = iy * gm->meters_per_pixel + gm->y0;
}

int 
gridmap_get_pixels_aligned_u16 (gridmap_u16_t *gm, int xoff, int yoff, 
        int width, int height, uint16_t *data, uint16_t fill);
/** 
 * gridmap_recenter:
 *
 * recenter the gridmap if the new (cx,cy) is farther than max_distance from
 * the current center.  New pixels in the gridmap are set to value %fill
 */
int gridmap_recenter_u16 (gridmap_u16_t *gm, double cx, double cy, 
        double max_distance, uint16_t fill);


//int gridmap_combine_max(gridmap_t *dst, const gridmap_t *src1, const gridmap_t *src2);

/** combine the gridmap src into the current gridmap gm. Each
 * gridcell is the max(orig value, src_value * src_scale) 
 **/
int gridmap_max_scale(gridmap_t *gm, const gridmap_t *src, double src_scale);



////////////////////////////////////////////////////////////////////////////////////
// restricted functions: these treat the low bit as a special "restricted" flag.
// cost values still range from 0-254, but are always even. (Cost values >= 254 are
// infeasible.)

int gridmap_render_rectangle_restricted(gridmap_t *gm, 
                                        double cx, double cy, 
                                        double x_size, double y_size, 
                                        double theta,
                                        const gridmap_lut_t *lut);


int gridmap_render_line_restricted(gridmap_t *gm, 
                                   double xy0[2], double xy1[2],
                                   const gridmap_lut_t *lut);

// ranges < restricted_distance will be restricted.
gridmap_lut_t *gridmap_lut_create_cliff_restricted(int table_size, double max_dist, 
                                                   double cliff_distance, 
                                                   double restricted_distance, 
                                                   double decay, int max_value, int outside_value);

/** combine the gridmap src into the current gridmap gm. Each
 * gridcell is the max(orig value, src_value * src_scale) 
 **/
int gridmap_max_scale_restricted(gridmap_t *gm, const gridmap_t *src, double src_scale);


// integrate the cost and restricted bit over the line segment. The
// result is not well-defined if the line does not lie within the
// gridmap.
void gridmap_test_line_restricted(gridmap_t *gm, double x0, double y0, double x1, double y1,
                                  double *cost, double *restricted);

void gridmap_test_line_restricted_worst(gridmap_t *gm, double x0, double y0, double x1, double y1,
                                        int *cost, int *restricted);

void gridmap_polygon_fill(gridmap_t *gm, polygon2d_t *polyd, int fill);
void gridmap_polygon_fill_min_restricted(gridmap_t *gm, polygon2d_t *polyd, int fill);

void gridmap_polygon_render_min(gridmap_t *gm, polygon2d_t *polyd, gridmap_lut_t *lut);

gridmap_t *gridmap_create_max_restricted(const gridmap_t *a, const gridmap_t *b);
void gridmap_fill(gridmap_t *gm, int fill);

gridmap_lut_t *gridmap_lut_create_cliff_restricted_linear(int table_size, double max_dist, double cliff_distance, double restricted_distance, int max_value, int outside_value);

#endif
