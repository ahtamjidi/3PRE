#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include "geometry_tester.h"

typedef struct _PolylineAdvancePointTester {
    GeometryTester parent;

    GArray * line1;
    GArray * line2;

    GLUquadricObj *quadric;
} PolylineAdvancePointTester;

static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void destroy (GeometryTester *self);

GeometryTester * 
polyline_advance_point_tester_new ()
{
    PolylineAdvancePointTester * self = (PolylineAdvancePointTester*) 
        calloc (1, sizeof (PolylineAdvancePointTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.destroy = destroy;
    self->parent.name = "polyline advance point";

    self->line1 = g_array_new (FALSE, FALSE, sizeof (point2d_t));
    self->line2 = g_array_new (FALSE, FALSE, sizeof (point2d_t));

    point2d_t p;
    p.x = 100; p.y = 100; g_array_append_val (self->line1, p);
    p.x = 200; p.y = 100; g_array_append_val (self->line1, p);
    p.x = 300; p.y = 100; g_array_append_val (self->line1, p);

    p.x = 100; p.y = 300; g_array_append_val (self->line2, p);
    p.x = 300; p.y = 300; g_array_append_val (self->line2, p);

    self->quadric = gluNewQuadric ();

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    PolylineAdvancePointTester *self = (PolylineAdvancePointTester*) super;
    gluDeleteQuadric (self->quadric);
    free (self);
}

static void
draw_disk (PolylineAdvancePointTester *self, double x, double y)
{
    glPushMatrix ();
    glTranslatef (x, y, 0);
    gluDisk (self->quadric, 0, 3, 20, 1);
    glPopMatrix ();
}

static void 
draw (GeometryTester *super)
{
    PolylineAdvancePointTester *self = (PolylineAdvancePointTester*) super;

    if (self->line1->len) {
        pointlist2d_t *line1 = 
            pointlist2d_new_from_array ((point2d_t*)self->line1->data, 
                    self->line1->len);

        glColor3f (1, 1, 0);
        glBegin (GL_LINE_STRIP);
        for (int i=0; i<line1->npoints; i++) {
            glVertex2f (line1->points[i].x, line1->points[i].y);
        }
        glEnd ();

        double inc = 50;
        int ind = 0;
        double alpha = 0;

        point2d_t next_pt = line1->points[0];
        draw_disk (self, next_pt.x, next_pt.y);
        while (1) {
            int status = geom_polyline_advance_point_by_dist (line1,
                    ind, alpha, inc, &ind, &alpha, &next_pt);
            draw_disk (self, next_pt.x, next_pt.y);
            if (0 != status) break;
        }
        pointlist2d_free (line1);
    }

    printf ("===\n");

    if (self->line2->len) {
        pointlist2d_t *line2 = 
            pointlist2d_new_from_array ((point2d_t*)self->line2->data, 
                    self->line2->len);
        glColor3f (0, 0.5, 1);
        glBegin (GL_LINE_STRIP);
        for (int i=0; i<line2->npoints; i++) {
            glVertex2f (line2->points[i].x, line2->points[i].y);
        }
        glEnd ();

        double inc = -50;
        int ind = line2->npoints - 1;
        double alpha = 0;

        point2d_t next_pt = line2->points[ind];
        draw_disk (self, next_pt.x, next_pt.y);
        while (1) {
            int status = geom_polyline_advance_point_by_dist (line2,
                    ind, alpha, inc, &ind, &alpha, &next_pt);
            draw_disk (self, next_pt.x, next_pt.y);
            if (0 != status) break;
        }
        pointlist2d_free (line2);
    }
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    PolylineAdvancePointTester *self = (PolylineAdvancePointTester*) super;

    point2d_t p = { event->x, event->y };

    if (event->button == 1) {
        g_array_append_val (self->line1, p);
    } else if (event->button == 2) {
        if (self->line1->len) 
            g_array_remove_range (self->line1, 0, self->line1->len);
        if (self->line2->len) 
            g_array_remove_range (self->line2, 0, self->line2->len);
    } else if (event->button == 3) {
        g_array_append_val (self->line2, p);
    }
}
