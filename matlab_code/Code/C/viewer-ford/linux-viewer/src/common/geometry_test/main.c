#include <stdio.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <gtk_util/gtk_util.h>

#include <common/glib_util.h>
#include <common/geometry.h>
#include <common/convexhull.h>

#include "geometry_tester.h"


GeometryTester * line_seg_line_seg_intersect_tester_new ();
GeometryTester * convex_polygon_convex_polygon_intersect_tester_new ();
GeometryTester * convex_polygon_covered_points_2i_tester_new ();
GeometryTester * polyline_offset_tester_new ();
GeometryTester * circle_circle_intersect_tester_new ();
GeometryTester * point_circle_tangent_tester_new ();
GeometryTester * circle_circle_tangents_tester_new ();
GeometryTester * point_inside_polygon_tester_new ();
GeometryTester * polyline_shift_tester_new ();
GeometryTester * simple_polygon_centroid_tester_new ();
GeometryTester * polygon_edge_points_tester_new ();
GeometryTester * polygon_covered_points_tester_new ();
GeometryTester * point_line_closest_point_tester_new ();
GeometryTester * hermite_spline_tester_new ();
GeometryTester * polyline_advance_point_tester_new ();
GeometryTester * convex_polygon_dilate_tester_new ();
GeometryTester * point_polyline_closest_point_tester_new ();
GeometryTester * quadratic_spline_tester_new ();
GeometryTester * polyline_subsection_tester_new ();

typedef struct _state_t {
    GtkWidget *combo;
    GtkWidget *widget_vbox;
    GtkWidget *gl_area;
    GList *testers;

    GeometryTester *active_tester;
} state_t;

static void
activate_tester (state_t *self, GeometryTester *tester)
{
    printf ("activating [%s]\n", tester->name);
    if (self->active_tester && self->active_tester->widget) {
        gtk_widget_hide (self->active_tester->widget);
    }
    self->active_tester = tester;
    if (tester->widget) {
        gtk_widget_show (tester->widget);
    }
    int index = 0;
    for (GList *titer = self->testers; titer; titer=titer->next) {
        GeometryTester *ctester = (GeometryTester*) titer->data;
        if (!strcmp (tester->name, ctester->name)) break;
        index++;
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX (self->combo), index);
    gtku_gl_drawing_area_invalidate (GTKU_GL_DRAWING_AREA (self->gl_area));
}

static gboolean
on_gl_expose (GtkWidget * widget, GdkEventExpose * event, gpointer user_data)
{
    state_t * self = (state_t *) user_data;
    GtkuGLDrawingArea *gl_area = GTKU_GL_DRAWING_AREA (widget);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (0, widget->allocation.width, 0, widget->allocation.height, -1, 1);
    glMatrixMode (GL_MODELVIEW);

    if (self->active_tester && self->active_tester->draw) {
        self->active_tester->draw (self->active_tester);
    }
    
    gtku_gl_drawing_area_swap_buffers (gl_area);
    return TRUE;
}


static gboolean 
on_button_press (GtkWidget *widget, GdkEventButton *event, void *user_data)
{
    state_t *self = (state_t*) user_data;
    GtkuGLDrawingArea *gl_area = GTKU_GL_DRAWING_AREA (widget);

    event->y = widget->allocation.height - event->y;
    if (self->active_tester && self->active_tester->mouse_press) {
        self->active_tester->mouse_press (self->active_tester, event);
    }

    gtku_gl_drawing_area_invalidate (gl_area);
    return TRUE;
}

static gboolean 
on_button_release (GtkWidget *widget, GdkEventButton *event, void *user_data)
{
    state_t *self = (state_t*) user_data;
    GtkuGLDrawingArea *gl_area = GTKU_GL_DRAWING_AREA (widget);

    event->y = widget->allocation.height - event->y;
    if (self->active_tester && self->active_tester->mouse_release) {
        self->active_tester->mouse_release (self->active_tester, event);
    }

    gtku_gl_drawing_area_invalidate (gl_area);
    return TRUE;
}

static gboolean
on_motion_notify (GtkWidget *widget, GdkEventMotion *motion, void *user_data)
{
    state_t *self = (state_t*) user_data;
    GtkuGLDrawingArea *gl_area = GTKU_GL_DRAWING_AREA (widget);

    motion->y = widget->allocation.height - motion->y;

    if (self->active_tester && self->active_tester->mouse_motion) {
        self->active_tester->mouse_motion (self->active_tester, motion);
        gtku_gl_drawing_area_invalidate (gl_area);
    }
    return TRUE;
}

static void
on_combo_changed (GtkComboBox *combo, void *user_data)
{
    state_t *self = (state_t*) user_data;
    char *text = gtk_combo_box_get_active_text (combo);
    for (GList *titer = self->testers; titer; titer=titer->next) {
        GeometryTester *ctester = (GeometryTester*) titer->data;
        if (!strcmp (text, ctester->name)) {
            if (ctester != self->active_tester) {
                activate_tester (self, ctester);
            }
            break;
        }
    }
    free (text);
}

static void
setup_gtk (state_t *self)
{
    // create the main application window
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (G_OBJECT (window), "delete_event", gtk_main_quit, NULL);
    g_signal_connect (G_OBJECT (window), "destroy", gtk_main_quit, NULL);
    gtk_window_set_default_size (GTK_WINDOW (window), 600, 400);
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_widget_show (vbox);

    // listbox chooser
    self->combo = gtk_combo_box_new_text ();
    gtk_box_pack_start (GTK_BOX (vbox), self->combo, FALSE, TRUE, 0);
    gtk_widget_show (self->combo);

    g_signal_connect (G_OBJECT (self->combo), "changed", 
            G_CALLBACK (on_combo_changed), self);

    GtkWidget *hpane = gtk_hpaned_new ();
    gtk_box_pack_start (GTK_BOX (vbox), hpane, TRUE, TRUE, 0);
    gtk_widget_show (hpane);

    // OpenGL drawing area
    self->gl_area = gtku_gl_drawing_area_new (TRUE);
    gtk_widget_set_events (self->gl_area, GDK_BUTTON_PRESS_MASK |
            GDK_BUTTON_MOTION_MASK );
    gtk_paned_pack1 (GTK_PANED (hpane), self->gl_area, TRUE, TRUE);
    gtk_widget_show (self->gl_area);

    self->widget_vbox = gtk_vbox_new (FALSE, 0);
    gtk_paned_pack2 (GTK_PANED (hpane), self->widget_vbox, FALSE, TRUE);
    gtk_widget_show (self->widget_vbox);

    // connect mouse and redraw signal handlers
    g_signal_connect (G_OBJECT (self->gl_area), "expose-event",
            G_CALLBACK (on_gl_expose), self);
    g_signal_connect (G_OBJECT (self->gl_area), "button-press-event",
            G_CALLBACK (on_button_press), self);
    g_signal_connect (G_OBJECT (self->gl_area), "button-release-event",
            G_CALLBACK (on_button_release), self);
    g_signal_connect (G_OBJECT (self->gl_area), "motion-notify-event",
            G_CALLBACK (on_motion_notify), self);

    gtk_widget_show (window);
}

static void
add_tester (state_t *self, GeometryTester *tester)
{
    self->testers = g_list_append (self->testers, tester);
    printf ("adding [%s]\n", tester->name);
    gtk_combo_box_append_text (GTK_COMBO_BOX (self->combo), tester->name);
    if (tester->widget) {
        gtk_box_pack_start (GTK_BOX (self->widget_vbox), tester->widget,
                FALSE, FALSE, 0);
        gtk_widget_hide (tester->widget);
    }
    tester->priv = self;
}

void geometry_tester_request_redraw (GeometryTester *tester)
{
    state_t *self = tester->priv;
    gtku_gl_drawing_area_invalidate (GTKU_GL_DRAWING_AREA (self->gl_area));
}

int main (int argc, char **argv)
{
    gtk_init (&argc, &argv);

    state_t *self = (state_t*)calloc (1, sizeof (state_t));
    setup_gtk (self);

    glClearColor (0,0,0,0);

    add_tester (self, line_seg_line_seg_intersect_tester_new ());
    add_tester (self, convex_polygon_convex_polygon_intersect_tester_new ());
    add_tester (self, convex_polygon_covered_points_2i_tester_new ());
    add_tester (self, polyline_offset_tester_new ());
    add_tester (self, circle_circle_intersect_tester_new ());
    add_tester (self, point_circle_tangent_tester_new ());
    add_tester (self, circle_circle_tangents_tester_new ());
    add_tester (self, point_inside_polygon_tester_new ());
    add_tester (self, polyline_shift_tester_new ());
    add_tester (self, simple_polygon_centroid_tester_new ());
    add_tester (self, polygon_edge_points_tester_new ());
    add_tester (self, polygon_covered_points_tester_new ());
    add_tester (self, point_line_closest_point_tester_new ());
    add_tester (self, hermite_spline_tester_new ());
    add_tester (self, polyline_advance_point_tester_new ());
    add_tester (self, point_polyline_closest_point_tester_new ());
    add_tester (self, quadratic_spline_tester_new ());
    add_tester (self, polyline_subsection_tester_new ());

    if (self->testers) {
        activate_tester (self, self->testers->data);
    }

    gtk_main ();

    // cleanup
    for (GList *titer = self->testers; titer; titer=titer->next) {
        GeometryTester *ctester = (GeometryTester*) titer->data;
        if (ctester->destroy) ctester->destroy (ctester);
    }
    g_list_free (self->testers);
    free (self);
    return 0;
}
