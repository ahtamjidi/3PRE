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
#include <lcmtypes/lcmtypes_adu_status_t.h>
#include <lcmtypes/lcmtypes_adu_secondary_t.h>
#include <gtk_util/gtk_util.h>
#include <glutil/glutil.h>
#include <libviewer/viewer.h>

#include "dgcviewer.h"

typedef struct _RendererADU RendererADU;
struct _RendererADU {
    Renderer renderer;

    double render_offset[2];

    lcmtypes_adu_status_t * adu_status;
    lcmtypes_adu_secondary_t * adu_secondary;
    lcmtypes_adu_status_t_subscription_t * status_handler;
    lcmtypes_adu_secondary_t_subscription_t * secondary_handler;
    lcm_t *lc;
};

static void
adu_free (Renderer *renderer) 
{
    RendererADU *self = (RendererADU*) renderer;
    if (self->adu_status)
        lcmtypes_adu_status_t_destroy (self->adu_status);
    if (self->adu_secondary)
        lcmtypes_adu_secondary_t_destroy (self->adu_secondary);
    lcmtypes_adu_status_t_unsubscribe (self->lc, self->status_handler);
    lcmtypes_adu_secondary_t_unsubscribe (self->lc, self->secondary_handler);
    globals_release_lcm (self->lc);
    free (renderer);
}

void draw_arrow_ll(double xmult)
{
	glBegin(GL_LINE_LOOP);
	glVertex2d(xmult*-0.5, 0.25);
	glVertex2d(0, 0.25);
	glVertex2d(0, 0.5);
	glVertex2d(xmult*0.5, 0);
	glVertex2d(0, -0.5);
	glVertex2d(0, -0.25);
	glVertex2d(xmult*-0.5, -0.25);
	glEnd();
}

void draw_arrow_filled(double xmult)
{
	glBegin(GL_QUADS);
	glVertex2d(xmult*-0.5, 0.25);
	glVertex2d(0, 0.25);
	glVertex2d(0, -0.25);
	glVertex2d(xmult*-0.5, -0.25);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex2d(0, 0.5);
	glVertex2d(xmult*0.5, 0);
	glVertex2d(0, -0.5);
	glEnd();
}

void draw_light_semic()
{
	glVertex2d(0, 0.5);
	glVertex2d(0.5*-0.5, 0.5*0.87);
	glVertex2d(0.5*-0.87, 0.5*0.5);
	glVertex2d(-0.5, 0);
	glVertex2d(0.5*-0.87, 0.5*-0.5);
	glVertex2d(0.5*-0.5, 0.5*-0.87);
	glVertex2d(0, -0.5);
}

void draw_light_rays()
{
	glVertex2d(0.1, 0);
	glVertex2d(0.5, 0);
	glVertex2d(0.1, 0.1);
	glVertex2d(0.5, 0.2);
	glVertex2d(0.1, -0.1);
	glVertex2d(0.5, -0.2);
	glVertex2d(0.1, 0.2);
	glVertex2d(0.5, 0.4);
	glVertex2d(0.1, -0.2);
	glVertex2d(0.5, -0.4);
}

#define COLOR_LIGHT_OFF 0.2, 0.2, 0.5
#define COLOR_LIGHT_ON 0.3, 0.3, 0.9

void draw_headlight(float w, double r, double g, double b)
{
	glColor3d(0, 0, 0);
	glLineWidth(2*w);
	glBegin(GL_LINE_LOOP);
	draw_light_semic();
	glEnd();
	glBegin(GL_LINES);
	draw_light_rays();
	glEnd();

	glColor3d(r, g, b);
	glBegin(GL_POLYGON);
	draw_light_semic();
	glEnd();
	glLineWidth(0.75*w);
	glBegin(GL_LINES);
	draw_light_rays();
	glEnd();
}

#define COLOR_TURN_ON 1.0, 0.77, 0.0
#define COLOR_TURN_OFF 0.31, 0.24, 0.0

static void
draw_P ()
{
    glBegin (GL_LINE_LOOP);
    glVertex2f (0.2, 0);
    glVertex2f (0.6, 0);
    glVertex2f (0.8, 0.2);
    glVertex2f (0.8, 0.4);
    glVertex2f (0.6, 0.6);
    glVertex2f (0.4, 0.6);
    glVertex2f (0.4, 1.0);
    glVertex2f (0.2, 1.0);
    glEnd ();

    glBegin (GL_LINE_LOOP);
    glVertex2f (0.4, 0.2);
    glVertex2f (0.6, 0.2);
    glVertex2f (0.6, 0.4);
    glVertex2f (0.4, 0.4);
    glEnd ();
}

static void
draw_R ()
{
    glBegin (GL_LINE_LOOP);
    glVertex2f (0.2, 0);
    glVertex2f (0.6, 0);
    glVertex2f (0.8, 0.2);
    glVertex2f (0.8, 0.4);
    glVertex2f (0.6, 0.6);
    glVertex2f (0.8, 1.0);
    glVertex2f (0.6, 1.0);
    glVertex2f (0.4, 0.6);
    glVertex2f (0.4, 1.0);
    glVertex2f (0.2, 1.0);
    glEnd ();

    glBegin (GL_LINE_LOOP);
    glVertex2f (0.4, 0.2);
    glVertex2f (0.6, 0.2);
    glVertex2f (0.6, 0.4);
    glVertex2f (0.4, 0.4);
    glEnd ();
}

static void
draw_N ()
{
    glBegin (GL_LINE_LOOP);
    glVertex2f (0.2, 0.0);
    glVertex2f (0.4, 0.0);
    glVertex2f (0.6, 0.5);
    glVertex2f (0.6, 0.0);
    glVertex2f (0.8, 0.0);

    glVertex2f (0.8, 1.0);
    glVertex2f (0.6, 1.0);
    glVertex2f (0.4, 0.5);
    glVertex2f (0.4, 1.0);
    glVertex2f (0.2, 1.0);
    glEnd ();
}

static void
draw_D ()
{
    glBegin (GL_LINE_LOOP);
    glVertex2f (0.2, 0.0);
    glVertex2f (0.6, 0.0);
    glVertex2f (0.8, 0.2);
    glVertex2f (0.8, 0.8);
    glVertex2f (0.6, 1.0);
    glVertex2f (0.2, 1.0);
    glEnd ();

    glBegin (GL_LINE_LOOP);
    glVertex2f (0.4, 0.2);
    glVertex2f (0.5, 0.2);
    glVertex2f (0.6, 0.3);
    glVertex2f (0.6, 0.7);
    glVertex2f (0.5, 0.8);
    glVertex2f (0.4, 0.8);
    glEnd ();
}

static void
draw_2 ()
{
    glBegin (GL_LINE_LOOP);
    glVertex2f (0.2, 0.2);
    glVertex2f (0.4, 0.0);
    glVertex2f (0.6, 0.0);
    glVertex2f (0.8, 0.2);
    glVertex2f (0.8, 0.5);
    glVertex2f (0.5, 0.8);

    glVertex2f (0.8, 0.8);
    glVertex2f (0.8, 1.0);
    glVertex2f (0.2, 1.0);
    glVertex2f (0.2, 0.8);
    glVertex2f (0.6, 0.4);
    glVertex2f (0.6, 0.3);
    glVertex2f (0.55, 0.20);
    glVertex2f (0.45, 0.20);
    glVertex2f (0.3, 0.35);

    glEnd ();
}

static void
_setcolor (RendererADU *self, lcmtypes_shift_enum_t gear)
{
    if (! self->adu_status) {
        glColor3f (0.3, 0.3, 0.3); return;
    }

    if (self->adu_status->shift_current == gear) {
        glColor3f (0, 1, 0);
        return;
    }

    if (self->adu_status->shift_target == gear &&
        self->adu_status->shift_in_progress) {
        glColor3f (0.5, 0.5, 0);
        return;
    }

    glColor3f (0.3, 0.3, 0.3);
}

static void
do_draw (RendererADU *self)
{
    double letter_width = 25;
    double letter_height = 25;

    glColor3f (0.3, 0.3, 0.3);
    glBegin (GL_LINE_LOOP);
    glVertex2d (0, 0);
    glVertex2d (letter_width, 0);
    glVertex2d (letter_width, letter_height * 6);
    glVertex2d (0, letter_height * 6);
    glEnd ();

    glScalef (letter_width, letter_height, 1);
    _setcolor (self, LCMTYPES_SHIFT_ENUM_T_PARK);
    draw_P ();
    glTranslatef (0, 1.2, 0);
    _setcolor (self, LCMTYPES_SHIFT_ENUM_T_REVERSE);
    draw_R ();
    glTranslatef (0, 1.2, 0);
    _setcolor (self, LCMTYPES_SHIFT_ENUM_T_NEUTRAL);
    draw_N ();
    glTranslatef (0, 1.2, 0);
    _setcolor (self, LCMTYPES_SHIFT_ENUM_T_DRIVE);
    draw_D ();
    glTranslatef (0, 1.2, 0);
    _setcolor (self, LCMTYPES_SHIFT_ENUM_T_DRIVE2);
    draw_2 ();
}

static void
do_draw_secondary (RendererADU *self)
{
    if (!self->adu_secondary)
        return;

    glScalef (30, 30, 30);
	// left
	glTranslated(0.7, 0.7, 0);
	if (self->adu_secondary->signals & (1<<0))
        glColor3d(COLOR_TURN_ON);
	else
        glColor3d(COLOR_TURN_OFF);
	draw_arrow_filled(-1);
	glColor3d(0, 0, 0);
	draw_arrow_ll(-1);

	// right
	glTranslated(1.2, 0, 0);
	if (self->adu_secondary->signals & (1<<1))
        glColor3d(COLOR_TURN_ON);
	else
        glColor3d(COLOR_TURN_OFF);
	draw_arrow_filled(1);
	glColor3d(0, 0, 0);
	draw_arrow_ll(1);

    int linewidth = 1;
	glTranslated(1.2, 0, 0);
	if (self->adu_secondary->headlights)
        draw_headlight(linewidth, COLOR_LIGHT_ON);
	else
        draw_headlight(linewidth, COLOR_LIGHT_OFF);
}

static void
adu_draw (Viewer *viewer, Renderer *renderer)
{
    RendererADU *self = (RendererADU*) renderer;
    // figure out how big the drawing window (viewport) is
    GLint viewport[4];
    glGetIntegerv (GL_VIEWPORT, viewport);

    // transform into window coordinates, where <0, 0> is the top left corner
    // of the window and <viewport[2], viewport[3]> is the bottom right corner
    // of the window
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, viewport[2], 0, viewport[3]);

    glColor3f(1,1,1);
    int state = self->adu_status ? self->adu_status->state : LCMTYPES_ADU_STATE_ENUM_T_UNDEFINED;
    char *adu_string;
    switch (state) 
    {
    case LCMTYPES_ADU_STATE_ENUM_T_RUN:
        adu_string = "RUN"; break;
    case LCMTYPES_ADU_STATE_ENUM_T_STOP:
        adu_string = "PAUSE"; break;
    case LCMTYPES_ADU_STATE_ENUM_T_MANUAL_OVERRIDE:
        adu_string = "MANUAL"; break;
    case LCMTYPES_ADU_STATE_ENUM_T_STANDBY:
        adu_string = "STANDBY"; break;
    case LCMTYPES_ADU_STATE_ENUM_T_ERROR:
        adu_string = "ERROR"; break;
    case LCMTYPES_ADU_STATE_ENUM_T_SHIFT:
        adu_string = "SHIFT"; break;
    default:
    case LCMTYPES_ADU_STATE_ENUM_T_UNDEFINED:
        adu_string = "UNDEFINED"; break;
    }
        
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    double state_xyz[] = {50, 55, 100};
    glutil_draw_text(state_xyz, NULL, adu_string,
//                     GLUTIL_DRAW_TEXT_NORMALIZED_SCREEN_COORDINATES |
                     GLUTIL_DRAW_TEXT_JUSTIFY_CENTER |
                     GLUTIL_DRAW_TEXT_ANCHOR_VCENTER |
                     GLUTIL_DRAW_TEXT_ANCHOR_HCENTER |
                     GLUTIL_DRAW_TEXT_DROP_SHADOW);



    do_draw_secondary (self);

    glLoadIdentity();
    glTranslatef(0, viewport[3], 0);
    glScalef(1, -1, 1);

    do_draw (self);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static void
on_adu_status (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_adu_status_t *msg, void *user_data)
{
    RendererADU *self = (RendererADU*) user_data;
    if (self->adu_status)
        lcmtypes_adu_status_t_destroy (self->adu_status);
    self->adu_status = lcmtypes_adu_status_t_copy (msg);
}

static void
on_adu_secondary (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_adu_secondary_t *msg,
        void *user_data)
{
    RendererADU *self = (RendererADU*) user_data;
    if (self->adu_secondary)
        lcmtypes_adu_secondary_t_destroy (self->adu_secondary);
    self->adu_secondary = lcmtypes_adu_secondary_t_copy (msg);
}

void setup_renderer_adu(Viewer *viewer, int priority)
{
    RendererADU *self = (RendererADU*) calloc (1, sizeof (RendererADU));
    Renderer *renderer = &self->renderer;

    renderer->name = "ADU";
    renderer->enabled = 1;

    renderer->draw = adu_draw;
    renderer->destroy = adu_free;
    renderer->widget = NULL;

    self->render_offset[0] = 0;
    self->render_offset[1] = 0;

    self->lc = globals_get_lcm ();
    self->status_handler = lcmtypes_adu_status_t_subscribe (self->lc, "ADU_STATUS",
            on_adu_status, self);
    self->adu_status = NULL;
    self->secondary_handler = lcmtypes_adu_secondary_t_subscribe (self->lc,
            "ADU_SECONDARY", on_adu_secondary, self);
    self->adu_secondary = NULL;

    viewer_add_renderer(viewer, renderer, priority);
}

