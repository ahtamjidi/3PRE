#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/rotations.h>
#include <common/small_linalg.h>
#include <common/glib_util.h>
#include <dgc/globals.h>

#include <lcm/lcm.h>
#include <gtk_util/gtk_util.h>

#include <libviewer/viewer.h>

#include <lcmtypes/lcmtypes_lcgl_data_t.h>

#include "lcgl_decode.h"

typedef struct
{
    GPtrArray *backbuffer;
    GPtrArray *frontbuffer;
    int enabled;
} lcgl_channel_t;

typedef struct _RendererLcgl {
    Renderer renderer;
    GtkuParamWidget *pw; 
    Viewer   *viewer;
    lcm_t     *lc;

    GHashTable *channels;
    
} RendererLcgl;

static void my_free( Renderer *renderer )
{
    RendererLcgl *self = (RendererLcgl*) renderer;

    free( self );
}

static void my_draw( Viewer *viewer, Renderer *renderer )
{
    RendererLcgl *self = (RendererLcgl*) renderer->user;

    // iterate over each channel
    GPtrArray *keys = gu_hash_table_get_keys(self->channels);

    for (int k = 0; k < g_ptr_array_size(keys); k++) {
        lcgl_channel_t *chan = g_hash_table_lookup(self->channels, 
                                                   g_ptr_array_index(keys, k));
        glPushMatrix();
        glPushAttrib(GL_ENABLE_BIT);

        if (chan->enabled) {
            // iterate over all the messages received for this channel
            for (int i = 0; i < g_ptr_array_size(chan->frontbuffer); i++) {
                lcmtypes_lcgl_data_t *data = g_ptr_array_index(chan->frontbuffer, i);
                
                lcgl_decode(data->data, data->datalen);
            }
        }
        glPopAttrib ();
        glPopMatrix();
    }
    g_ptr_array_free(keys, TRUE);
}

static void on_lcgl_data (const lcm_recv_buf_t *rbuf, const char *channel, 
        const lcmtypes_lcgl_data_t *_msg, void *user_data )
{
    RendererLcgl *self = (RendererLcgl*) user_data;

    lcgl_channel_t *chan = g_hash_table_lookup(self->channels, _msg->name);
        
    if (!chan) {
        chan = (lcgl_channel_t*) calloc(1, sizeof(lcgl_channel_t));
        chan->enabled=1;
        //chan->backbuffer = g_ptr_array_new();
        chan->frontbuffer = g_ptr_array_new();
        g_hash_table_insert(self->channels, strdup(_msg->name), chan);
        gtku_param_widget_add_booleans (self->pw, 0, strdup(_msg->name), 1, NULL);
    }

#if 0
    int current_scene = -1;
    if (g_ptr_array_size(chan->backbuffer) > 0) {
        lcmtypes_lcgl_data_t *ld = g_ptr_array_index(chan->backbuffer, 0);
        current_scene = ld->scene;
    }

    // new scene?
    if (current_scene != _msg->scene) {

        // free objects in foreground buffer
        for (int i = 0; i < g_ptr_array_size(chan->frontbuffer); i++)
            lcmtypes_lcgl_data_t_destroy(g_ptr_array_index(chan->frontbuffer, i));
        g_ptr_array_set_size(chan->frontbuffer, 0);
        
        // swap front and back buffers
        GPtrArray *tmp = chan->backbuffer;
        chan->backbuffer = chan->frontbuffer;
        chan->frontbuffer = tmp;
        
        viewer_request_redraw( self->viewer );
    }
#endif
    
    for (int i = 0; i < g_ptr_array_size(chan->frontbuffer); i++)
        lcmtypes_lcgl_data_t_destroy(g_ptr_array_index(chan->frontbuffer, i));
    g_ptr_array_set_size (chan->frontbuffer, 0);
    g_ptr_array_add(chan->frontbuffer, lcmtypes_lcgl_data_t_copy(_msg));
    viewer_request_redraw( self->viewer );
}

static void on_param_widget_changed (GtkuParamWidget *pw, const char *name, void *user)
{
    RendererLcgl *self = (RendererLcgl*) user;

    // iterate over each channel
    GPtrArray *keys = gu_hash_table_get_keys(self->channels);

    for (int k = 0; k < g_ptr_array_size(keys); k++) {  
        lcgl_channel_t *chan = g_hash_table_lookup(self->channels, 
                                                   g_ptr_array_index(keys, k));
        
        chan->enabled = gtku_param_widget_get_bool (pw, g_ptr_array_index(keys, k));
    }
    g_ptr_array_free(keys, TRUE);

    viewer_request_redraw(self->viewer);
}


void setup_renderer_lcgl(Viewer *viewer, int priority)
{
    RendererLcgl *self = 
        (RendererLcgl*) calloc(1, sizeof(RendererLcgl));

    Renderer *renderer = &self->renderer;

    self->lc = globals_get_lcm();
    self->viewer = viewer;
    self->pw = GTKU_PARAM_WIDGET(gtku_param_widget_new());

    renderer->draw = my_draw;
    renderer->destroy = my_free;
    renderer->name = "LcGL";
    renderer->widget = GTK_WIDGET(self->pw);
    renderer->enabled = 1;
    renderer->user = self;


    self->channels = g_hash_table_new(g_str_hash, g_str_equal);

    g_signal_connect (G_OBJECT (self->pw), "changed", 
                      G_CALLBACK (on_param_widget_changed), self);

    lcmtypes_lcgl_data_t_subscribe(self->lc, "LCGL.*", on_lcgl_data, self);

    viewer_add_renderer(viewer, renderer, priority);
}
