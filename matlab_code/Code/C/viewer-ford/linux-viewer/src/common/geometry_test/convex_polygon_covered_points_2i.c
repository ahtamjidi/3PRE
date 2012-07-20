#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include <common/convexhull.h>
#include "geometry_tester.h"

typedef struct _ConvexPolyCoveredPoints2iTester {
    GeometryTester parent;

    GQueue * poly1;
} ConvexPolyCoveredPoints2iTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void destroy (GeometryTester *self);

GeometryTester * 
convex_polygon_covered_points_2i_tester_new ()
{
    ConvexPolyCoveredPoints2iTester * self = 
        (ConvexPolyCoveredPoints2iTester*) calloc (1, 
                sizeof (ConvexPolyCoveredPoints2iTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.destroy = destroy;
    self->parent.name = "convex polygon covered points";

    self->poly1 = g_queue_new ();

    g_queue_push_tail (self->poly1, point2i_new (200, 100));
    g_queue_push_tail (self->poly1, point2i_new (300, 150));
    g_queue_push_tail (self->poly1, point2i_new (100, 200));
    g_queue_push_tail (self->poly1, point2i_new (300, 200));

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    ConvexPolyCoveredPoints2iTester *self = 
        (ConvexPolyCoveredPoints2iTester*) super;
    while (! g_queue_is_empty (self->poly1)) {
        point2i_free ( (point2i_t*) g_queue_pop_head (self->poly1));
    }
    g_queue_free (self->poly1);
    free (self);
}

//static void
//draw_polygon (pointlist2i_t *poly)
//{
//    printf ("drawing pointlist with %d points\n", poly->npoints);
//    glBegin (GL_LINE_LOOP);
//    for (int i=0; i<poly->npoints; i++) {
//        glVertex2i (poly->points[i].x, poly->points[i].y);
//    }
//    glEnd ();
//
//    GLUquadricObj *q = gluNewQuadric();
//    for (int i=0; i<poly->npoints; i++) {
//        glPushMatrix ();
//        glTranslatef (poly->points[i].x, poly->points[i].y, 0);
//        gluDisk (q, 0, 5, 50, 1);
//        glPopMatrix ();
//    }
//    gluDeleteQuadric (q);
//}

static void
draw_pointlist (pointlist2i_t *poly)
{
    printf ("drawing polygon with %d points\n", poly->npoints);
    glBegin (GL_POINTS);
    for (int i=0; i<poly->npoints; i++) {
        glVertex2i (poly->points[i].x, poly->points[i].y);
    }
    glEnd ();
}

static void 
draw (GeometryTester *super)
{
    ConvexPolyCoveredPoints2iTester *self = 
        (ConvexPolyCoveredPoints2iTester*) super;
    glColor3f (1, 1, 0);

    pointlist2i_t *poly1 = pointlist2i_new_from_gqueue (self->poly1);
    pointlist2i_t *c1 = convexhull_graham_scan_2i (poly1);
    pointlist2i_free (poly1);

    if (c1) {
        pointlist2i_t *covered = 
            geom_compute_convex_polygon_covered_points_2i (c1);

        if (covered) {
            glColor3f (1, 0, 0);
            draw_pointlist (covered);
            pointlist2i_free (covered);
        }

        glColor3f (0, 1, 0);
//        draw_polygon (c1);
        pointlist2i_free (c1);
    }
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    ConvexPolyCoveredPoints2iTester *self = 
        (ConvexPolyCoveredPoints2iTester*) super;

    if (event->button == 1) {
        point2i_t * newpoint = point2i_new ((int)event->x, (int)event->y);
        g_queue_push_tail (self->poly1, newpoint);
    } else if (event->button == 2) {
        while (! g_queue_is_empty (self->poly1)) {
            point2i_free ((point2i_t*) g_queue_pop_head (self->poly1));
        }
    }
}
