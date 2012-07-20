#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <lcm/lcm.h>
#include <gtk_util/gtk_util.h>
#include <common/small_linalg.h>
#include <common/glib_util.h>
#include <common/config.h>
#include <common/fasttrig.h>
#include <common/color_util.h>

#include <dgc/globals.h>
#include <dgc/config_util.h>

#include <glutil/glutil.h>
#include <lcmtypes/lcmtypes_track_list_t.h>
#include <lcmtypes/lcmtypes_obstacle_list_t.h>

#include <libviewer/viewer.h>

#define RENDERER_NAME "Tracks"

struct track_info
{
    lcmtypes_track_list_t *tlist;
    double        color[3];

    char          channel[256];
};

typedef struct _RendererTracks {
    Renderer renderer;

    Config          *config;
    lcm_t            *lc;
    GHashTable      *channels_hashtable;
    GPtrArray       *channels;
    Viewer          *viewer;
    GtkuParamWidget *pw;

    CTrans          *ctrans;
    int64_t         last_utime;
} RendererTracks;


static void my_free( Renderer *renderer )
{
    RendererTracks *self = (RendererTracks*) renderer->user;

    free(self);
}

static void my_draw( Viewer *viewer, Renderer *renderer )
{
    RendererTracks *self = (RendererTracks*) renderer->user;

    double carpos[3] = { 0, 0, 0 };
    ctrans_local_pos (self->ctrans, carpos);

    glLineWidth(1);
    glEnable(GL_BLEND);

    for (unsigned int chanidx = 0; chanidx < g_ptr_array_size(self->channels); chanidx++) {
        struct track_info *tdata = g_ptr_array_index(self->channels, chanidx);

        lcmtypes_track_list_t *tlist = tdata->tlist;
        for (int tidx = 0; tidx < tlist->ntracks; tidx++) {
            lcmtypes_track_t *t = &tlist->tracks[tidx];

            double x0 = t->pos[0];
            double y0 = t->pos[1];
            double sx = t->size[0]/2;
            double sy = t->size[1]/2;

            glColor4d(tdata->color[0], tdata->color[1], tdata->color[2], 0.5);
            double alpha = 1;
            glBegin(GL_LINES);
            glVertex3d(x0, y0, carpos[2]);
            glVertex3d(x0 + t->vel[0]*alpha, y0 + t->vel[1]*alpha, carpos[2]);
            glEnd();

            glPushMatrix();
            glTranslated(x0, y0, carpos[2]);
            glRotatef(to_degrees(t->theta), 0, 0, 1);
            glColor4d(tdata->color[0], tdata->color[1], tdata->color[2], 0.5);
            glBegin(GL_QUADS);
            glVertex2f( - sx, - sy);
            glVertex2f( - sx, + sy);
            glVertex2f( + sx, + sy);
            glVertex2f( + sx, - sy);
            glEnd();

            char buf[128];
            double velocity = sqrt(sq(t->vel[0]) + sq(t->vel[1]));
            sprintf(buf, "%5.1f (%d)", velocity, t->confidence);
            double pos[3] = { 0, 0, 0 };
            glutil_draw_text(pos, NULL, buf, GLUTIL_DRAW_TEXT_DROP_SHADOW);
  
            glPopMatrix();
        }
    }
}

static void on_tracks (const lcm_recv_buf_t *rbuf, const char *channel, 
        const lcmtypes_track_list_t *msg, void *user_data )
{
    RendererTracks *self = (RendererTracks*) user_data;

    struct track_info *tinfo = g_hash_table_lookup(self->channels_hashtable, channel);
    if (tinfo == NULL) {
        tinfo = (struct track_info*) calloc(1, sizeof(struct track_info));
        strcpy(tinfo->channel, channel);
        g_hash_table_insert(self->channels_hashtable, tinfo->channel, tinfo);
        g_ptr_array_add(self->channels, tinfo);

        char key[256];
        sprintf(key, "%s.viewer_color", channel);

        int sz = config_get_double_array(self->config, key, tinfo->color, 3);
        if (sz != 3) {
            printf("%s : funny color!\n", key);
            tinfo->color[0] = 1;
            tinfo->color[1] = 1;
            tinfo->color[2] = 1;
        }
    }

    if (tinfo->tlist)
        lcmtypes_track_list_t_destroy(tinfo->tlist);

    tinfo->tlist = lcmtypes_track_list_t_copy(msg);

    viewer_request_redraw(self->viewer);
}

static void
on_obstacles (const lcm_recv_buf_t *rbuf, const char * channel, const lcmtypes_obstacle_list_t * msg, void * user)
{
    on_tracks (rbuf, channel, &msg->tracks, user);
}

static void on_param_widget_changed (GtkuParamWidget *pw, const char *name, void *user)
{
    RendererTracks *self = (RendererTracks*) user;

    viewer_request_redraw(self->viewer);
}

static void on_load_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererTracks *self = user_data;
    gtku_param_widget_load_from_key_file (self->pw, keyfile, RENDERER_NAME);
}

static void on_save_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererTracks *self = user_data;
    gtku_param_widget_save_to_key_file (self->pw, keyfile, RENDERER_NAME);
}

void setup_renderer_tracks(Viewer *viewer, int priority) 
{
    RendererTracks *self = 
        (RendererTracks*) calloc(1, sizeof(RendererTracks));

    Renderer *renderer = &self->renderer;

    renderer->draw = my_draw;
    renderer->destroy = my_free;
    renderer->name = RENDERER_NAME;
    self->pw = GTKU_PARAM_WIDGET(gtku_param_widget_new());
    renderer->widget = GTK_WIDGET(self->pw);
    renderer->enabled = 1;
    renderer->user = self;

    self->lc = globals_get_lcm();
    self->ctrans = globals_get_ctrans();
    self->config = globals_get_config();
    self->channels_hashtable = g_hash_table_new(g_str_hash, g_str_equal);
    self->channels = g_ptr_array_new();
    self->viewer = viewer;

    g_signal_connect (G_OBJECT (self->pw), "changed", 
                      G_CALLBACK (on_param_widget_changed), self);
    
    lcmtypes_track_list_t_subscribe(self->lc, ".*TRACKS.*", on_tracks, self);
    lcmtypes_obstacle_list_t_subscribe (self->lc, "OBSTACLES", on_obstacles, self);

    viewer_add_renderer(viewer, renderer, priority);

    g_signal_connect (G_OBJECT (viewer), "load-preferences", 
            G_CALLBACK (on_load_preferences), self);
    g_signal_connect (G_OBJECT (viewer), "save-preferences",
            G_CALLBACK (on_save_preferences), self);

}
