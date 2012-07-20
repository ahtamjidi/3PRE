#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/geometry.h>
#include <gtk_util/gtk_util.h>
#include "geometry_tester.h"

typedef struct _PolylineSubsectionTester {
    GeometryTester parent;

    GArray * line;

    GLUquadricObj *quadric;

    GtkuParamWidget *pw;
} PolylineSubsectionTester;

static void draw (GeometryTester *self);
static void mouse_press (GeometryTester *self, const GdkEventButton *event);
static void destroy (GeometryTester *self);
static void on_param_widget_changed (GtkuParamWidget *pw, const char *name, 
        void *user);

GeometryTester * 
polyline_subsection_tester_new ()
{
    PolylineSubsectionTester * self = (PolylineSubsectionTester*) 
        calloc (1, sizeof (PolylineSubsectionTester));

    self->parent.draw = draw;
    self->parent.mouse_press = mouse_press;
    self->parent.destroy = destroy;
    self->parent.name = "polyline subsection";

    self->line = g_array_new (FALSE, FALSE, sizeof (point2d_t));

    point2d_t p;
    p.x = 100; p.y = 100; g_array_append_val (self->line, p);
    p.x = 200; p.y = 100; g_array_append_val (self->line, p);
    p.x = 300; p.y = 100; g_array_append_val (self->line, p);
    p.x = 400; p.y = 100; g_array_append_val (self->line, p);

    self->quadric = gluNewQuadric ();

    self->parent.widget = gtku_param_widget_new ();
    self->pw = GTKU_PARAM_WIDGET (self->parent.widget);

    gtku_param_widget_add_int (self->pw, "start ind", 0, 0, 10, 1, 0);
    gtku_param_widget_add_double (self->pw, "start alpha", 0, 0, 1, 0.01, 0.5);
    gtku_param_widget_add_int (self->pw, "end ind", 0, 0, 10, 1, 2);
    gtku_param_widget_add_double (self->pw, "end alpha", 0, 0, 1, 0.01, 0.5);

    g_signal_connect (G_OBJECT (self->pw), "changed", 
                      G_CALLBACK (on_param_widget_changed), self);

    return (GeometryTester*) self;
}

static void 
destroy (GeometryTester *super)
{
    PolylineSubsectionTester *self = (PolylineSubsectionTester*) super;
    gluDeleteQuadric (self->quadric);
    free (self);
}

static void
draw_disk (PolylineSubsectionTester *self, double x, double y)
{
    glPushMatrix ();
    glTranslatef (x, y, 0);
    gluDisk (self->quadric, 0, 3, 20, 1);
    glPopMatrix ();
}

static void 
draw (GeometryTester *super)
{
    PolylineSubsectionTester *self = (PolylineSubsectionTester*) super;

    if (!self->line->len) return;

    pointlist2d_t *line = 
        pointlist2d_new_from_array ((point2d_t*)self->line->data, 
                self->line->len);

    glColor3f (1, 1, 0);
    glBegin (GL_LINE_STRIP);
    for (int i=0; i<line->npoints; i++) {
        glVertex2f (line->points[i].x, line->points[i].y);
    }
    glEnd ();
    glColor3f (0, 0.4, 0);
    for (int i=0; i<line->npoints; i++) {
        draw_disk (self, line->points[i].x, line->points[i].y);
    }

    int start_ind = gtku_param_widget_get_int (self->pw, "start ind");
    double start_alpha = gtku_param_widget_get_double (self->pw, "start alpha");
    int end_ind = gtku_param_widget_get_int (self->pw, "end ind");
    double end_alpha = gtku_param_widget_get_double (self->pw, "end alpha");

    pointlist2d_t *subsection = pointlist2d_new_copy_subsection (line, 
            start_ind, start_alpha, end_ind, end_alpha);
    if (subsection) {
        glColor3f (0, 1, 0);
        glBegin (GL_LINE_STRIP);
        for (int i=0; i<subsection->npoints; i++) {
            glVertex2f (subsection->points[i].x, subsection->points[i].y);
        }
        glEnd ();
        glColor3f (0, 1, 0);
        for (int i=0; i<subsection->npoints; i++) {
            draw_disk (self, subsection->points[i].x, subsection->points[i].y);
        }
        pointlist2d_free (subsection);
    }

    pointlist2d_free (line);
}

static void 
mouse_press (GeometryTester *super, const GdkEventButton *event)
{
    PolylineSubsectionTester *self = (PolylineSubsectionTester*) super;

    point2d_t p = { event->x, event->y };

    if (event->button == 1) {
        g_array_append_val (self->line, p);
    } else if (event->button == 2) {
        if (self->line->len) 
            g_array_remove_range (self->line, 0, self->line->len);
    }
}

static void
on_param_widget_changed (GtkuParamWidget *pw, const char *name, void *user)
{
    geometry_tester_request_redraw (user);
}
