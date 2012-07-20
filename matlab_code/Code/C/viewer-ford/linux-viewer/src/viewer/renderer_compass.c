#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <common/small_linalg.h>
#include <common/rotations.h>
#include <dgc/globals.h>
#include <gtk_util/gtk_util.h>

#include <libviewer/viewer.h>

#include "dgcviewer.h"

typedef struct _RendererCompass RendererCompass;
struct _RendererCompass {
    Renderer renderer;

    CTrans * ctrans;
};

static void
compass_free (Renderer *renderer) 
{
  //    RendererCompass *self = (RendererCompass*) renderer;
    free (renderer);
}

/* The GLU that comes with Mesa seems fucked up.  It sometimes produces bogus
 * results.  They are probably using a bad matrix inverse routine.  This
 * version works fine for me. */
GLint
mygluUnProject (double winx, double winy, double winz,
        const double model[16], const double proj[16], const int viewport[4],
        double * objx, double * objy, double * objz)
{
    double p[16], m[16];
    matrix_transpose_4x4d (proj, p);
    matrix_transpose_4x4d (model, m);
    double t[16];
    matrix_multiply_4x4_4x4 (p, m, t);
    if (matrix_inverse_4x4d (t, m) < 0)
        return GL_FALSE;
    if (viewport[2] == 0 || viewport[3] == 0)
        return GL_FALSE;
    double v[4] = {
        2 * (winx - viewport[0]) / viewport[2] - 1,
        2 * (winy - viewport[1]) / viewport[3] - 1,
        2 * winz - 1,
        1,
    };
    double v2[4];
    matrix_vector_multiply_4x4_4d (m, v, v2);
    if (v2[3] == 0)
        return GL_FALSE;
    *objx = v2[0] / v2[3];
    *objy = v2[1] / v2[3];
    *objz = v2[2] / v2[3];
    return GL_TRUE;
}

/* Given a window position (in pixels) and a world z-coordinate,
 * compute the world x and y coordinates corresponding to that pixel. */
static int
window_pos_to_ground_pos (double * x, double * y,
        double z, double winx, double winy)
{
    GLdouble model_matrix[16];
    GLdouble proj_matrix[16];
    GLint viewport[4];
    double x1, y1, z1, x2, y2, z2;

    glGetDoublev (GL_MODELVIEW_MATRIX, model_matrix);
    glGetDoublev (GL_PROJECTION_MATRIX, proj_matrix);
    glGetIntegerv (GL_VIEWPORT, viewport);

    if (mygluUnProject (winx, viewport[3]-winy, 0.0, model_matrix, proj_matrix,
            viewport, &x1, &y1, &z1) == GL_FALSE)
        return -1;
    if (mygluUnProject (winx, viewport[3]-winy, 1.0, model_matrix, proj_matrix,
            viewport, &x2, &y2, &z2) == GL_FALSE)
        return -1;

    if ((z1 > 0 && z2 > 0) || (z1 < 0 && z2 < 0))
        return -1;

    *x = x1 + (z-z1)/(z2-z1) * (x2-x1);
    *y = y1 + (z-z1)/(z2-z1) * (y2-y1);
    return 0;
}

/* Draws the letter N */
static void
draw_letter_N ()
{
    glBegin (GL_QUADS);

    glVertex3f (-1.0, -1.0, 0);
    glVertex3f (-0.5, -1.0, 0);
    glVertex3f (-0.5, 1.0, 0);
    glVertex3f (-1.0, 1.0, 0);

    glVertex3f (1.0, -1.0, 0);
    glVertex3f (0.5, -1.0, 0);
    glVertex3f (0.5, 1.0, 0);
    glVertex3f (1.0, 1.0, 0);

    glVertex3f (-0.5, 1.0, 0);
    glVertex3f (-1.0, 1.0, 0);
    glVertex3f (0.5, -1.0, 0);
    glVertex3f (1.0, -1.0, 0);

    glEnd ();
}

static void
compass_draw (Viewer *viewer, Renderer *renderer)
{
    RendererCompass *self = (RendererCompass*) renderer;
    int viewport[4];
    int pos[2] = { -50, -50 };
    int radius = 35;

    double carpos[3] = { 0, 0, 0 };
    ctrans_local_pos (self->ctrans, carpos);

    double z = carpos[2];
    double x, y;
    double heading = 0, variance = 0;

    if (viewer->mode == RENDER_MODE_LOCAL) {
        lcmtypes_gps_to_local_t gps_to_local;
        if (ctrans_gps_to_local_raw (self->ctrans, &gps_to_local) < 0)
            return;
        heading = gps_to_local.lat_lon_el_theta[3];
        variance = gps_to_local.gps_cov[3][3];
    }

    /* Find the world x,y coordinates for the compass, given we want it
     * near the side of the viewport. */
    glGetIntegerv (GL_VIEWPORT, viewport);
    if (window_pos_to_ground_pos (&x, &y, z,
                viewport[2]+pos[0], viewport[3]+pos[1]) < 0)
        return;

    /* Find the world x,y coordinates for the edge of the compass, so
     * we can compute how large to make it in world coordinates. */
    double x2, y2;
    window_pos_to_ground_pos (&x2, &y2, z,
            viewport[2]+pos[0]+radius, viewport[3]+pos[1]);

    glDisable (GL_DEPTH_TEST);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Draw the compass at the right place, size, and orientation */
    glPushMatrix ();
    glTranslatef (x, y, z);
    double scale = sqrt ((x2-x)*(x2-x) + (y2-y)*(y2-y));
    glScalef (scale, scale, scale);
    glRotatef (heading * 180 / M_PI + 90, 0, 0, 1);

    GLUquadricObj * q = gluNewQuadric ();

    /* Draw an error bar indicating one standard deviation of the
     * heading error.  If it's above the threshold, make it red. */

    glColor4f (0.8, 0.2, 0.2, 0.7);
    double std_deg = sqrt (variance) * 180 / M_PI;
    gluPartialDisk (q, 1.0, 1.1, 50, 1,
            90 - std_deg, 2 * std_deg);

    /* Draw the circle for the compass */
    glColor4f (1.0, 1.0, 1.0, 0.2);
    gluDisk (q, 0, 1.0, 20, 1);
    gluDeleteQuadric (q);

    /* Draw the arrow on the compass */
    glLineWidth (2.0);
    glColor4f (1.0, 1.0, 1.0, 1.0);
    glBegin (GL_LINES);
    glVertex3f (-0.5, 0, 0);
    glVertex3f ( 0.5, 0, 0);

    glVertex3f (0, 0.5, 0);
    glVertex3f (0, -0.5, 0);

    glVertex3f (0.25, 0.25, 0);
    glVertex3f (0.5, 0, 0);

    glVertex3f (0.25, -0.25, 0);
    glVertex3f (0.5, 0, 0);
    glEnd ();

    /* Draw tick marks around the compass (15 degree increments) */
    glLineWidth (1.0);
    int i;
    for (i = 30; i <= 330; i += 15) {
        glPushMatrix ();
        glRotatef (i, 0, 0, 1);
        glBegin (GL_LINES);
        glVertex3f (0.95, 0, 0.0);
        glVertex3f (((i % 45) == 0) ? 0.75 : 0.85, 0, 0.0);
        glEnd ();
        glPopMatrix ();
    }

    /* Draw the letter N */
    glColor4f (0.2, 1.0, 0.6, 1.0);
    glTranslatef (0.75, 0, 0);
    glScalef (0.2, 0.2, 0.2);
    glRotatef (90, 0, 0, 1);
    draw_letter_N();

    glDisable (GL_BLEND);

    glPopMatrix ();
}


void setup_renderer_compass(Viewer *viewer, int priority)
{
    RendererCompass *self = (RendererCompass*) calloc (1, sizeof (RendererCompass));
    Renderer *renderer = &self->renderer;

    renderer->name = "Compass";
    renderer->enabled = 1;

    renderer->draw = compass_draw;
    renderer->destroy = compass_free;
    renderer->widget = NULL;

    self->ctrans = globals_get_ctrans ();

    viewer_add_renderer(viewer, renderer, priority);
}

