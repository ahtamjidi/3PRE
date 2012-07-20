#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <common/small_linalg.h>
#include <common/math_util.h>
#include <common/camtrans.h>
#include <common/rotations.h>
#include <common/small_linalg.h>
#include <common/geometry.h>
#include <common/config.h>
#include <common/glib_util.h>
#include <glutil/glutil.h>

#include <dgc/config_util.h>
#include <dgc/globals.h>

#include <gtk_util/gtk_util.h>

#include "libviewer/viewer.h"
#include "dgcviewer.h"
//#include "gl_util.h"

#define err(...) fprintf (stderr, __VA_ARGS__)

typedef struct _RendererSensorPlacement RendererSensorPlacement;

typedef struct _RCamera
{
    CamTrans *camtrans;
    char *name;
} RCamera;

#if 0
typedef struct _RLidar 
{
    char *name;
    ranged_transform_t *rtrans;
} RLidar;

typedef struct _RRadar
{
    char *name;
    ranged_transform_t *rtrans;
} RRadar;
#endif

struct _RendererSensorPlacement {
    Renderer renderer;

    CTrans *ctrans;

    Config *config;
    GList *camera_renderers;

    GtkuParamWidget    *pw;

    Viewer *viewer;
};

static inline void 
char_replace(char *str, char old, char new) {
    for (char *c=str; c<str+strlen(str);c++) if (*c==old) *c=new;
}

#if 0
static inline void
add_lidar_renderer (RendererSensorPlacement *self, 
        const char *cfg_prefix, const char *name)
{
    RLidar *rlidar = (RLidar*) calloc (1, sizeof (RLidar));
    rlidar->rtrans = ranged_transform_create (self->config, cfg_prefix);
    if (!rlidar->rtrans) {
        err ("Couldn't load renderer for %s\n", name);
        free (rlidar);
        return;
    }
	rlidar->name = strdup(name);
	char_replace(rlidar->name,'_',' ');
	gtku_param_widget_add_booleans (self->pw, 0, rlidar->name, 0, NULL);

    self->lidar_renderers = g_list_append (self->lidar_renderers, rlidar);
}
#endif

static inline void
add_camera_renderer (RendererSensorPlacement *self, const char *cfg_prefix, 
        const char *name)
{
    RCamera *rcam = (RCamera*) calloc (1, sizeof (RCamera));
    rcam->camtrans = config_util_get_new_camtrans (self->config, name);
    if (!rcam->camtrans) {
        err ("Couldn't load renderer for %s\n", name);
        free (rcam);
        return;
    }

	rcam->name = strdup(name);
	char_replace(rcam->name,'_',' ');
	gtku_param_widget_add_booleans (self->pw, 0, rcam->name, 0, NULL);

    self->camera_renderers = g_list_append (self->camera_renderers, rcam);
}

#if 0
static inline void
add_radar_renderer (RendererSensorPlacement *self, const char *cfg_prefix,
        const char *name)
{
    RRadar *rradar = (RRadar*) calloc (1, sizeof (RRadar));
    rradar->rtrans = ranged_transform_create (self->config, cfg_prefix);
    if (!rradar->rtrans) {
        err ("Couldn't load renderer for %s\n", name);
        free (rradar);
        return;
    }

	rradar->name = strdup(name);
	char_replace(rradar->name,'_',' ');
	gtku_param_widget_add_booleans (self->pw, 0, rradar->name, 0, NULL);

    self->radar_renderers = g_list_append (self->radar_renderers, rradar);
}
#endif

static void
sensor_fov_draw_cameras (RendererSensorPlacement *self)
{
    double pos[3];
    ctrans_local_pos (self->ctrans, pos);

    double calib_to_local[16], calib_to_local_rot[16];
    ctrans_calibration_to_local_matrix (self->ctrans, calib_to_local);
    matrix_rigid_body_transform_get_rotation_matrix_4x4d (calib_to_local,
            calib_to_local_rot);

    for (GList *citer=self->camera_renderers; citer; citer=citer->next) {
        RCamera *rcam = (RCamera*) citer->data;
		if (!gtku_param_widget_get_bool (self->pw, rcam->name))
		  continue;
		CamTrans *camtrans = rcam->camtrans;

		double width = camtrans_get_image_width (camtrans);
		double height = camtrans_get_image_height (camtrans);
		
		double campos_calib[3];
		camtrans_get_position (camtrans, campos_calib);
		double campos[3];
        matrix_vector_multiply_4x4_3d (calib_to_local, campos_calib, campos);
		
		double tl_ray_calib[3];
		double tr_ray_calib[3];
		double bl_ray_calib[3];
		double br_ray_calib[3];
		camtrans_pixel_to_ray (camtrans, 0, 0, tl_ray_calib);
		camtrans_pixel_to_ray (camtrans, width, 0, tr_ray_calib);
		camtrans_pixel_to_ray (camtrans, 0, height, bl_ray_calib);
		camtrans_pixel_to_ray (camtrans, width, height, br_ray_calib);
		vector_normalize_3d (tl_ray_calib);
		vector_normalize_3d (tr_ray_calib);
		vector_normalize_3d (bl_ray_calib);
		vector_normalize_3d (br_ray_calib);
		
		double tl_ray_local[3];
		double tr_ray_local[3];
		double bl_ray_local[3];
		double br_ray_local[3];
        matrix_vector_multiply_4x4_3d (calib_to_local_rot, tl_ray_calib,
                tl_ray_local);
		matrix_vector_multiply_4x4_3d (calib_to_local_rot, tr_ray_calib,
									tr_ray_local);
		matrix_vector_multiply_4x4_3d (calib_to_local_rot, bl_ray_calib,
									bl_ray_local);
		matrix_vector_multiply_4x4_3d (calib_to_local_rot, br_ray_calib,
									br_ray_local);
		
		double tl_pt_local[3];
		double tr_pt_local[3];
		double bl_pt_local[3];
		double br_pt_local[3];
		vector_add_3d (campos, tl_ray_local, tl_pt_local);
		vector_add_3d (campos, tr_ray_local, tr_pt_local);
		vector_add_3d (campos, bl_ray_local, bl_pt_local);
		vector_add_3d (campos, br_ray_local, br_pt_local);
		
		// draw translucent triangles
		glColor4f (0.3, 0.3, 1, 0.3);
		glBegin (GL_TRIANGLES);
		glVertex3f (campos[0], campos[1], campos[2]);
		glVertex3f (tl_pt_local[0], tl_pt_local[1], tl_pt_local[2]);
		glVertex3f (bl_pt_local[0], bl_pt_local[1], bl_pt_local[2]);
		
		glVertex3f (campos[0], campos[1], campos[2]);
		glVertex3f (tl_pt_local[0], tl_pt_local[1], tl_pt_local[2]);
		glVertex3f (tr_pt_local[0], tr_pt_local[1], tr_pt_local[2]);
		
		glVertex3f (campos[0], campos[1], campos[2]);
		glVertex3f (bl_pt_local[0], bl_pt_local[1], bl_pt_local[2]);
		glVertex3f (br_pt_local[0], br_pt_local[1], br_pt_local[2]);
		
		glVertex3f (campos[0], campos[1], campos[2]);
		glVertex3f (br_pt_local[0], br_pt_local[1], br_pt_local[2]);
		glVertex3f (tr_pt_local[0], tr_pt_local[1], tr_pt_local[2]);
		glEnd ();
		  
		// draw solid outline
		glColor3f (0, 0, 1);
		glBegin (GL_LINE_LOOP);
		glVertex3f (campos[0], campos[1], campos[2]);
		glVertex3f (tl_pt_local[0], tl_pt_local[1], tl_pt_local[2]);
		glVertex3f (bl_pt_local[0], bl_pt_local[1], bl_pt_local[2]);
		glEnd ();
		
		glBegin (GL_LINE_LOOP);
		glVertex3f (campos[0], campos[1], campos[2]);
		glVertex3f (tl_pt_local[0], tl_pt_local[1], tl_pt_local[2]);
		glVertex3f (tr_pt_local[0], tr_pt_local[1], tr_pt_local[2]);
		glEnd ();
		
		glBegin (GL_LINE_LOOP);
		glVertex3f (campos[0], campos[1], campos[2]);
		glVertex3f (bl_pt_local[0], bl_pt_local[1], bl_pt_local[2]);
		glVertex3f (br_pt_local[0], br_pt_local[1], br_pt_local[2]);
		glEnd ();
		
		glBegin (GL_LINE_LOOP);
		glVertex3f (campos[0], campos[1], campos[2]);
		glVertex3f (br_pt_local[0], br_pt_local[1], br_pt_local[2]);
		glVertex3f (tr_pt_local[0], tr_pt_local[1], tr_pt_local[2]);
		glEnd ();
		
		// draw approximate fov coverage
    }
}

static void
sensor_placement_draw (Viewer *viewer, Renderer *renderer)
{
    RendererSensorPlacement *self = (RendererSensorPlacement*) renderer->user;

    if (!ctrans_have_pose (self->ctrans)) return;

//    glEnable (GL_DEPTH_TEST);
    glDisable (GL_DEPTH_TEST);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if 0
    sensor_fov_draw_lidars (self);
    sensor_fov_draw_radars (self);
#endif
    sensor_fov_draw_cameras (self);

    glDisable (GL_BLEND);
}

static void
sensor_placement_free (Renderer *renderer) 
{
    RendererSensorPlacement *self = (RendererSensorPlacement*) renderer;
    for (GList *citer=self->camera_renderers; citer; citer=citer->next) {
        RCamera *rcam = (RCamera*) citer->data;
        camtrans_destroy (rcam->camtrans);
        free (rcam);
    }
    g_list_free (self->camera_renderers);

#if 0
    for (GList *liter=self->lidar_renderers; liter; liter=liter->next) {
        RLidar *rlidar = (RLidar*) liter->data;
        ranged_transform_destroy (rlidar->rtrans);
        free (rlidar);
    }
    g_list_free (self->lidar_renderers);

    for (GList *riter=self->radar_renderers; riter; riter=riter->next) {
        RRadar *rradar = (RRadar*) riter->data;
        ranged_transform_destroy (rradar->rtrans);
        free (rradar);
    }
    g_list_free (self->radar_renderers);
#endif

    globals_release_ctrans (self->ctrans);
    globals_release_config (self->config);

    free (renderer);
}

static void 
on_param_widget_changed (GtkuParamWidget *pw, const char *param,
        void *user_data)
{
    RendererSensorPlacement *self = (RendererSensorPlacement*) user_data;

    viewer_request_redraw (self->viewer);
}


Renderer *renderer_sensor_placement_new (Viewer *viewer)
{
    RendererSensorPlacement *self = 
        (RendererSensorPlacement*) calloc (1, sizeof (RendererSensorPlacement));
    self->viewer = viewer;
    self->renderer.draw = sensor_placement_draw;
    self->renderer.destroy = sensor_placement_free;
    self->renderer.name = "Sensor Placement";
    self->renderer.user = self;
    self->renderer.enabled = 1;

    self->ctrans = globals_get_ctrans ();
    self->config = globals_get_config ();

    self->renderer.widget = gtku_param_widget_new ();
    self->pw = (GtkuParamWidget*) self->renderer.widget;

#if 0
    // enumerate and add all lidars
    char **lidar_names = config_util_get_all_lidar_names (self->config);
    for (int i=0; lidar_names && lidar_names[i]; i++) {
        char calib_prefix[256];
        config_util_get_lidar_calibration_config_prefix (self->config, 
                lidar_names[i], calib_prefix, sizeof (calib_prefix));
        if (config_get_num_subkeys (self->config, calib_prefix) > 0) {
            add_lidar_renderer (self, calib_prefix, lidar_names[i]);
        }

    }
    g_strfreev (lidar_names);
#endif

    // enumerate and add all cameras
    char **cam_names = config_util_get_all_camera_names (self->config);
    for (int i=0; cam_names && cam_names[i]; i++) {
        char calib_prefix[256];
        config_util_get_camera_calibration_config_prefix (self->config, 
                cam_names[i], calib_prefix, sizeof (calib_prefix));
        if (config_get_num_subkeys (self->config, calib_prefix) > 0) {
            add_camera_renderer (self, calib_prefix, cam_names[i]);
        }
    }
    g_strfreev (cam_names);

#if 0
    // enumerate and add all radars
    char **radar_names = config_util_get_all_radar_names (self->config);
    for (int i=0; radar_names && radar_names[i]; i++) {
        char calib_prefix[256];
        config_util_get_radar_calibration_config_prefix (self->config, 
                radar_names[i], calib_prefix, sizeof (calib_prefix));
        if (config_get_num_subkeys (self->config, calib_prefix) > 0) {
            add_radar_renderer (self, calib_prefix, radar_names[i]);
        }
    }
    g_strfreev (radar_names);
#endif

    g_signal_connect (G_OBJECT (self->pw), "changed",
            G_CALLBACK (on_param_widget_changed), self);

    return &self->renderer;
}

void setup_renderer_sensor_placement(Viewer *viewer, int render_priority)
{
    viewer_add_renderer(viewer, renderer_sensor_placement_new(viewer), 
            render_priority);
}
