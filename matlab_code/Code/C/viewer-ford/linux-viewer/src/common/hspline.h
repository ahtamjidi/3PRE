#ifndef __HERMITE_SPLINE_H__
#define __HERMITE_SPLINE_H__

typedef struct _hspline_point {
    double pos[2];
    double tan[2];
} hspline_point_t;

/**
 * hspline_sample_segment_points_autoscale:
 *
 * Resamples points along the length of a single segment (between two
 * control points) of a Hermite cubic spline.  Automatically scales the 
 * tangents of the control points on each spline segment by the distance
 * between the control points.
 *
 * @p1:          Starting control point
 * @p2:          Ending control point
 * @x:           Output array of resampled x-coordinates
 * @y:           Output array of resampled y-coordinates
 * @num_samples: Number of output points to sample
 * @scale:       Scale factor applied to the tangents
 *
 * Returns: 0 on success, -1 on failure.
 */
int hspline_sample_segment_points (const hspline_point_t * p1,
        const hspline_point_t * p2,
        double * x, double * y, int num_samples, double scale);

/**
 * hspline_sample_points:
 *
 * Resamples points along the length of a Hermite cubic spline.  
 *
 * @sp:          Array of control points that define the spline
 * @num_control: Number of control points in the definition
 * @x:           Output array of resampled x-coordinates
 * @y:           Output array of resampled y-coordinates
 * @num_output:  Number of output points to sample
 *
 * Returns: 0 on success, -1 on failure.
 */
int hspline_sample_points (const hspline_point_t *sp,
        int num_control, double *x, double *y, int num_output);

/**
 * hspline_sample_points:
 *
 * Resamples points along the length of a Hermite cubic spline.  When sampling
 * a segment, automatically scales the tangent vectors of the control points by
 * the distance between the control points.  This is a cheap hack to produce
 * reasonably smooth curves with unit length tangent vectors.
 *
 * @sp:          Array of control points that define the spline
 * @num_control: Number of control points in the definition
 * @x:           Output array of resampled x-coordinates
 * @y:           Output array of resampled y-coordinates
 * @num_output:  Number of output points to sample
 *
 * Returns: 0 on success, -1 on failure.
 */
int hspline_sample_points_autoscale  (const hspline_point_t * sp, 
        int num_control, double * x, double * y, int num_output);

#endif
