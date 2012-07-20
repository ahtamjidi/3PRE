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

#include <lcmtypes/lcmtypes_laser_t.h>

#include <libviewer/viewer.h>

#define RENDERER_NAME "Laser"
#define PARAM_MEMORY "Scan memory"
#define PARAM_SPATIAL_DECIMATE "Spatial Decimation"
#define PARAM_BIG_POINTS "Big points"

#define COLOR_MENU "Color"
enum {
    COLOR_DRAB,
    COLOR_INTENSITY,
    COLOR_LASER,
};

struct laser_data
{
    lcmtypes_laser_t  *msg;
    double   m[16];
};

struct laser_channel
{
    char      name[256];

    GUPtrCircular *data;

    double    color[3];
    char      calib[256];
    int       complained;
};

typedef struct _RendererLaser {
    Renderer renderer;

    Config *config;
    lcm_t *lc;
    GHashTable *channels_hashtable;
    GPtrArray  *channels;
    Viewer *viewer;
    GtkuParamWidget *pw;

    CTrans * ctrans;
    int64_t last_utime;
} RendererLaser;

static void free_ldata(void *user, void *p)
{
    struct laser_data *ldata = (struct laser_data*) p;
    lcmtypes_laser_t_destroy(ldata->msg);
    free(ldata);
}

static void my_free( Renderer *renderer )
{
    RendererLaser *self = (RendererLaser*) renderer->user;

    free( self );
}

static void my_draw( Viewer *viewer, Renderer *renderer )
{
    RendererLaser *self = (RendererLaser*) renderer->user;

    int colormode = gtku_param_widget_get_enum(self->pw, COLOR_MENU);
    
    for (unsigned int chanidx = 0; chanidx < g_ptr_array_size(self->channels); chanidx++) {
        struct laser_channel *lchannel = g_ptr_array_index(self->channels, chanidx);

        if (gtku_param_widget_get_bool(self->pw, PARAM_BIG_POINTS))
            glPointSize(8.0f);
        else
            glPointSize(4.0f);

        glBegin(GL_POINTS);
        
        for (unsigned int didx = 0; didx < gu_ptr_circular_size(lchannel->data); didx++) {
            struct laser_data *ldata = (struct laser_data*) gu_ptr_circular_index(lchannel->data, didx);

            for (int rangeidx = 0; rangeidx < ldata->msg->nranges; rangeidx++) {
                double range = ldata->msg->ranges[rangeidx];
                double theta = ldata->msg->rad0 + ldata->msg->radstep*rangeidx;

                switch(colormode) 
                {
                case COLOR_INTENSITY:
                    if (ldata->msg->nintensities == ldata->msg->nranges) {
                        double v = ldata->msg->intensities[rangeidx];
                        if (v > 1) {
                            // old non-normalized encoding
                            double minv = 7000, maxv = 12000;
                            v = (v-minv)/(maxv-minv);
                        }
                        glColor3fv(color_util_jet(v));
                        break;
                    }
                    // no intensity data? fall through!
                case COLOR_DRAB:
                    glColor3d(0.3, 0.3, 0.3);
                    break;
                case COLOR_LASER:
                    glColor3d(lchannel->color[0], lchannel->color[1], lchannel->color[2]);
                    break;
                }
                
                if (range > 79.0)
                    continue;
                
                double sensor_xyz[4], local_xyz[4];
                double s, c;
                fasttrig_sincos(theta, &s, &c);
                
                sensor_xyz[0] = c * range;
                sensor_xyz[1] = s * range;
                sensor_xyz[2] = 0;
                sensor_xyz[3] = 1;
                
                matrix_vector_multiply_4x4_4d(ldata->m, sensor_xyz, local_xyz);
                glVertex3dv(local_xyz);
            }
        }

        glEnd();
    }
}

static void on_laser (const lcm_recv_buf_t *rbuf, const char *channel, 
        const lcmtypes_laser_t *msg, void *user_data )
{
    RendererLaser *self = (RendererLaser*) user_data;

    if (!ctrans_have_pose (self->ctrans))
        return;

    double dt = (msg->utime - self->last_utime)/1000000.0;
    self->last_utime = msg->utime;
    if (fabs(dt) > 3) {
        for (unsigned int i = 0; i < g_ptr_array_size(self->channels); i++) {
            struct laser_channel *lchan = (struct laser_channel*) g_ptr_array_index(self->channels, i);
            gu_ptr_circular_clear(lchan->data);
        }
    }

    struct laser_channel *lchannel = g_hash_table_lookup(self->channels_hashtable, channel);
    if (lchannel == NULL) {
        lchannel = (struct laser_channel*) calloc(1, sizeof(struct laser_channel));
        unsigned int capacity = gtku_param_widget_get_int(self->pw, PARAM_MEMORY);
        lchannel->data = gu_ptr_circular_new(capacity, free_ldata, NULL);

        strcpy(lchannel->name, channel);
        g_hash_table_insert(self->channels_hashtable, lchannel->name, lchannel);
        g_ptr_array_add(self->channels, lchannel);

        char key[256];
        sprintf(key, "%s.viewer_color", channel);

        int sz = config_get_double_array(globals_get_config(), key, lchannel->color, 3);
        if (sz != 3) {
            printf("%s : missing or funny color!\n", key);
            lchannel->color[0] = 1;
            lchannel->color[1] = 1;
            lchannel->color[2] = 1;
        }
    }

    struct laser_data *ldata = (struct laser_data*) calloc(1, sizeof(struct laser_data));

    if (config_util_sensor_to_local(channel, ldata->m)) {
        if (!lchannel->complained)
            printf("Didn't get calibration for %s\n", channel);
        lchannel->complained = 1;
        return;
    }

    if (gtku_param_widget_get_bool(self->pw, PARAM_SPATIAL_DECIMATE) &&
        gtku_param_widget_get_int(self->pw, PARAM_MEMORY) > 10 && gu_ptr_circular_size(lchannel->data) > 0) {
        struct laser_data *last_ldata = gu_ptr_circular_index(lchannel->data, 0);
        double dist = sqrt(sq(last_ldata->m[3] - ldata->m[3]) +
                           sq(last_ldata->m[7] - ldata->m[7]));

        if (dist < 0.2) {
            free(ldata);
            return;
        }
    }

    ldata->msg = lcmtypes_laser_t_copy(msg);
    gu_ptr_circular_add(lchannel->data, ldata);

    viewer_request_redraw(self->viewer);
    return;
}

static void on_param_widget_changed (GtkuParamWidget *pw, const char *name, void *user)
{
    RendererLaser *self = (RendererLaser*) user;
    unsigned int capacity = gtku_param_widget_get_int(self->pw, PARAM_MEMORY);

    for (unsigned int i = 0; i < g_ptr_array_size(self->channels); i++) {
        struct laser_channel *lchannel = g_ptr_array_index(self->channels, i);
        gu_ptr_circular_resize(lchannel->data, capacity);
    }

    viewer_request_redraw(self->viewer);
}

static void on_load_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererLaser *self = user_data;
    gtku_param_widget_load_from_key_file (self->pw, keyfile, RENDERER_NAME);
}

static void on_save_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererLaser *self = user_data;
    gtku_param_widget_save_to_key_file (self->pw, keyfile, RENDERER_NAME);
}

void setup_renderer_laser(Viewer *viewer, int priority) 
{
    RendererLaser *self = 
        (RendererLaser*) calloc(1, sizeof(RendererLaser));

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

    gtku_param_widget_add_int (self->pw, PARAM_MEMORY, 
                               GTKU_PARAM_WIDGET_SLIDER, 1, 200, 1, 1);

    gtku_param_widget_add_enum( self->pw, COLOR_MENU, GTKU_PARAM_WIDGET_MENU,
                                COLOR_LASER,
                                "Laser", COLOR_LASER,
                                "Drab", COLOR_DRAB,
                                "Intensity", COLOR_INTENSITY,
                                NULL);

    gtku_param_widget_add_booleans (self->pw, 0, PARAM_SPATIAL_DECIMATE, 0, NULL);
    gtku_param_widget_add_booleans (self->pw, 0, PARAM_BIG_POINTS, 0, NULL);

    g_signal_connect (G_OBJECT (self->pw), "changed", 
                      G_CALLBACK (on_param_widget_changed), self);
    
    lcmtypes_laser_t_subscribe(self->lc, "SKIRT_.*", on_laser, self);
    lcmtypes_laser_t_subscribe(self->lc, "BROOM_.*", on_laser, self);

    viewer_add_renderer(viewer, renderer, priority);

    g_signal_connect (G_OBJECT (viewer), "load-preferences", 
            G_CALLBACK (on_load_preferences), self);
    g_signal_connect (G_OBJECT (viewer), "save-preferences",
            G_CALLBACK (on_save_preferences), self);

}
