#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <common/small_linalg.h>
#include <common/math_util.h>
#include <glutil/glutil.h>

#include <dgc/globals.h>

#include <gtk_util/gtk_util.h>

#include "libviewer/viewer.h"
#include "dgcviewer.h"
//#include "gl_util.h"

#define PARAM_MODE "Mode"
#define PARAM_AZIMUTH "Rotation"

typedef struct _RendererGrid RendererGrid;

struct _RendererGrid {
    Renderer renderer;

    CTrans *ctrans;

    GtkuParamWidget    *pw;
    double             last_meters_per_grid;
    GtkWidget          *label;
    int                display_dl_ready;
    GLuint             circle_list_dl;
    Viewer *viewer;
};

enum {
    MODE_GRID,
    MODE_RINGS
};

/** round the input number to the next number of the form 1*10^n,
 * 2*10^n, or 5*10^n. */
static double round_to_125(double in)
{
    double v = 1;

    while (v < in) {
        if (v < in)
            v *= 2;
        if (v < in)
            v = v/2 * 5;
        if (v < in)
            v *= 2;
    }

    return v;
}

static double round_to_15 (double in)
{
    double v = 1;
    while (v < in) {
        if (v < in)
            v *= 5;
        if (v < in)
            v *= 2;
    }
    return v;
}

static double
get_eye_dist (Viewer *viewer, RendererGrid *self)
{
    double eye[3];
    double look[3];
    double up[3];

    viewer->view_handler->get_eye_look(viewer->view_handler, eye, look, up);

    double eye_dist = 0;
    for (int i = 0; i < 3; i++) {
        eye_dist += sq(look[i] - eye[i]);
    }

    return sqrt(eye_dist);
}

static void
draw_grid (Viewer *viewer, Renderer *renderer)
{
    RendererGrid *self = (RendererGrid*) renderer->user;

    double pos[3]={0,0,0};
    ctrans_local_pos (self->ctrans, pos);

    // when looking directly down at the world, about how many grids
    // should appear across the screen?
    double grids_per_screen = 10;

    double eye_dist = get_eye_dist (viewer, self);
    double meters_per_grid = round_to_125(eye_dist / grids_per_screen );

    char txt[64];
    snprintf (txt, sizeof (txt), "Spacing: %.0fm", meters_per_grid);
    gtk_label_set_text (GTK_LABEL (self->label), txt);

    double grid_ox = ceil (pos[0] / meters_per_grid) * meters_per_grid;
    double grid_oy = ceil (pos[1] / meters_per_grid) * meters_per_grid;
    double grid_oz = pos[2];
    int num_lines = 300;

    glPushMatrix ();
    glTranslatef (grid_ox, grid_oy, grid_oz);
    glRotatef (gtku_param_widget_get_double (self->pw, PARAM_AZIMUTH), 0, 0, 1);

    glColor4f(0.1, 0.2, 0.1, 0.5);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);
    glBegin(GL_TRIANGLES);
    glVertex2f((-num_lines/2)*meters_per_grid, (-num_lines/2)*meters_per_grid);
    glVertex2f((+num_lines/2)*meters_per_grid, (-num_lines/2)*meters_per_grid);
    glVertex2f((+num_lines/2)*meters_per_grid, (+num_lines/2)*meters_per_grid);
    glVertex2f((-num_lines/2)*meters_per_grid, (-num_lines/2)*meters_per_grid);
    glVertex2f((+num_lines/2)*meters_per_grid, (+num_lines/2)*meters_per_grid);
    glVertex2f((-num_lines/2)*meters_per_grid, (+num_lines/2)*meters_per_grid);
    glEnd();

    grid_oz+=.01;
    glLineWidth (1);
    glEnable (GL_DEPTH_TEST);
    glBegin (GL_LINES);
    glColor3f (0.2, 0.2, 0.2);

    for (int i=0; i<num_lines; i++) {
        glVertex2f ((-num_lines/2 + i) * meters_per_grid,
                - num_lines/2 * meters_per_grid);
        glVertex2f ((-num_lines/2 + i) * meters_per_grid,
                num_lines/2 * meters_per_grid);

        glVertex2f (- num_lines/2 * meters_per_grid,
                (-num_lines/2 + i) * meters_per_grid);
        glVertex2f (num_lines/2 * meters_per_grid,
                (-num_lines/2 + i) * meters_per_grid);
    }
    glEnd ();
    glDisable (GL_DEPTH_TEST);
    glDisable (GL_BLEND);

    glPopMatrix ();
}

static void
draw_rings (Viewer *viewer, Renderer *renderer)
{
    RendererGrid *self = (RendererGrid*) renderer->user;

    double pos[3] = { 0, 0, 0 };
    ctrans_local_pos (self->ctrans, pos);

    // when looking directly down at the world, about how many grids
    // should appear across the screen?
    double bright_rings_per_screen = 10;

    double eye_dist = get_eye_dist (viewer, self);
    double meters_per_bright_ring = 
        round_to_15 (eye_dist / bright_rings_per_screen);

    char txt[64];
    snprintf (txt, sizeof (txt), "Spacing: %.0fm", meters_per_bright_ring);
    gtk_label_set_text (GTK_LABEL (self->label), txt);

    GLUquadricObj * q = gluNewQuadric();

    glPushMatrix();
    glTranslatef(pos[0], pos[1], pos[2]);

    for (int i=0; i<50; i++) {
        double r = i * meters_per_bright_ring;
        glColor3f (0.2, 0.2, 0.2);
        glPushMatrix();
        glScalef(r,r,0);
        glCallList(self->circle_list_dl);
        glPopMatrix();

        double small_step = meters_per_bright_ring / 5;
        for (double j = r + small_step; 
                j < r + meters_per_bright_ring; 
                j += small_step) {
            glColor3f (0.03, 0.03, 0.03);
            glPushMatrix();
            glScalef(j,j,0);
            glCallList(self->circle_list_dl);
            glPopMatrix();
        }
    }

    glPopMatrix();
    gluDeleteQuadric(q);
}

static GLuint
compile_display_list( RendererGrid* self )
{
    GLuint dl = glGenLists(1);
    
    glutil_build_circle(dl);

    return dl;
}

static void
grid_draw (Viewer *viewer, Renderer *renderer)
{
    RendererGrid *self = (RendererGrid*) renderer->user;

    if ( !self->display_dl_ready ) {
        self->circle_list_dl = compile_display_list(self);
        self->display_dl_ready = 1;
    }

    if (gtku_param_widget_get_enum (self->pw, PARAM_MODE) == MODE_GRID) {
        draw_grid (viewer, renderer);
    } else {
        draw_rings (viewer, renderer);
    }
}

static void
grid_free (Renderer *renderer) 
{
    free (renderer);
}

static void 
on_param_widget_changed (GtkuParamWidget *pw, const char *param,
        void *user_data)
{
    RendererGrid *self = (RendererGrid*) user_data;

    viewer_request_redraw (self->viewer);
}


Renderer *renderer_grid_new (Viewer *viewer)
{
    RendererGrid *self = (RendererGrid*) calloc (1, sizeof (RendererGrid));
    self->viewer = viewer;
    self->renderer.draw = grid_draw;
    self->renderer.destroy = grid_free;
    self->renderer.name = "Grid";
    self->renderer.user = self;
    self->renderer.enabled = 1;
    self->display_dl_ready = 0;

    self->renderer.widget = gtk_alignment_new (0, 0.5, 1.0, 0);
    self->ctrans = globals_get_ctrans ();

    self->label = gtk_label_new ("Spacing: ???");
    self->pw = GTKU_PARAM_WIDGET (gtku_param_widget_new ());
    GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
    gtk_misc_set_alignment (GTK_MISC (self->label), 0.0, 0.5);
    gtk_container_add (GTK_CONTAINER (self->renderer.widget), vbox);
    gtk_widget_show (vbox);

    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (self->pw), FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), self->label, FALSE, TRUE, 0);
    gtk_widget_show (GTK_WIDGET (self->pw));
    gtk_widget_show (self->label);

    gtku_param_widget_add_enum (self->pw, PARAM_MODE, 0, 0,
            "Grid", MODE_GRID,
            "Rings", MODE_RINGS,
            NULL);
    gtku_param_widget_add_double (self->pw, PARAM_AZIMUTH,
            GTKU_PARAM_WIDGET_SLIDER,
            0, 90, 0.1, 0);

    g_signal_connect (G_OBJECT (self->pw), "changed",
            G_CALLBACK (on_param_widget_changed), self);

    return &self->renderer;
}

void setup_renderer_grid(Viewer *viewer, int render_priority)
{
    viewer_add_renderer(viewer, renderer_grid_new(viewer), render_priority);
}
