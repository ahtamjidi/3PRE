#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <lcm/lcm.h>
#include <lcmtypes/lcmtypes_velodyne_t.h>

#include <gtk_util/gtk_util.h>

#include <common/small_linalg.h>
#include <common/glib_util.h>
#include <common/velodyne.h>
#include <common/math_util.h>
#include <common/color_util.h>

#include <dgc/globals.h>
#include <dgc/config_util.h>

#include <libviewer/viewer.h>

#define RENDERER_NAME "Velodyne"
#define COLOR_MENU "Color"
#define THETA_LOW "T0"
#define THETA_HIGH "T1"

enum {
    COLOR_DRAB,
    COLOR_Z,
    COLOR_COUNTED,
    COLOR_INTENSITY,
    COLOR_LASER,
};
#define NUM_BUCKETS 400

struct velodyne_data
{
    double             m[16];
    lcmtypes_velodyne_t        *data;
    unsigned int       datalen;

    unsigned int       num_samples;
    velodyne_sample_t *samples;
    
    double             ctheta;
};

typedef struct _RendererVelodyne {
    Renderer renderer;

    Config *config;
    lcm_t *lc;

    velodyne_calib_t *calib;
    GUPtrCircular *circular;

    GtkuParamWidget * pw;

    double laser_colors[VELODYNE_NUM_LASERS][3];

    Viewer *viewer;
    CTrans *ctrans;
} RendererVelodyne;

static void free_velodyne_data(void *user, void *_p)
{
    struct velodyne_data *vdata = (struct velodyne_data*) _p;

    free(vdata->data);

    if (vdata->samples)
        free(vdata->samples);

    free(vdata);
}

static void my_free( Renderer *renderer )
{
/*
    RendererVelodyne *self = (RendererVelodyne*) renderer;

    if (self->current_data) {
        for (int i=0; i<self->current_data->len; i++) {
            lcmtypes_velodyne_t *vdata = g_ptr_array_index (self->current_data, i);
            lcmtypes_velodyne_t_destroy (vdata);
        }
        g_ptr_array_free (self->current_data, TRUE);
    }

    free( self );
*/
}

//static int lastsign;
//static int64_t rotation_utime;
//static int rotation_count = 0;
static int bad_ranges[64];
static int total_ranges[64];

int bad_samples[64];

static void my_draw( Viewer *viewer, Renderer *renderer )
{
    RendererVelodyne *self = (RendererVelodyne*) renderer->user;
    GtkuParamWidget *pw = GTKU_PARAM_WIDGET (self->pw);

    double total_theta = 0;
    double last_ctheta = HUGE;
    double pos[3];
    if (!ctrans_have_pose(self->ctrans))
      return;

    ctrans_local_pos(self->ctrans, pos);

    glEnable(GL_DEPTH_TEST);
    glPointSize(2.0);
    glBegin(GL_POINTS);

    int colormode = gtku_param_widget_get_enum(pw, COLOR_MENU);
    
    for (unsigned int cidx = 0; cidx < gu_ptr_circular_size(self->circular); cidx++) {
        struct velodyne_data *vdata = (struct velodyne_data*) gu_ptr_circular_index(self->circular, cidx);

        // do these returns still need to be projected
        if (vdata->samples == NULL) {
            vdata->samples = (velodyne_sample_t*) calloc(velodyne_decoder_estimate_samples(self->calib, vdata->data, vdata->datalen), 
                                                         sizeof(velodyne_sample_t));

            velodyne_decoder_t vdecoder;
            velodyne_decoder_init(self->calib, &vdecoder, vdata->data, vdata->datalen);
            
            while (!velodyne_decoder_next(self->calib, &vdecoder, &vdata->samples[vdata->num_samples])) {

                velodyne_sample_t *vsample = &vdata->samples[vdata->num_samples++];

                if (vsample->range < 0.25 && fabs(mod2pi(vsample->theta+M_PI/2)) < to_radians(20)) {
                    bad_samples[vsample->logical]++;
                } 

                total_ranges[vsample->logical]++;
                
                if (vsample->range < 0.01)
                    bad_ranges[vsample->logical]++;
                
                double sensor_xyz[4] = { vsample->xyz[0], vsample->xyz[1], vsample->xyz[2], 1 };
                double local_xyz[4];
                matrix_vector_multiply_4x4_4d(vdata->m, sensor_xyz, local_xyz);
                
                vsample->xyz[0] = local_xyz[0];
                vsample->xyz[1] = local_xyz[1];
		vsample->xyz[2] = local_xyz[2];

                vdata->ctheta = vsample->ctheta;
/*                
                int thissign = sgn(mod2pi(vsample.ctheta));
                if (thissign==1 && lastsign==-1) {
                    
                    rotation_count++;
                    int ROTATION_COUNT_THRESH = 5;
                    
                    if (rotation_count == ROTATION_COUNT_THRESH) {
                        double elapsed_time = (v->utime - rotation_utime)/1000000.0;
                        printf("velodyne %6s [%8d]: %5.3f Hz\n", 
                               vdecoder.version_string, vdecoder.revolution_count,
                               rotation_count / elapsed_time);
                        rotation_count = 0;
                        rotation_utime = v->utime;
                        
                        if (0) {
                            for (int i = 0; i < 64; i++)
                                printf("%3d : %6f\n", i, ((double)bad_ranges[i]*100.0)/total_ranges[i]);
                        }
                    }
                }
*/

            }
        }

        // have we plotted a whole revolution of data?
        if (last_ctheta == HUGE)
            last_ctheta = vdata->ctheta;

        total_theta += fabs(mod2pi(last_ctheta - vdata->ctheta));
        if (total_theta > 2*M_PI)
            break;
        last_ctheta = vdata->ctheta;

        // project these samples
        for (unsigned int s = 0; s < vdata->num_samples; s++) {
            velodyne_sample_t *vsample = &vdata->samples[s];
            
            switch (colormode) 
            {
            case COLOR_INTENSITY:
            {
                double v = vsample->intensity;
                glColor3fv(color_util_jet(v));
                break;
            }
            case COLOR_LASER:
                glColor3d(self->laser_colors[vsample->logical][0],
                          self->laser_colors[vsample->logical][1], 
                          self->laser_colors[vsample->logical][2]);
                break;
            case COLOR_DRAB:
                glColor3d(0.3, 0.3, 0.3);
                break;
            case COLOR_Z:
            {
                double z = vsample->xyz[2] - pos[2];
                double Z_MIN = -1, Z_MAX = 2;

                double z_norm = (z - Z_MIN) / (Z_MAX - Z_MIN);
                glColor3fv(color_util_jet(z_norm));
                break;
            }
            case COLOR_COUNTED:
            {
                switch (vsample->logical%10) {
                case 0:
                    glColor3d(1, 1, 0.5); break;
                case 5:
                    glColor3d(0.5, 1, 1); break;
                default:
                    glColor3d(0.3, 0.3, 0.3); break;
                }
            }
            break;
            }

            glVertex3dv(vsample->xyz);
        }
    }

    glEnd();
}

/*
static int on_velodyne_calib(const lcm_recv_buf_t *rbuf, const char *channel, const opaque_t *v, void *user_data)
{
    return 0;
}
*/


static void on_velodyne(const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_velodyne_t *v, void *user_data)
{
    RendererVelodyne *self = (RendererVelodyne*) user_data;

    if (!ctrans_have_pose(self->ctrans))
        return;

    struct velodyne_data *vdata = (struct velodyne_data*) calloc(1, sizeof(struct velodyne_data));
    vdata->data = (void*) malloc(v->datalen);
    vdata->datalen = v->datalen;
    memcpy(vdata->data, v->data, v->datalen);
    
    if (config_util_sensor_to_local(channel, vdata->m))
        printf("Velodyne: no calibration\n");

    gu_ptr_circular_add(self->circular, vdata);

    viewer_request_redraw( self->viewer );
}

static void on_param_changed( GtkuParamWidget *pw, const char *name, void *user_data )
{
    RendererVelodyne *self = (RendererVelodyne*) user_data;

    viewer_request_redraw( self->viewer );
}

static void on_load_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererVelodyne *self = user_data;
    gtku_param_widget_load_from_key_file (self->pw, keyfile, RENDERER_NAME);
}

static void on_save_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererVelodyne *self = user_data;
    gtku_param_widget_save_to_key_file (self->pw, keyfile, RENDERER_NAME);
}


Renderer* renderer_velodyne_new( Viewer *viewer)
{
    RendererVelodyne *self = 
        (RendererVelodyne*) calloc(1, sizeof(RendererVelodyne));

    Renderer *renderer = (Renderer*) self;

    // build color table
    for (int i = 0; i < VELODYNE_NUM_LASERS; i++) {
        float f[4];
        srand(i);
        color_util_rand_color(f, .6, .3);
        self->laser_colors[i][0] = f[0];
        self->laser_colors[i][1] = f[1];
        self->laser_colors[i][2] = f[2];
    }

    renderer->draw = my_draw;
    renderer->name = "Velodyne";
    renderer->destroy = my_free;
    renderer->widget = gtk_vbox_new ( FALSE, 0);
    renderer->enabled = 1;
    renderer->user = self;

    self->pw = GTKU_PARAM_WIDGET (gtku_param_widget_new ());
    self->circular = gu_ptr_circular_new(1000, free_velodyne_data, NULL);
    self->viewer = viewer;

    gtku_param_widget_add_enum( self->pw, COLOR_MENU, GTKU_PARAM_WIDGET_MENU,
                                COLOR_Z,
                                "Drab", COLOR_DRAB,
                                "Height", COLOR_Z,
                                "Counted", COLOR_COUNTED,
                                "Intensity", COLOR_INTENSITY,
                                "Laser", COLOR_LASER,
                                NULL);

    gtk_box_pack_start (GTK_BOX (renderer->widget), GTK_WIDGET (self->pw),
                        FALSE, TRUE, 0);
    gtk_widget_show (GTK_WIDGET (self->pw));

    self->lc = globals_get_lcm();
    self->ctrans = globals_get_ctrans();

    self->calib = velodyne_calib_create();

    lcmtypes_velodyne_t_subscribe(self->lc, "VELODYNE", on_velodyne, self);

    g_signal_connect( G_OBJECT(self->pw), "changed", G_CALLBACK(on_param_changed), self);

    g_signal_connect (G_OBJECT (viewer), "load-preferences", 
            G_CALLBACK (on_load_preferences), self);
    g_signal_connect (G_OBJECT (viewer), "save-preferences",
            G_CALLBACK (on_save_preferences), self);

    return (Renderer*) self;
}

void setup_renderer_velodyne(Viewer *viewer, int render_priority)
{
    viewer_add_renderer(viewer, renderer_velodyne_new(viewer), render_priority);
}
