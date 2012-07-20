#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include <common/convexhull.h>
#include "geometry_tester.h"

typedef struct _PointCircleTangentTester {
    GeometryTester parent;

    point2d_t circle_center;
    double r;

    point2d_t point;

    GLUquadricObj *quadric;
} PointCircleTangentTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void mouse_motion (GeometryTester *self, const GdkEventMotion *motion);
static void destroy (GeometryTester *self);

GeometryTester * 
point_circle_tangent_tester_new ()
{
    PointCircleTangentTester * self = 
        (PointCircleTangentTester*) calloc (1, 
                sizeof (PointCircleTangentTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.mouse_motion = mouse_motion;
    self->parent.destroy = destroy;
    self->parent.name = "point-circle tangent";

    self->quadric = gluNewQuadric ();

    self->circle_center.x = 100;
    self->circle_center.y = 100;
    self->r = 50;

    self->point.x = 180;
    self->point.y = 100;

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    PointCircleTangentTester *self = (PointCircleTangentTester*) super;
    gluDeleteQuadric (self->quadric);

    free (self);
}

static void
draw_circle (double r)
{
    glBegin (GL_LINE_LOOP);
    for (int i=0; i<360; i++) {
        double theta = i * M_PI / 180;
        glVertex2f (r * cos (theta), r * sin (theta));
    }
    glEnd ();
}

static void 
draw (GeometryTester *super)
{
    PointCircleTangentTester *self = (PointCircleTangentTester*) super;

    glColor3f (1, 0, 0);
    glPushMatrix ();
    glTranslatef (self->circle_center.x, self->circle_center.y, 0);
    gluDisk (self->quadric, 0, 3, 20, 1);
    draw_circle (self->r);
    glPopMatrix ();

    glColor3f (0, 1, 1);
    glPushMatrix ();
    glTranslatef (self->point.x, self->point.y, 0);
    gluDisk (self->quadric, 0, 3, 20, 1);
    glPopMatrix ();

    point2d_t tangent_left = { 0, 0 };
    point2d_t tangent_right = { 0, 0 };

    int result = geom_point_circle_tangent_2d (&self->point,
            &self->circle_center, self->r,
            &tangent_left, &tangent_right);
    if (result) {
        glColor3f (1, 1, 0);
        glBegin (GL_LINES);
        glVertex2d (self->point.x, self->point.y);
        glVertex2d (tangent_left.x, tangent_left.y);
        glEnd ();
        glPushMatrix ();
        glTranslatef (tangent_left.x, tangent_left.y, 0);
        gluDisk (self->quadric, 0, 3, 20, 1);
        glPopMatrix ();

        glColor3f (1, 0, 1);
        glBegin (GL_LINES);
        glVertex2d (self->point.x, self->point.y);
        glVertex2d (tangent_right.x, tangent_right.y);
        glEnd ();
        glPushMatrix ();
        glTranslatef (tangent_right.x, tangent_right.y, 0);
        gluDisk (self->quadric, 0, 3, 20, 1);
        glPopMatrix ();
    }
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    PointCircleTangentTester *self = (PointCircleTangentTester*) super;

    if (event->button == 1) {
        self->circle_center.x = event->x;
        self->circle_center.y = event->y;
        self->r = 1;
    } else if (event->button == 2) {
        self->point.x = event->x;
        self->point.y = event->y;
    }
}

static void
mouse_motion (GeometryTester *super, const GdkEventMotion *motion)
{
    PointCircleTangentTester *self = (PointCircleTangentTester*) super;

    if (motion->state & GDK_BUTTON1_MASK) {
        double dx = motion->x - self->circle_center.x;
        double dy = motion->y - self->circle_center.y;
        self->r = sqrt (dx*dx + dy*dy);
    } else if (motion->state & GDK_BUTTON2_MASK) {
        self->point.x = motion->x;
        self->point.y = motion->y;
    }
}
