#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include "geometry_tester.h"

//#define CHECK_INT

typedef struct _PointInsidePolygonTester {
    GeometryTester parent;

    GQueue * poly1;
} PointInsidePolygonTester;


static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void destroy (GeometryTester *self);

GeometryTester * 
point_inside_polygon_tester_new ()
{
    PointInsidePolygonTester * self = 
        (PointInsidePolygonTester*) calloc (1, 
                sizeof (PointInsidePolygonTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.destroy = destroy;
    self->parent.name = "point inside polygon";

    self->poly1 = g_queue_new ();

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    PointInsidePolygonTester *self = 
        (PointInsidePolygonTester*) super;
    while (! g_queue_is_empty (self->poly1)) {
#ifdef CHECK_INT
        point2i_free ( (point2i_t*) g_queue_pop_head (self->poly1));
#else
        point2d_free ( (point2d_t*) g_queue_pop_head (self->poly1));
#endif
    }
    g_queue_free (self->poly1);
    free (self);
}

static void 
draw (GeometryTester *super)
{
    PointInsidePolygonTester *self = 
        (PointInsidePolygonTester*) super;
    glColor3f (1, 1, 0);

#ifdef CHECK_INT
    pointlist2i_t *poly1 = pointlist2i_new_from_gqueue (self->poly1);
#else
    pointlist2d_t *poly1 = pointlist2d_new_from_gqueue (self->poly1);
#endif
    if (!poly1)
        return;

    glBegin (GL_LINE_LOOP);
    int i;
    for (i = 0; i < poly1->npoints; i++) {
        glVertex2d (poly1->points[i].x, poly1->points[i].y);
    }
    glEnd ();

#ifdef CHECK_INT
    polygon2i_t p = {
        .nlists = 1,
        .pointlists = poly1,
    };
#else
    polygon2d_t p = {
        .nlists = 1,
        .pointlists = poly1,
    };
#endif
    int vals[4];
    glGetIntegerv (GL_VIEWPORT, vals);
    glBegin (GL_POINTS);
    glColor3f (1, 1, 1);
    for (i = 0; i < vals[2]; i++) {
        int j;
        for (j = 0; j < vals[3]; j++) {
#ifdef CHECK_INT
            point2i_t pt = { .x = i, .y = j };
            if (geom_point_inside_polygon_2i (&pt, &p)) glVertex2i (i, j);
#else
            point2d_t pt = { .x = i, .y = j };
            if (geom_point_inside_polygon_2d (&pt, &p)) glVertex2i (i, j);
#endif
        }
    }
    glEnd ();

#ifdef CHECK_INT
    pointlist2i_free (poly1);
#else
    pointlist2d_free (poly1);
#endif
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    PointInsidePolygonTester *self = 
        (PointInsidePolygonTester*) super;

    if (event->button == 1) {
#ifdef CHECK_INT
        point2i_t * newpoint = point2i_new (event->x, event->y);
#else
        point2d_t * newpoint = point2d_new (event->x, event->y);
#endif
        g_queue_push_tail (self->poly1, newpoint);
    } else if (event->button == 3) {
        while (! g_queue_is_empty (self->poly1)) {
#ifdef CHECK_INT
            point2i_free ((point2i_t*) g_queue_pop_head (self->poly1));
#else
            point2d_free ((point2d_t*) g_queue_pop_head (self->poly1));
#endif
        }
    }
}
