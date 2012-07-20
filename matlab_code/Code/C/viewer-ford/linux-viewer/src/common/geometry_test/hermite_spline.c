#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include <common/hspline.h>
#include "geometry_tester.h"

typedef struct _HermiteSplineTester {
    GeometryTester parent;

    point2d_t seg1;
    point2d_t seg2;
    point2d_t seg3;
    point2d_t seg4;

    GLUquadricObj *quadric;
} HermiteSplineTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void mouse_motion (GeometryTester *self, const GdkEventMotion *motion);
static void destroy (GeometryTester *self);

GeometryTester * 
hermite_spline_tester_new ()
{
    HermiteSplineTester * self = (HermiteSplineTester*) 
        calloc (1, sizeof (HermiteSplineTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.mouse_motion = mouse_motion;
    self->parent.destroy = destroy;
    self->parent.name = "hermite spline";

    self->seg1.x = 100;
    self->seg1.y = 100;
    self->seg2.x = 300;
    self->seg2.y = 100;

    self->seg3.x = 200;
    self->seg3.y = 200;
    self->seg4.x = 200;
    self->seg4.y = 300;

    self->quadric = gluNewQuadric ();

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    HermiteSplineTester *self = (HermiteSplineTester*) super;
    gluDeleteQuadric (self->quadric);
    free (self);
}

static void 
draw (GeometryTester *super)
{
    HermiteSplineTester *self = (HermiteSplineTester*) super;
    glColor3f (1, 1, 0);
    glBegin (GL_LINES);
    glVertex2f (self->seg1.x, self->seg1.y);
    glVertex2f (self->seg2.x, self->seg2.y);
    glEnd ();
    glPushMatrix ();
    glTranslatef (self->seg1.x, self->seg1.y, 0);
    gluDisk (self->quadric, 0, 5, 50, 1);
    glPopMatrix ();
    glPushMatrix ();
    glTranslatef (self->seg2.x, self->seg2.y, 0);
    gluDisk (self->quadric, 0, 5, 50, 1);
    glPopMatrix ();

    glColor3f (0, 0.5, 1);
    glBegin (GL_LINES);
    glVertex2f (self->seg3.x, self->seg3.y);
    glVertex2f (self->seg4.x, self->seg4.y);
    glEnd ();
    glPushMatrix ();
    glTranslatef (self->seg3.x, self->seg3.y, 0);
    gluDisk (self->quadric, 0, 5, 50, 1);
    glPopMatrix ();
    glPushMatrix ();
    glTranslatef (self->seg4.x, self->seg4.y, 0);
    gluDisk (self->quadric, 0, 5, 50, 1);
    glPopMatrix ();

    vec2d_t tan1 = { self->seg2.x - self->seg1.x, self->seg2.y - self->seg1.y };
    vec2d_t tan2 = { self->seg4.x - self->seg3.x, self->seg4.y - self->seg3.y };

#if 0
    double mag1 = geom_vec_magnitude_2d (&tan1);
    double mag2 = geom_vec_magnitude_2d (&tan2);
    if (mag1) geom_vec_normalize_2d (&tan1);
    if (mag2) geom_vec_normalize_2d (&tan2);
    double dist = geom_point_point_distance_2d (&self->seg1, &self->seg2);
    geom_vec_scale_2d (&tan1, dist, &tan1);
    geom_vec_scale_2d (&tan2, dist, &tan2);
#endif

    hspline_point_t p1 = {
        { self->seg1.x, self->seg1.y },
        { tan1.x, tan1.y },
    };
    hspline_point_t p2 = {
        { self->seg3.x, self->seg3.y },
        { tan2.x, tan2.y },
    };

    int npoints = 50;
    double spline_x[npoints];
    double spline_y[npoints];
    hspline_sample_segment_points (&p1, &p2, spline_x, spline_y, npoints, 1);

    glColor3f (1, 0, 1);
    glBegin (GL_LINE_STRIP);
    for (int i=0; i<npoints; i++) {
        glVertex2d (spline_x[i], spline_y[i]);
    }
    glVertex2d (self->seg3.x, self->seg3.y);
    glEnd ();
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    HermiteSplineTester *self = (HermiteSplineTester*) super;

    if (event->button == 1) {
        self->seg1.x = event->x;
        self->seg1.y = event->y;
        self->seg2.x = event->x;
        self->seg2.y = event->y;
    } else if (event->button == 3) {
        self->seg3.x = event->x;
        self->seg3.y = event->y;
        self->seg4.x = event->x;
        self->seg4.y = event->y;
    }
}

static void
mouse_motion (GeometryTester *super, const GdkEventMotion *motion)
{
    HermiteSplineTester *self = (HermiteSplineTester*) super;

    if (motion->state & GDK_BUTTON1_MASK) {
        self->seg2.x = motion->x;
        self->seg2.y = motion->y;
    } else if (motion->state & GDK_BUTTON3_MASK) {
        self->seg4.x = motion->x;
        self->seg4.y = motion->y;
    }
}
