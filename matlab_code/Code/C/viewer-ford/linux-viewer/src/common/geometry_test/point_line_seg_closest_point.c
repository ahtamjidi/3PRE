#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include "geometry_tester.h"

typedef struct _PointLineClosestPointTester {
    GeometryTester parent;

    point2d_t seg1;
    point2d_t seg2;
    point2d_t pt;

    GLUquadricObj *quadric;
} PointLineClosestPointTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void mouse_motion (GeometryTester *self, const GdkEventMotion *motion);
static void destroy (GeometryTester *self);

GeometryTester * 
point_line_closest_point_tester_new ()
{
    PointLineClosestPointTester * self = (PointLineClosestPointTester*) 
        calloc (1, sizeof (PointLineClosestPointTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.mouse_motion = mouse_motion;
    self->parent.destroy = destroy;
    self->parent.name = "point/line closest point";

    self->seg1.x = 100;
    self->seg1.y = 100;
    self->seg2.x = 300;
    self->seg2.y = 100;

    self->pt.x = 200;
    self->pt.y = 200;

    self->quadric = gluNewQuadric ();

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    PointLineClosestPointTester *self = (PointLineClosestPointTester*) super;
    gluDeleteQuadric (self->quadric);
    free (self);
}

static void
draw_disk (PointLineClosestPointTester *self, double x, double y, double r)
{
    glPushMatrix ();
    glTranslatef (x, y, 0);
    gluDisk (self->quadric, 0, r, 50, 1);
    glPopMatrix ();
}

static void 
draw (GeometryTester *super)
{
    PointLineClosestPointTester *self = (PointLineClosestPointTester*) super;
    glColor3f (1, 1, 0);
    glBegin (GL_LINES);
    glVertex2f (self->seg1.x, self->seg1.y);
    glVertex2f (self->seg2.x, self->seg2.y);
    glEnd ();
//    glPushMatrix ();
//    glTranslatef (self->seg1.x, self->seg1.y, 0);
//    gluDisk (self->quadric, 0, 5, 50, 1);
//    glPopMatrix ();
//    glPushMatrix ();
//    glTranslatef (self->seg2.x, self->seg2.y, 0);
//    gluDisk (self->quadric, 0, 5, 50, 1);
//    glPopMatrix ();

    glColor3f (0, 0.5, 1);
    draw_disk (self, self->pt.x, self->pt.y, 5);

    glColor3f (1, 0, 1);
    point2d_t seg_closest_point;
    double u = 0;
    geom_point_line_seg_closest_point_2d (&self->pt, 
            &self->seg1, &self->seg2, &seg_closest_point, &u);
    draw_disk (self, seg_closest_point.x, seg_closest_point.y, 5);
    
    vec3d_t a = { self->seg1.x, self->seg1.y, 1 };
    vec3d_t b = { self->seg2.x, self->seg2.y, 1 };
    vec3d_t c;
    geom_vec_cross_3d (&a, &b, &c);
    geom_vec_normalize_3d (&c);
    vec3d_t p = { self->pt.x, self->pt.y, 1 };
    double d = geom_vec_vec_dot_3d (&c, &p);
    printf ("%f %f %f\n", d, d*d, 
            geom_point_point_distance_2d (&self->pt, 
                &seg_closest_point));

    glColor3f (0, 1, 0);
    point2d_t line_closest_point;
    if (0 != geom_point_line_closest_point_2d (&self->pt, 
            &self->seg1, &self->seg2, &line_closest_point, NULL)) return;
    draw_disk (self, line_closest_point.x, line_closest_point.y, 3);
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    PointLineClosestPointTester *self = (PointLineClosestPointTester*) super;

    if (event->button == 1) {
        self->seg1.x = event->x;
        self->seg1.y = event->y;
        self->seg2.x = event->x;
        self->seg2.y = event->y;
    } else if (event->button == 3) {
        self->pt.x = event->x;
        self->pt.y = event->y;
    }
}

static void
mouse_motion (GeometryTester *super, const GdkEventMotion *motion)
{
    PointLineClosestPointTester *self = (PointLineClosestPointTester*) super;

    if (motion->state & GDK_BUTTON1_MASK) {
        self->seg2.x = motion->x;
        self->seg2.y = motion->y;
    } else if (motion->state & GDK_BUTTON3_MASK) {
        self->pt.x = motion->x;
        self->pt.y = motion->y;
    }
}
