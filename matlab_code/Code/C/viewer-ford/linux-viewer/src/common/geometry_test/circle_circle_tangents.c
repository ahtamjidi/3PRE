#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include <common/convexhull.h>
#include "geometry_tester.h"

typedef struct _CircleCircleTangentsTester {
    GeometryTester parent;

    point2d_t p0;
    double r0;

    point2d_t p1;
    double r1;

    GLUquadricObj *quadric;
} CircleCircleTangentsTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void mouse_motion (GeometryTester *self, const GdkEventMotion *motion);
static void destroy (GeometryTester *self);

GeometryTester * 
circle_circle_tangents_tester_new ()
{
    CircleCircleTangentsTester * self = 
        (CircleCircleTangentsTester*) calloc (1, 
                sizeof (CircleCircleTangentsTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.mouse_motion = mouse_motion;
    self->parent.destroy = destroy;
    self->parent.name = "circle circle tangents";

    self->quadric = gluNewQuadric ();

#if 0
    self->p1.x = 100;
    self->p1.y = 100;
    self->r1 = 80;

    self->p0.x = 220;
    self->p0.y = 100;
    self->r0 = 40;
#else
    self->p1.x = 100;
    self->p1.y = 100;
    self->r1 = 50;

    self->p0.x = 200;
    self->p0.y = 200;
    self->r0 = 50;
#endif

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    CircleCircleTangentsTester *self = (CircleCircleTangentsTester*) super;
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
    CircleCircleTangentsTester *self = (CircleCircleTangentsTester*) super;

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

    pointlist2d_t *tangents = 
        geom_circle_circle_tangents_2d (&self->p0, self->r0,
            &self->p1, self->r1);

    if (tangents) {
        point3d_t colors[4];
        memset (colors, 0, sizeof (colors));

        switch (tangents->npoints) {
            case 2:
                colors[0].x = colors[0].y = colors[0].z = 1;
                break;
            case 4:
                colors[0].x = 1; colors[0].y = 1; colors[0].z = 0;
                colors[1].x = 1; colors[1].y = 0; colors[1].z = 1;
                break;
            case 6:
                colors[0].x = 1; colors[0].y = 1; colors[0].z = 1;
                colors[1].x = 1; colors[1].y = 1; colors[1].z = 0;
                colors[2].x = 1; colors[2].y = 0; colors[2].z = 1;
                break;
            case 8:
                colors[0].x = 1; colors[0].y = 1; colors[0].z = 0;
                colors[1].x = 1; colors[1].y = 0; colors[1].z = 1;
                colors[2].x = 1; colors[2].y = 1; colors[2].z = 0;
                colors[3].x = 1; colors[3].y = 0; colors[3].z = 1;
                break;
        };

        glBegin (GL_LINES);
        for (int i=0; i<tangents->npoints/2; i++) {
            point3d_t c = colors[i];
            glColor3f (c.x, c.y, c.z);
            glVertex2d (tangents->points[i*2+0].x, tangents->points[i*2+0].y);
            glVertex2d (tangents->points[i*2+1].x, tangents->points[i*2+1].y);
        }
        glEnd ();

        pointlist2d_free (tangents);
    }
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    CircleCircleTangentsTester *self = (CircleCircleTangentsTester*) super;

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
    CircleCircleTangentsTester *self = (CircleCircleTangentsTester*) super;

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
