#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include <common/convexhull.h>
#include "geometry_tester.h"

typedef struct _CircleCircleIsectTester {
    GeometryTester parent;

    point2d_t p0;
    double r0;

    point2d_t p1;
    double r1;

    GLUquadricObj *quadric;
} CircleCircleIsectTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void mouse_motion (GeometryTester *self, const GdkEventMotion *motion);
static void destroy (GeometryTester *self);

GeometryTester * 
circle_circle_intersect_tester_new ()
{
    CircleCircleIsectTester * self = 
        (CircleCircleIsectTester*) calloc (1, sizeof (CircleCircleIsectTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.mouse_motion = mouse_motion;
    self->parent.destroy = destroy;
    self->parent.name = "circle circle intersect";

    self->quadric = gluNewQuadric ();

    self->p0.x = 100;
    self->p0.y = 100;
    self->r0 = 30;

    self->p1.x = 180;
    self->p1.y = 100;
    self->r1 = 60;

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    CircleCircleIsectTester *self = (CircleCircleIsectTester*) super;
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
    CircleCircleIsectTester *self = (CircleCircleIsectTester*) super;

    glColor3f (1, 0, 0);
    glPushMatrix ();
    glTranslatef (self->p0.x, self->p0.y, 0);
    gluDisk (self->quadric, 0, 3, 20, 1);
    draw_circle (self->r0);
    glPopMatrix ();

    glColor3f (0, 1, 1);
    glPushMatrix ();
    glTranslatef (self->p1.x, self->p1.y, 0);
    gluDisk (self->quadric, 0, 3, 20, 1);
    draw_circle (self->r1);
    glPopMatrix ();

    point2d_t isect_left = { 0, 0 };
    point2d_t isect_right = { 0, 0 };

    int result = geom_circle_circle_intersect_2d (&self->p0, self->r0,
            &self->p1, self->r1, &isect_left, &isect_right);
    printf ("%d\n", result);
    if (result) {
        glColor3f (1, 1, 0);
        glPushMatrix ();
        glTranslatef (isect_left.x, isect_left.y, 0);
        gluDisk (self->quadric, 0, 3, 20, 1);
        glPopMatrix ();
        glColor3f (1, 0, 1);
        glPushMatrix ();
        glTranslatef (isect_right.x, isect_right.y, 0);
        gluDisk (self->quadric, 0, 3, 20, 1);
        glPopMatrix ();
    }
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    CircleCircleIsectTester *self = (CircleCircleIsectTester*) super;

    if (event->button == 1) {
        self->p0.x = event->x;
        self->p0.y = event->y;
        self->r0 = 1;
    } else if (event->button == 2) {
        self->p1.x = event->x;
        self->p1.y = event->y;
        self->r1 = 1;
    }
}

static void
mouse_motion (GeometryTester *super, const GdkEventMotion *motion)
{
    CircleCircleIsectTester *self = (CircleCircleIsectTester*) super;

    if (motion->state & GDK_BUTTON1_MASK) {
        double dx = motion->x - self->p0.x;
        double dy = motion->y - self->p0.y;
        self->r0 = sqrt (dx*dx + dy*dy);
    } else if (motion->state & GDK_BUTTON2_MASK) {
        double dx = motion->x - self->p1.x;
        double dy = motion->y - self->p1.y;
        self->r1 = sqrt (dx*dx + dy*dy);
    }
}
