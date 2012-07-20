#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <common/small_linalg.h>
#include <common/math_util.h>
#include <common/geometry.h>
#include <glutil/glutil.h>
#include <lcmtypes/lcmtypes_adu_status_t.h>
#include <lcmtypes/lcmtypes_car_summary_t.h>
#include <lcmtypes/lcmtypes_emc_feedback_t.h>
#include <lcmtypes/lcmtypes_controller_aux_t.h>
#include <lcmtypes/lcmtypes_vel_advice_t.h>
#ifdef USE_CANDECODE
#include <dgc/can_decode.h>
#endif
#include <dgc/globals.h>

#include <gtk_util/gtk_util.h>

#include "libviewer/viewer.h"
#include "dgcviewer.h"

#define PARAM_NAME_GRAPH_TIMESPAN "Time span"
#define PARAM_NAME_FREEZE "Freeze"
#define PARAM_NAME_SIZE "Size"
#define PARAM_NAME_RENDER_GAS "Gas"
#define PARAM_NAME_RENDER_STEER "Steer"
#define PARAM_NAME_RENDER_SPEED "Speed"
#define PARAM_NAME_SHOW_LEGEND "Show Legends"

#define REDRAW_THRESHOLD_UTIME 5000000

typedef struct _RendererScrollingPlots RendererScrollingPlots;

struct _RendererScrollingPlots {
    Renderer renderer;

    CTrans *ctrans;

    GtkuParamWidget    *pw;

    GLUtilSPlot2d *gas_plot;
    GLUtilSPlot2d *steer_plot;
    GLUtilSPlot2d *speed_plot;

    lcm_t *lc;

    uint64_t      max_utime;
#ifdef USE_CANDECODE
    CanDecode     *can_decode;
#endif
    Viewer *viewer;
};

static void on_adu_status (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_adu_status_t *msg, 
        void *user_data);
//static int on_odometry_update (const lcm_recv_buf_t *rbuf, const char *channel, 
//        const odometry_update_t *msg, void *user_data);

#ifdef USE_CANDECODE
static void on_pedal (const lcm_recv_buf_t *rbuf, const char *channel, const pedal_t *msg, void *user_data);
static void on_steering_wheel (const lcm_recv_buf_t *rbuf, const char *channel, 
        const steering_wheel_t *msg, void *user_data);
#endif

static void on_param_widget_changed (GtkuParamWidget *pw, const char *name, 
        RendererScrollingPlots *self);
static void on_emc_feedback (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_emc_feedback_t *msg, 
        void *user_data);
static void on_controller_aux (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_controller_aux_t *msg, 
        void *user_data);
static void on_vel_advice (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_vel_advice_t *msg, 
        void *user_data);
static void update_xaxis (RendererScrollingPlots *self, uint64_t utime);
static gboolean get_speed_update (void *user_data);

static void
scrolling_plots_draw (Viewer *viewer, Renderer *renderer)
{
    RendererScrollingPlots *self = (RendererScrollingPlots*) renderer->user;
    if (!self->max_utime) return;

    GLdouble model_matrix[16];
    GLdouble proj_matrix[16];
    GLint viewport[4];

    glGetDoublev (GL_MODELVIEW_MATRIX, model_matrix);
    glGetDoublev (GL_PROJECTION_MATRIX, proj_matrix);
    glGetIntegerv (GL_VIEWPORT, viewport);

    double gs_ts_max = self->max_utime * 1e-6;
    double gs_ts_min = gs_ts_max - 
        gtku_param_widget_get_double (self->pw, PARAM_NAME_GRAPH_TIMESPAN);

    glutil_splot2d_set_xlim (self->gas_plot, gs_ts_min, gs_ts_max);
    glutil_splot2d_set_xlim (self->steer_plot, gs_ts_min, gs_ts_max);
    
    glutil_splot2d_set_xlim (self->speed_plot, gs_ts_min, gs_ts_max);

    int plot_width = gtku_param_widget_get_int (self->pw, PARAM_NAME_SIZE);
    int plot_height = plot_width / 3;

    int x = viewport[2] - plot_width;
    int y = viewport[1];

    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_RENDER_GAS)) {
        glutil_splot2d_gl_render_at_window_pos (self->gas_plot, 
                x, y, plot_width, plot_height);
        y += plot_height;
    }

    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_RENDER_STEER)) {
        glutil_splot2d_gl_render_at_window_pos (self->steer_plot, 
                x, y, plot_width, plot_height);
        y += plot_height;
    }

    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_RENDER_SPEED)) {
        glutil_splot2d_gl_render_at_window_pos (self->speed_plot, 
                x, y, plot_width, plot_height);
        y += plot_height;
    }
}

static void
scrolling_plots_free (Renderer *renderer) 
{
    RendererScrollingPlots *self = (RendererScrollingPlots*) renderer;
    globals_release_ctrans (self->ctrans);
    globals_release_lcm (self->lc);
    free (renderer);
}

Renderer *renderer_scrolling_plots_new (Viewer *viewer)
{
    RendererScrollingPlots *self = 
        (RendererScrollingPlots*) calloc (1, sizeof (RendererScrollingPlots));
    self->viewer = viewer;
    self->renderer.draw = scrolling_plots_draw;
    self->renderer.destroy = scrolling_plots_free;
    self->renderer.name = "Scrolling Plots";
    self->renderer.user = self;
    self->renderer.enabled = 1;

    self->renderer.widget = gtk_alignment_new (0, 0.5, 1.0, 0);
    self->ctrans = globals_get_ctrans();

    self->lc = globals_get_lcm ();

    self->pw = GTKU_PARAM_WIDGET (gtku_param_widget_new ());
    gtk_container_add (GTK_CONTAINER (self->renderer.widget), 
            GTK_WIDGET(self->pw));
    gtk_widget_show (GTK_WIDGET (self->pw));

    gtku_param_widget_add_int (self->pw, PARAM_NAME_SIZE,
            GTKU_PARAM_WIDGET_SLIDER, 50, 800, 10, 150);
    gtku_param_widget_add_double (self->pw, PARAM_NAME_GRAPH_TIMESPAN, 
            GTKU_PARAM_WIDGET_SLIDER, 1, 20, 0.5, 5);
    gtku_param_widget_add_booleans (self->pw, 
            GTKU_PARAM_WIDGET_TOGGLE_BUTTON, PARAM_NAME_FREEZE, 0, NULL);
    gtku_param_widget_add_booleans (self->pw, 0,
            PARAM_NAME_RENDER_GAS, 1, 
            PARAM_NAME_RENDER_STEER, 1, 
            PARAM_NAME_RENDER_SPEED, 1, NULL);
    gtku_param_widget_add_booleans (self->pw, 0, 
            PARAM_NAME_SHOW_LEGEND, 0, NULL);

    g_signal_connect (G_OBJECT (self->pw), "changed", 
            G_CALLBACK (on_param_widget_changed), self);

    // gas plot
    self->gas_plot = glutil_splot2d_new ();
    glutil_splot2d_set_title        (self->gas_plot, "Gas mV");
    glutil_splot2d_set_text_color   (self->gas_plot, 0.7, 0.7, 0.7, 1);
    glutil_splot2d_set_bgcolor      (self->gas_plot, 0.1, 0.1, 0.1, 0.7);
    glutil_splot2d_set_border_color (self->gas_plot, 1, 1, 1, 0.7);
    glutil_splot2d_set_ylim    (self->gas_plot, 0, 5000);
    glutil_splot2d_add_plot    (self->gas_plot, "control", 1000);
    glutil_splot2d_set_color   (self->gas_plot, "control", 0, 0, 1, 1);
    glutil_splot2d_add_plot    (self->gas_plot, "emc", 1000);
    glutil_splot2d_set_color   (self->gas_plot, "emc", 0.5, 1, 0.5, 1);
    glutil_splot2d_add_plot    (self->gas_plot, "actual", 1000);
    glutil_splot2d_set_color   (self->gas_plot, "actual", 0.7, 0, 0.7, 1);
    glutil_splot2d_add_plot    (self->gas_plot, "2500", 1000);
    glutil_splot2d_set_color   (self->gas_plot, "2500", 0.8, 0.8, 0.8, 0.5);

    // steer plot
    self->steer_plot = glutil_splot2d_new ();
    glutil_splot2d_set_title        (self->steer_plot, "Steer mV");
    glutil_splot2d_set_text_color   (self->steer_plot, 0.7, 0.7, 0.7, 1);
    glutil_splot2d_set_border_color (self->steer_plot, 1, 1, 1, 0.7);
    glutil_splot2d_set_bgcolor (self->steer_plot, 0.1, 0.1, 0.1, 0.7);
    glutil_splot2d_set_ylim    (self->steer_plot, 0, 5000);
    glutil_splot2d_add_plot    (self->steer_plot, "control", 1000);
    glutil_splot2d_set_color   (self->steer_plot, "control", 0, 0, 1, 1);
    glutil_splot2d_add_plot    (self->steer_plot, "cmded", 1000);
    glutil_splot2d_set_color   (self->steer_plot, "cmded", 1, 1, 0, 1);
    glutil_splot2d_add_plot    (self->steer_plot, "emc", 1000);
    glutil_splot2d_set_color   (self->steer_plot, "emc", 0.5, 1, 0.5, 1);
    glutil_splot2d_add_plot    (self->steer_plot, "actual", 1000);
    glutil_splot2d_set_color   (self->steer_plot, "actual", 0.7, 0, 0.7, 1);
    glutil_splot2d_add_plot    (self->steer_plot, "2500", 1000);
    glutil_splot2d_set_color   (self->steer_plot, "2500", 0.8, 0.8, 0.8, 0.5);

    // speed plot
    self->speed_plot = glutil_splot2d_new ();
    glutil_splot2d_set_title        (self->speed_plot, "Speed");
    glutil_splot2d_set_text_color   (self->speed_plot, 0.7, 0.7, 0.7, 1);
    glutil_splot2d_set_border_color (self->speed_plot, 1, 1, 1, 0.7);
    glutil_splot2d_set_bgcolor (self->speed_plot, 0.1, 0.1, 0.1, 0.7);
    glutil_splot2d_set_ylim    (self->speed_plot, -2, 15);

    glutil_splot2d_add_plot    (self->speed_plot, "cmded", 1000);
    glutil_splot2d_set_color   (self->speed_plot, "cmded", 0, 0.5, 0, 0.5);
    glutil_splot2d_add_plot    (self->speed_plot, "actual", 1000);
    glutil_splot2d_set_color   (self->speed_plot, "actual", 0.7, 0, 0.7, 1);
    glutil_splot2d_add_plot    (self->speed_plot, "v_adv", 1000);
    glutil_splot2d_set_color   (self->speed_plot, "v_adv", 1, 0, 0, 1);
    glutil_splot2d_add_plot    (self->speed_plot, "v_cmd_va", 1000);
    glutil_splot2d_set_color   (self->speed_plot, "v_cmd_va", 0, 1, 0, 1);
/*
    glutil_splot2d_add_plot    (self->speed_plot, "v_adv_disc", 1000);
    glutil_splot2d_set_color   (self->speed_plot, "v_adv_disc", 0, 0, 1, 1);

    glutil_splot2d_add_plot    (self->speed_plot, "va_lane_conf", 1000);
    glutil_splot2d_set_color   (self->speed_plot, "va_lane_conf", 1, 0, 0.8, 1);
    glutil_splot2d_add_plot    (self->speed_plot, "va_a_vert", 1000);
    glutil_splot2d_set_color   (self->speed_plot, "va_a_vert", 1, 0, 0.6, 1);
    glutil_splot2d_add_plot    (self->speed_plot, "va_pitch", 1000);
    glutil_splot2d_set_color   (self->speed_plot, "va_pitch", 1, 0, 0.4, 1);
    glutil_splot2d_add_plot    (self->speed_plot, "va_roll", 1000);
    glutil_splot2d_set_color   (self->speed_plot, "va_roll", 1, 0, 0.2, 1);
*/
    glutil_splot2d_add_plot    (self->speed_plot, "10", 1000);
    glutil_splot2d_set_color   (self->speed_plot, "10", 0.8, 0.8, 0.8, 0.5);
    glutil_splot2d_add_plot    (self->speed_plot, "5", 1000);
    glutil_splot2d_set_color   (self->speed_plot, "5", 0.6, 0.6, 0.6, 0.5);
    glutil_splot2d_add_plot    (self->speed_plot, "0", 1000);
    glutil_splot2d_set_color   (self->speed_plot, "0", 0.8, 0.8, 0.8, 0.5);

    // legends?
    GLUtilSPlot2dLegendLocation legloc = GLUTIL_SPLOT2D_HIDDEN;
    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_SHOW_LEGEND)) {
        legloc = GLUTIL_SPLOT2D_TOP_RIGHT;
    }
    glutil_splot2d_set_show_legend (self->speed_plot, legloc);
    glutil_splot2d_set_show_legend (self->gas_plot, legloc);
    glutil_splot2d_set_show_legend (self->steer_plot, legloc);


    // subscribe to LC messages
    lcmtypes_adu_status_t_subscribe (self->lc, "ADU_STATUS", on_adu_status, self);
    lcmtypes_emc_feedback_t_subscribe (self->lc, "EMC_FEEDBACK", on_emc_feedback, 
            self);
    lcmtypes_controller_aux_t_subscribe (self->lc, "CONTROLLER_AUX", 
            on_controller_aux, self);
    lcmtypes_vel_advice_t_subscribe (self->lc, "VEL_ADVICE", on_vel_advice, self);
#ifdef USE_CANDECODE
    self->can_decode = can_decode_new (self->lc);
//    can_decode_subscribe (self->can_decode, CAN_MSG_ODOMETRY,
//                          (CanDecodeHandler) on_odometry_update, self);
    can_decode_subscribe (self->can_decode, CAN_MSG_STEERING_WHEEL,
                          (CanDecodeHandler) on_steering_wheel, self);
    can_decode_subscribe (self->can_decode, CAN_MSG_PEDAL_GAS, 
                          (CanDecodeHandler) on_pedal, self);
    can_decode_subscribe (self->can_decode, CAN_MSG_PEDAL_BRAKE, 
                          (CanDecodeHandler) on_pedal, self);
#endif

//    lcmtypes_adu_command_t_subscribe (self->lc, "ADU_COMMAND", on_lcmtypes_adu_command_t, self);

    // periodically pull pose data from CTrans
    g_timeout_add (30, get_speed_update, self);

    return &self->renderer;
}

void setup_renderer_scrolling_plots (Viewer *viewer, int render_priority)
{
    viewer_add_renderer(viewer, 
            renderer_scrolling_plots_new(viewer), render_priority);
}

static void
on_adu_status (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_adu_status_t *msg, void *user_data)
{
    RendererScrollingPlots *self = (RendererScrollingPlots*) user_data;

    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_FREEZE)) return;

    update_xaxis(self,msg->utime);

    double timestamp = msg->utime * 1e-6;

    glutil_splot2d_add_point (self->gas_plot, "control", timestamp, 
                              msg->gas_mv);
    glutil_splot2d_add_point (self->steer_plot, "control", timestamp, 
                              msg->steer_mv);
    glutil_splot2d_add_point (self->steer_plot, "cmded", timestamp,
                              msg->steer_goal_mv + 50);

    viewer_request_redraw (self->viewer);
    return;
}

static void
on_emc_feedback (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_emc_feedback_t *msg, 
        void *user_data)
{
    RendererScrollingPlots *self = (RendererScrollingPlots*) user_data;

    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_FREEZE)) return;

    update_xaxis(self,msg->utime);

    double timestamp = msg->utime * 1e-6;
    switch (msg->control) {
    case LCMTYPES_WHICH_CONTROL_T_GASBRAKE:
      glutil_splot2d_add_point (self->gas_plot, "emc", timestamp, 
            5000.0*msg->encoder_pos/1024);
      break;
    case LCMTYPES_WHICH_CONTROL_T_STEERING:
      glutil_splot2d_add_point (self->steer_plot, "emc", timestamp, 
            5000.0*msg->encoder_pos/1024);
      break;
    }
    viewer_request_redraw (self->viewer);
}

static void
on_controller_aux (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_controller_aux_t *msg, 
        void *user_data)
{
    RendererScrollingPlots  *self = (RendererScrollingPlots *) user_data;

    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_FREEZE)) {
        return;
    }

    update_xaxis (self, msg->utime);

    double  timestamp = msg->utime * 1.0e-6;

    glutil_splot2d_add_point (self->speed_plot, "cmded", timestamp, 
                                msg->v_cmd);
    glutil_splot2d_add_point (self->speed_plot, "v_cmd_va", timestamp, 
                                msg->v_cmd_va);

    viewer_request_redraw (self->viewer);
}

static void
on_vel_advice (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_vel_advice_t *msg, void *user_data)
{
    RendererScrollingPlots  *self = (RendererScrollingPlots *) user_data;

    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_FREEZE)) {
        return;
    }

    update_xaxis (self, msg->utime);

    double  timestamp = msg->utime * 1.0e-6;

    glutil_splot2d_add_point (self->speed_plot, "v_adv", timestamp, 
                                msg->vel_advice);
/*
    double  vel_advice_disc = (msg->vel_level + 1) * msg->vel_step;
    glutil_splot2d_add_point (self->speed_plot, "v_adv_disc", timestamp, 
                                vel_advice_disc);

    glutil_splot2d_add_point (self->speed_plot, "va_a_vert", timestamp, 
                                msg->va_a_vert);
    glutil_splot2d_add_point (self->speed_plot, "va_pitch", timestamp, 
                                msg->va_pitch);
    glutil_splot2d_add_point (self->speed_plot, "va_roll", timestamp, 
                                msg->va_roll);
    glutil_splot2d_add_point (self->speed_plot, "va_lane_conf", timestamp, 
                                msg->va_lane_conf);
*/
    viewer_request_redraw (self->viewer);
}

//static int
//on_odometry_update (const lcm_recv_buf_t *rbuf, const char *channel, const odometry_update_t *msg, 
//        void *user_data)
//{
//    RendererScrollingPlots *self = (RendererScrollingPlots*) user_data;
//
//    update_xaxis(self,msg->utime);
//    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_FREEZE)) return 0;
//
//    glutil_splot2d_add_point (self->speed_plot, "actual",
//                                msg->utime * 1.0e-6, msg->speed);
//    return 0;
//}

#ifdef USE_CANDECODE
static void
on_pedal (const lcm_recv_buf_t *rbuf, const char *channel, const pedal_t *msg, void *user_data)
{
    RendererScrollingPlots *self = (RendererScrollingPlots*) user_data;

    update_xaxis(self,msg->utime);
    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_FREEZE)) return;
    // doesn't matter in this case if it's gas or brake.
    if (msg->value>0.0) {
        float value = msg->value*(msg->which==PEDAL_GAS?1.0:-1.0);
        glutil_splot2d_add_point (self->gas_plot, "actual",
                                    msg->utime * 1.0e-6, 
                                    2500.0+(5*value*2500.0)); 
    }
}
    
static void
on_steering_wheel (const lcm_recv_buf_t *rbuf, const char *channel, const steering_wheel_t *msg, 
        void *user_data)
{
    RendererScrollingPlots *self = (RendererScrollingPlots*) user_data;

    update_xaxis(self,msg->utime);
    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_FREEZE)) return;
    glutil_splot2d_add_point (self->steer_plot, "actual", 
                                msg->utime * 1.0e-6,
                                2500.0-msg->wheel_angle*2500.0/(4*M_PI));   
}
#endif

static void 
on_param_widget_changed (GtkuParamWidget *pw, const char *name, 
        RendererScrollingPlots *self)
{
    if (! strcmp (name, PARAM_NAME_SHOW_LEGEND)) {
        GLUtilSPlot2dLegendLocation legloc = GLUTIL_SPLOT2D_HIDDEN;
        if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_SHOW_LEGEND)) {
            legloc = GLUTIL_SPLOT2D_TOP_RIGHT;
        }
        glutil_splot2d_set_show_legend (self->speed_plot, legloc);
        glutil_splot2d_set_show_legend (self->gas_plot, legloc);
        glutil_splot2d_set_show_legend (self->steer_plot, legloc);
    }
    viewer_request_redraw (self->viewer);
}

static void 
update_xaxis (RendererScrollingPlots *self, uint64_t utime)
{
    if ((utime < self->max_utime) && 
        (utime > self->max_utime - REDRAW_THRESHOLD_UTIME)) return;

    self->max_utime = utime;
    double timestamp = self->max_utime * 1e-6;
    glutil_splot2d_add_point (self->gas_plot, "2500", timestamp, 2500.0);
    glutil_splot2d_add_point (self->steer_plot, "2500", timestamp, 2500.0);
    glutil_splot2d_add_point (self->speed_plot, "10", timestamp, 10.0);
    glutil_splot2d_add_point (self->speed_plot, "5",  timestamp, 5.0);
    glutil_splot2d_add_point (self->speed_plot, "0", timestamp, 0.0);

}

static gboolean
get_speed_update (void *user_data)
{
    RendererScrollingPlots *self = (RendererScrollingPlots*) user_data;
    if (! ctrans_have_pose (self->ctrans)) return TRUE;
    if (gtku_param_widget_get_bool (self->pw, PARAM_NAME_FREEZE)) return 0;

    lcmtypes_pose_t pose;
    ctrans_local_pose (self->ctrans, &pose);

    double speed = vector_magnitude_2d (pose.vel);
    glutil_splot2d_add_point (self->speed_plot, "actual", 
            pose.utime * 1e-6, speed);
    return TRUE;
}
