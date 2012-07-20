#ifndef __glutilglutil_h__
#define __glutilglutil_h__

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include "texture.h"
#include "splot2d.h"
#include "console.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * glutil_draw_cube:
 *
 * renders a unit cube centered on the origin
 */
void glutil_draw_cube ();

/**
 * glutil_draw_cube_frame:
 *
 * renders a unit cube wireframe centered on the origin
 */
void glutil_draw_cube_frame ();

/**
 * glutil_draw_circle:
 *
 * renders a circle of radius r on the plane Z = 0 centered on the origin
 */
void glutil_draw_circle (double r);

/**
 * glutil_build_circle:
 *
 * builds a circle as a display list
 */
void glutil_build_circle( GLuint id);

/**
 * glutil_draw_ellipse:
 *
 * renders an ellipse with semimajor axis of length a, semiminor axis of length
 * b, and an angle of theta between the semimajor axis and the X axis
 */
void glutil_draw_ellipse (double a, double b, double theta, int npoints);

//void glutil_draw_text (double x, double y, const char *string);
//
//void glutil_draw_text_window (double x, double y, const char *text, 
//        int w, int h);

/**
 * glutil_draw_arrow_2d:
 *
 * renders an arrow centered on the origin pointing along the X axis
 */
void glutil_draw_arrow_2d (double length, double head_width, double head_length,
        double body_width, int fill);

#define GLUTIL_DRAW_TEXT_DROP_SHADOW      1
#define GLUTIL_DRAW_TEXT_JUSTIFY_LEFT     2
#define GLUTIL_DRAW_TEXT_JUSTIFY_RIGHT    4
#define GLUTIL_DRAW_TEXT_JUSTIFY_CENTER   8
#define GLUTIL_DRAW_TEXT_ANCHOR_LEFT     16
#define GLUTIL_DRAW_TEXT_ANCHOR_RIGHT    32
#define GLUTIL_DRAW_TEXT_ANCHOR_TOP      64
#define GLUTIL_DRAW_TEXT_ANCHOR_BOTTOM  128
#define GLUTIL_DRAW_TEXT_ANCHOR_HCENTER 256
#define GLUTIL_DRAW_TEXT_ANCHOR_VCENTER 512
#define GLUTIL_DRAW_TEXT_NORMALIZED_SCREEN_COORDINATES 1024
#define GLUTIL_DRAW_TEXT_MONOSPACED 2048

/** Draw text centered at xyz in the current projection, but the text
 * will be drawn in pixel coordinates (so it will be "rectified"). A
 * font may be specified, as well as an optional drop shadow.
 *
 * We DO support multi-line text.
 *
 **/
void glutil_draw_text (const double xyz[3], void *font, const char *text, 
        int flags);

#endif
