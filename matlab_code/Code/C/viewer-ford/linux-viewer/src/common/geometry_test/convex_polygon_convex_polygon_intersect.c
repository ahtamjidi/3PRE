#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include <common/convexhull.h>
#include "geometry_tester.h"

typedef struct _ConvexPolyTester {
    GeometryTester parent;

    GQueue * poly1;
    GQueue * poly2;

    GLUquadricObj *quadric;
} ConvexPolyTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void destroy (GeometryTester *self);

GeometryTester * 
convex_polygon_convex_polygon_intersect_tester_new ()
{
    ConvexPolyTester * self = 
        (ConvexPolyTester*) calloc (1, sizeof (ConvexPolyTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.destroy = destroy;
    self->parent.name = "convex polygon convex polygon intersect";

    self->quadric = gluNewQuadric ();

#if 0
    self->poly1 = g_queue_new ();
    g_queue_push_tail (self->poly1, point2d_new (0, 100));
    g_queue_push_tail (self->poly1, point2d_new (0, 0));
    g_queue_push_tail (self->poly1, point2d_new (100, 0));
    g_queue_push_tail (self->poly1, point2d_new (100, 100));

    self->poly2 = g_queue_new ();
    g_queue_push_tail (self->poly2, point2d_new (50, 150));
    g_queue_push_tail (self->poly2, point2d_new (50, 50));
    g_queue_push_tail (self->poly2, point2d_new (150, 50));
    g_queue_push_tail (self->poly2, point2d_new (150, 150));

#else
    self->poly1 = g_queue_new ();
    self->poly2 = g_queue_new ();

    double a_pts[] = {
93.34669723347082, 49.451299010402728,
40, 65,
40, 55
    };
    int n_apts = sizeof (a_pts) / sizeof (double) / 2;

    for (int i=0; i<n_apts; i++) {
        g_queue_push_tail (self->poly1, point2d_new (a_pts[2*i], a_pts[2*i+1]));
    }

    double b_pts[] = {
15, 15,
93.34669723347082, 49.451299010402728,
15, 100,
    };
    int n_bpts = sizeof (b_pts) / sizeof (double) / 2;
    for (int i=0; i<n_bpts; i++) {
        g_queue_push_tail (self->poly2, point2d_new (b_pts[2*i], b_pts[2*i+1]));
    }

#endif
    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    ConvexPolyTester *self = (ConvexPolyTester*) super;
    gluDeleteQuadric (self->quadric);
    while (! g_queue_is_empty (self->poly1)) {
        point2d_free ((point2d_t*) g_queue_pop_head (self->poly1));
    }
    g_queue_free (self->poly1);
    while (! g_queue_is_empty (self->poly2)) {
        point2d_free ((point2d_t*) g_queue_pop_head (self->poly2));
    }
    g_queue_free (self->poly2);

    free (self);
}

static void
draw_polygon (pointlist2d_t *poly)
{
    printf ("drawing polygon with %d points\n", poly->npoints);
    glBegin (GL_LINE_LOOP);
    for (int i=0; i<poly->npoints; i++) {
        glVertex2f (poly->points[i].x, poly->points[i].y);
    }
    glEnd ();

    GLUquadricObj *q = gluNewQuadric();
    for (int i=0; i<poly->npoints; i++) {
        glPushMatrix ();
        glTranslatef (poly->points[i].x, poly->points[i].y, 0);
        gluDisk (q, 0, 5, 50, 1);
        glPopMatrix ();
    }
    gluDeleteQuadric (q);
}

static void 
draw (GeometryTester *super)
{
    ConvexPolyTester *self = (ConvexPolyTester*) super;
    glColor3f (1, 1, 0);


    pointlist2d_t *poly1 = pointlist2d_new_from_gqueue (self->poly1);
    pointlist2d_t *c1 = convexhull_graham_scan_2d (poly1);
    pointlist2d_free (poly1);

    pointlist2d_t *poly2 = pointlist2d_new_from_gqueue (self->poly2);
    pointlist2d_t *c2 = convexhull_graham_scan_2d (poly2);
    pointlist2d_free (poly2);

    if (c1) {
        glColor3f (1, 0, 0);
        printf ("convexhull of poly1\n");
        draw_polygon (c1);
    }
    if (c2) {
        glColor3f (1, 1, 0);
        printf ("convexhull of poly2\n");
        draw_polygon (c2);
    }

    if (c1 && c2) {
        pointlist2d_t *intersection = 
            geom_convex_polygon_convex_polygon_intersect_2d (c1, c2);

        if (intersection) {
            printf ("intersection has %d points\n", intersection->npoints);
            glColor3f (0, 0, 1);
            draw_polygon (intersection);
            pointlist2d_free (intersection);
        }
    }

    if (c1) pointlist2d_free (c1);
    if (c2) pointlist2d_free (c2);
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    ConvexPolyTester *self = (ConvexPolyTester*) super;

    if (event->button == 1) {
        point2d_t * newpoint = point2d_new (event->x, event->y);
        g_queue_push_tail (self->poly1, newpoint);
    } else if (event->button == 3) {
        point2d_t * newpoint = point2d_new (event->x, event->y);
        g_queue_push_tail (self->poly2, newpoint);
    } else if (event->button == 2) {
        while (! g_queue_is_empty (self->poly1)) {
            point2d_free ((point2d_t*) g_queue_pop_head (self->poly1));
        }
        while (! g_queue_is_empty (self->poly2)) {
            point2d_free ((point2d_t*) g_queue_pop_head (self->poly2));
        }
    }
}
