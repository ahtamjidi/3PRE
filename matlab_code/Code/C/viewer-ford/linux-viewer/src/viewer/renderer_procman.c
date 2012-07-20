#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <glutil/glutil.h>

#include <gtk_util/gtk_util.h>

#include "libviewer/viewer.h"

typedef struct _RendererProcman RendererProcman;

struct _RendererProcman {
    Renderer renderer;

    GtkuParamWidget    *pw;

    Viewer *viewer;
};

static void
procman_draw (Viewer *viewer, Renderer *renderer)
{
//    RendererProcman *self = (RendererProcman*) renderer->user;

// TODO
}

static void
procman_free (Renderer *renderer) 
{
    free (renderer);
}

static void 
on_param_widget_changed (GtkuParamWidget *pw, const char *param,
        void *user_data)
{
    RendererProcman *self = (RendererProcman*) user_data;
    viewer_request_redraw (self->viewer);
}


Renderer *renderer_procman_new (Viewer *viewer)
{
    RendererProcman *self = 
        (RendererProcman*) calloc (1, sizeof (RendererProcman));
    self->viewer = viewer;
    self->renderer.draw = procman_draw;
    self->renderer.destroy = procman_free;
    self->renderer.name = "Procman";
    self->renderer.user = self;
    self->renderer.enabled = 1;

    self->renderer.widget = gtku_param_widget_new ();
    self->pw = GTKU_PARAM_WIDGET (self->renderer.widget);

    gtk_widget_show (GTK_WIDGET (self->pw));

    g_signal_connect (G_OBJECT (self->pw), "changed",
            G_CALLBACK (on_param_widget_changed), self);

    return &self->renderer;
}

void setup_renderer_procman(Viewer *viewer, int render_priority)
{
    viewer_add_renderer(viewer, renderer_procman_new(viewer), render_priority);
}
