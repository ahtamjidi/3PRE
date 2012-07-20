#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include <common/convexhull.h>
#include "geometry_tester.h"

typedef struct _SimplePolygonCentroidTester {
    GeometryTester parent;

    GQueue * points;

    double width;
    GLUquadricObj *quadric;
} SimplePolygonCentroidTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void destroy (GeometryTester *self);

GeometryTester * 
simple_polygon_centroid_tester_new ()
{
    SimplePolygonCentroidTester * self = 
        (SimplePolygonCentroidTester*) calloc (1, sizeof (SimplePolygonCentroidTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.destroy = destroy;
    self->parent.name = "polygon centroid";

    self->quadric = gluNewQuadric ();

    self->points = g_queue_new ();

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    SimplePolygonCentroidTester *self = (SimplePolygonCentroidTester*) super;
    gluDeleteQuadric (self->quadric);
    while (! g_queue_is_empty (self->points)) {
        point2d_free ((point2d_t*) g_queue_pop_head (self->points));
    }
    g_queue_free (self->points);

    free (self);
}

static void 
draw (GeometryTester *super)
{
    SimplePolygonCentroidTester *self = (SimplePolygonCentroidTester*) super;
    if (!g_queue_get_length (self->points)) return;

    glColor3f (1, 1, 0);

    pointlist2d_t *polyline = pointlist2d_new_from_gqueue (self->points);

    glBegin (GL_LINE_LOOP);
    for (int i=0; i<polyline->npoints-1; i++) {
        glVertex2f (polyline->points[i].x, polyline->points[i].y);
        glVertex2f (polyline->points[i+1].x, polyline->points[i+1].y);
    }
    glEnd ();

    point2d_t centroid = { 0, 0 };
    geom_simple_polygon_centroid_2d (polyline, &centroid);

    glColor3f (0, 1, 1);
    glPushMatrix ();
    glTranslatef (centroid.x, centroid.y, 0);
    gluDisk (self->quadric, 0, 5, 20, 1);
    glPopMatrix ();

    pointlist2d_free (polyline);
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    SimplePolygonCentroidTester *self = (SimplePolygonCentroidTester*) super;

    if (event->button == 1) {
        point2d_t * newpoint = point2d_new (event->x, event->y);
        g_queue_push_tail (self->points, newpoint);
    } else if (event->button == 2) {
        while (! g_queue_is_empty (self->points)) {
            point2d_free ((point2d_t*) g_queue_pop_head (self->points));
        }
    }
}
