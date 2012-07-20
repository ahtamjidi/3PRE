#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>

#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <common/rotations.h>
#include <common/small_linalg.h>
#include <common/glib_util.h>
#include <common/math_util.h>
#include <common/fasttrig.h>
#include <dgc/globals.h>
#include <dgc/config_util.h>
#include <gtk_util/gtk_util.h>


#include <lcmtypes/lcmtypes_canbus_list_t.h>
#include <dgc/delphi.h>

#include <libviewer/viewer.h>

#define PARAM_SHOW_VELOCITY "Show velocity"
#define PARAM_SUBTRACT_VELOCITY "Subtract velocity"

typedef struct _RendererRadar RendererRadar;

struct radar_hit
{
    int64_t utime;
    double range;
    double bearing;
    double range_rate;

    double xyz[3];  // position in local
    double vxyz[3]; // velocity in local
    float  *color;
};

struct _RendererRadar {
    Renderer renderer;

    Config *config;
    lcm_t *lc;
    CTrans *ctrans;

    GHashTable *radar_data;
    GHashTable *radar_calib;
    GHashTable *radar_colors;
    Viewer *viewer;
    GtkuParamWidget *pw;
    GPtrArray *raw_hits;
};

static void rand_color(float *f)
{
again:
    f[0] = randf();
    f[1] = randf();
    f[2] = randf();
    f[3] = 0.6;

    float v = f[0] + f[1] + f[2];

    // reject colors that are too dark
    if (v < 0.3)
        goto again;
}

static void project(RendererRadar *self, double mat[16], double range, double theta, double local_xyz[3])
{
    double sensor_xyz[4], local[4];
    double s, c;
    fasttrig_sincos(theta, &s, &c);
    
    sensor_xyz[0] = c * range;
    sensor_xyz[1] = s * range;
    sensor_xyz[2] = 0;
    sensor_xyz[3] = 1;
    matrix_vector_multiply_4x4_4d(mat, sensor_xyz, local);
    local_xyz[0]=local[0]/local[3];
    local_xyz[1]=local[1]/local[3];
    local_xyz[2]=local[2]/local[3];
}

static void do_vertex(RendererRadar *self, double mat[16], double range, double theta)
{
    double local_xyz[3];
    project(self, mat, range, theta, local_xyz);
    glVertex3dv(local_xyz);
}


static void radar_draw (Viewer *viewer, Renderer *super)
{
    RendererRadar *self = (RendererRadar*) super;

    glPushAttrib(GL_ENABLE_BIT);

    glPointSize(5.0);
    glDisable (GL_DEPTH_TEST);
    glEnable (GL_BLEND);
    glEnable(GL_POINT_SMOOTH);

    // draw the raw hits
    if (1) {
        glPointSize(10.0);
        glBegin(GL_POINTS);
        for (unsigned int i = 0; i < g_ptr_array_size(self->raw_hits); i++) {
            struct radar_hit *rh = g_ptr_array_index(self->raw_hits, i);
            rh->color[3] = 0.6;
            glColor4fv(rh->color);            
            glVertex3dv(rh->xyz);
            
        }
        glEnd();
        
        int show_velocity = gtku_param_widget_get_bool(self->pw, PARAM_SHOW_VELOCITY);
        if (show_velocity) {
            // draw velocity TAILS
            glBegin(GL_LINES);
            for (unsigned int i = 0; i < g_ptr_array_size(self->raw_hits); i++) {
                struct radar_hit *rh = g_ptr_array_index(self->raw_hits, i);
                rh->color[3] = 0.6;
                glColor4fv(rh->color);            
                glVertex3dv(rh->xyz);
                double xyz[3];
                for (int j = 0; j < 3; j++)
                    xyz[j] = rh->xyz[j] - 2.0*rh->vxyz[j];
                glVertex3dv(xyz);            
            }
            glEnd();
        }
    }

    GPtrArray *keys = gu_hash_table_get_keys(self->radar_data);
    for (int i = 0; i < g_ptr_array_size(keys); i++) {
        const char *key = g_ptr_array_index(keys, i);
        
        delphi_data_t *dd = g_hash_table_lookup(self->radar_data, key);

        if (dd == NULL)
            continue;
        double *mat = g_hash_table_lookup(self->radar_calib, key);
        if (mat) {
            float *color = g_hash_table_lookup(self->radar_colors, key);
            assert(color);
            // draw background triangle
            if (1) {
                glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                color[3] = 0.05;
                glColor4fv(color);
                glBegin(GL_TRIANGLES);
                double range = 100;
                do_vertex(self, mat, 0, 0);
                do_vertex(self, mat, range, to_radians(-9));
                do_vertex(self, mat, range, to_radians(9));
                glEnd();
            }
        }
    }

    glPopAttrib ();
    g_ptr_array_free(keys, TRUE);
}

static void radar_free( Renderer *super ) 
{
    RendererRadar *self = (RendererRadar*) super;
    g_hash_table_destroy( self->radar_data );
    g_hash_table_destroy( self->radar_calib );
    free( super );
}

// callback function for radar target
static int on_canbus_list(const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_canbus_list_t *cbl, void *user_data)
{
    RendererRadar *self = (RendererRadar*) user_data;
    GtkuParamWidget *pw = self->pw;
    int subtract_velocity = gtku_param_widget_get_bool(pw, PARAM_SUBTRACT_VELOCITY);

    double *mat = g_hash_table_lookup(self->radar_calib, channel);
    if (mat == NULL) {
        mat = calloc(1,sizeof(double)*16);
        g_hash_table_insert(self->radar_calib, strdup(channel), mat);

    }
    char config_path[1024];
    sprintf(config_path, "calibration.%s", channel);
    if (config_util_sensor_to_local(channel, mat)) {
        printf("No calibration for %s\n", config_path);
        return 0;
    }

    float *color = g_hash_table_lookup(self->radar_colors, channel);
    if (color == NULL) {
        color = (float*) calloc(4, sizeof(float));
        srand(g_str_hash(channel)+1);
        rand_color(color);
        color[3] = 1;
        g_hash_table_insert(self->radar_colors, strdup(channel), color);
    }

    delphi_data_t *dd = g_hash_table_lookup(self->radar_data, channel);
    if (dd == NULL) {
        dd = (delphi_data_t*) calloc(1, sizeof(delphi_data_t));
        g_hash_table_insert(self->radar_data, strdup(channel), dd);
    }

    for (int i = 0; i < cbl->num_messages; i++) {
        lcmtypes_canbus_t *cb = &cbl->messages[i];
        int res = delphi_decode_message(dd, cb);
        assert(res==0);
        if (delphi_is_last_message(cb)) {
            viewer_request_redraw( self->viewer );
        }
    }

    // remove stale hits
    for (int i = 0; i < g_ptr_array_size(self->raw_hits); i++) {
        struct radar_hit *rh = g_ptr_array_index(self->raw_hits, i);

        double age = abs(cbl->utime - rh->utime)/1000000.0;
        if (age > 0.15) {
            g_ptr_array_remove_index_fast(self->raw_hits, i);
            free(rh);
            i--;
        }
    }

    lcmtypes_pose_t cur_pose;
    ctrans_local_pose (self->ctrans, &cur_pose);

    // add new raw hits to the buffer
    for (int i = 0; i < DELPHI_NUM_TRACKS; i++) {

        if (dd->track[i].has_new_unfiltered_data) {
            
            struct radar_hit *rh = calloc(1, sizeof(struct radar_hit));
            rh->utime = cbl->utime;
            rh->range = dd->track[i].unfiltered.range;
            rh->bearing = dd->track[i].unfiltered.bearing;
            rh->range_rate = dd->track[i].unfiltered.range_rate;
            rh->color = color;

            project(self, mat, rh->range, rh->bearing, rh->xyz);

            // compute velocity
            double closing_rate = rh->range_rate;
            
            double lxyz_far[3], lxyz_near[3];
            project(self, mat, 1.0, rh->bearing, lxyz_far);
            project(self, mat, 0.0, rh->bearing, lxyz_near);
            double dot = (lxyz_far[0] - lxyz_near[0])*cur_pose.vel[0] 
                + (lxyz_far[1] - lxyz_near[1])*cur_pose.vel[1];
            
            if (subtract_velocity) {
                closing_rate += dot;
            }
            
            for (int j = 0; j < 3; j++)
                rh->vxyz[j] = (lxyz_far[j] - lxyz_near[j]) * closing_rate;

            g_ptr_array_add(self->raw_hits, rh);
            
            dd->track[i].has_new_unfiltered_data = 0;
        }
    }
    
    return 0;
}

void setup_renderer_radar(Viewer *viewer, int priority) 
{
    RendererRadar *self = 
        (RendererRadar*) calloc(1, sizeof(RendererRadar));

    Renderer *renderer = &self->renderer;

    renderer->draw = radar_draw;
    renderer->destroy = radar_free;
    renderer->name = "Radar";
    self->pw = GTKU_PARAM_WIDGET(gtku_param_widget_new());
    renderer->widget = GTK_WIDGET(self->pw);
    renderer->enabled = 1;
    renderer->user = self;

    self->lc = globals_get_lcm();
    self->ctrans = globals_get_ctrans();
    self->config = globals_get_config();
    //self->channels_hashtable = g_hash_table_new(g_str_hash, g_str_equal);
    //self->channels = g_ptr_array_new();
    self->viewer = viewer;

    //gtku_param_widget_add_int (self->pw, PARAM_MEMORY, 
    //                         GTKU_PARAM_WIDGET_SLIDER, 0, 16, 1, 1);

    //g_signal_connect (G_OBJECT (self->pw), "changed", 
    //                G_CALLBACK (on_param_widget_changed), self);
    

    gtku_param_widget_add_booleans (self->pw, 0, PARAM_SUBTRACT_VELOCITY, 1, NULL);
    gtku_param_widget_add_booleans (self->pw, 0, PARAM_SHOW_VELOCITY, 0, NULL);

    self->radar_data = g_hash_table_new(g_str_hash, g_str_equal);
    self->radar_calib = g_hash_table_new(g_str_hash, g_str_equal);
    self->radar_colors = g_hash_table_new(g_str_hash, g_str_equal);

    self->raw_hits = g_ptr_array_new();

    // cant' subscribe to RADAR.* because of RADAR_TRACKS
    lcmtypes_canbus_list_t_subscribe(self->lc, "RADAR_FRONT.*", on_canbus_list, self);
    lcmtypes_canbus_list_t_subscribe(self->lc, "RADAR_ROOF.*", on_canbus_list, self);
    viewer_add_renderer(viewer, renderer, priority);
}
