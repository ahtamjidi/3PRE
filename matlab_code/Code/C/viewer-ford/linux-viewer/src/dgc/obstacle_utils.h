#ifndef __obstacle_utils_h__
#define __obstacle_utils_h__

#include <glib.h>

#include <dgc/ctrans.h>
#include "cam_fov_test.h"
#include <common/geometry.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @pointlist: should be a pointlist2d_t of length at least 4
 *
 * Convenience function to compute the actual points of the obstacle boundary
 * in the localframe.
 *
 * Returns: 0 on success, -1 if the pointlist is not of length at least 4
 */
int orutil_obstacle_to_pointlist2d (const double x0, const double y0,
                                    const float sx, const float sy,
                                    const float theta,
                                    pointlist2d_t *pointlist);

/**
 * Convenience function to compute the actual points of the obstacle boundary
 * in the localframe.
 */
static inline pointlist2d_t * 
orutil_obstacle_to_new_pointlist2d (const double x0, const double y0,
                                    const float sx, const float sy,
                                    const float theta)
{
    pointlist2d_t *result = pointlist2d_new (4);
    orutil_obstacle_to_pointlist2d (x0, y0, sx, sy, theta, result);
    return result;
}

/**
 * result should be a dynamically allocated pointlist2d_t.  It's internal point
 * allocation may change as a result of this function.
 */
pointlist2d_t * orutil_project_obstacle_to_image (const double x0, 
        const double y0, const double z0, 
        const float sx, const float sy, const float sz,
        const float theta,
        const double to_camera[16],
        CamTrans *camtrans,
        CamFOVTester *fov_tester);

#ifdef __cplusplus
}
#endif

#endif
