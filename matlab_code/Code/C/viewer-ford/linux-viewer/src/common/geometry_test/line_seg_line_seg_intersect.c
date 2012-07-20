#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include "geometry_tester.h"

typedef struct _LineSegTester {
    GeometryTester parent;

    point2d_t points[4];

    GLUquadricObj *quadric;
} LineSegTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void mouse_motion (GeometryTester *self, const GdkEventMotion *motion);
static void destroy (GeometryTester *self);

GeometryTester * 
line_seg_line_seg_intersect_tester_new ()
{
    LineSegTester * self = (LineSegTester*) calloc (1, sizeof (LineSegTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.mouse_motion = mouse_motion;
    self->parent.destroy = destroy;
    self->parent.name = "line seg line seg intersect";

    self->points[0].x = 100;
    self->points[0].y = 100;
    self->points[1].x = 200;
    self->points[1].y = 100;
    self->points[2].x = 200;
    self->points[2].y = 100;
    self->points[3].x = 200;
    self->points[3].y = 200;

    self->quadric = gluNewQuadric ();

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    LineSegTester *self = (LineSegTester*) super;
    gluDeleteQuadric (self->quadric);
    free (self);
}

static void 
draw (GeometryTester *super)
{
    LineSegTester *self = (LineSegTester*) super;
    glColor3f (1, 1, 0);
    glBegin (GL_LINES);
    for (int i=0; i<4; i++) {
        glVertex2f (self->points[i].x, self->points[i].y);
    }
    glEnd ();

    point2d_t intersection;
    if (geom_line_seg_line_seg_intersect_2d (&self->points[0], 
                &self->points[1], &self->points[2], &self->points[3],
                &intersection)) {
        glColor3f (0, 1, 0);
        glPushMatrix ();
        glTranslatef (intersection.x, intersection.y, 0);
        gluDisk (self->quadric, 0, 5, 50, 1);
        glPopMatrix ();
    }
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    LineSegTester *self = (LineSegTester*) super;

    if (event->button == 1) {
        self->points[0].x = event->x;
        self->points[0].y = event->y;
        self->points[1].x = event->x;
        self->points[1].y = event->y;
    } else if (event->button == 3) {
        self->points[2].x = event->x;
        self->points[2].y = event->y;
        self->points[3].x = event->x;
        self->points[3].y = event->y;
    }
}

static void
mouse_motion (GeometryTester *super, const GdkEventMotion *motion)
{
    LineSegTester *self = (LineSegTester*) super;

    if (motion->state & GDK_BUTTON1_MASK) {
        self->points[1].x = motion->x;
        self->points[1].y = motion->y;
    } else if (motion->state & GDK_BUTTON3_MASK) {
        self->points[3].x = motion->x;
        self->points[3].y = motion->y;
    }
}
