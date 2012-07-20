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

#include <dgc/globals.h>
#include <dgc/config_util.h>

#include <lcmtypes/lcmtypes_rrt_plan_lsc_t.h>
#include <lcmtypes/lcmtypes_rrt_plan_t.h>
#include <lcmtypes/lcmtypes_shift_enum_t.h>

#include <libviewer/viewer.h>

typedef struct _RendererMotionPlan {
    Renderer renderer;

    Config *config;
    lcm_t *lc;
    lcmtypes_rrt_plan_t *rrt_plan;
    lcmtypes_rrt_plan_lsc_t *rrt_plan_lsc;

    Viewer *viewer;

    CTrans * ctrans;
} RendererMotionPlan;

static void
my_free (Renderer *renderer)
{
    RendererMotionPlan *self = (RendererMotionPlan *)renderer->user;

    if (self->rrt_plan)
        lcmtypes_rrt_plan_t_destroy (self->rrt_plan);
    if (self->rrt_plan_lsc)
        lcmtypes_rrt_plan_lsc_t_destroy (self->rrt_plan_lsc);

    free (self);
}

static void
my_draw (Viewer *viewer, Renderer *renderer)
{
    RendererMotionPlan *self = (RendererMotionPlan *)renderer->user;

    if (!self->rrt_plan && !self->rrt_plan_lsc)
        return;

    double p_local[3], z;
    ctrans_local_pos (self->ctrans, p_local);
    z = p_local[2] - 0.05;

    // Controller scpp
    if (self->rrt_plan) {
        // Line
        glPointSize (1.0);
        glLineWidth (2.0);
        // change color based on the direction
        if (self->rrt_plan->is_forward) {
            glColor3f (1.0, 0.7, 0.0);
        } else {
            glColor3f (0.7, 1.0, 0.0);
        }
        glBegin (GL_LINE_STRIP);
        for (int i = 0; i < self->rrt_plan->n; i++) {
            glVertex3f (self->rrt_plan->steps[i].L1_pt[0], 
                        self->rrt_plan->steps[i].L1_pt[1], z);
        }
        glEnd();

        // Points
        glPointSize (2.0);
        glBegin (GL_POINTS);
        for (int i = 0; i < self->rrt_plan->n; i++) {
            glVertex3f (self->rrt_plan->steps[i].L1_pt[0], 
                        self->rrt_plan->steps[i].L1_pt[1], z);
        }
        glEnd();

        // Line
        glLineWidth (1.0);
        if (self->rrt_plan->is_forward) {
            glColor3f (0.5, 0.8, 0.0);
        } else {
            glColor3f (0.8, 0.5, 0.0);
        }
        glBegin (GL_LINE_STRIP);
        for (int i = 0; i < self->rrt_plan->n; i++) {
            glVertex3f (self->rrt_plan->steps[i].pos[0], 
                        self->rrt_plan->steps[i].pos[1], z);
        }
        glEnd();

        // Points
        glPointSize (2.0);
        glBegin (GL_POINTS);
        for (int i = 0; i < self->rrt_plan->n; i++) {
            glVertex3f (self->rrt_plan->steps[i].pos[0], 
                        self->rrt_plan->steps[i].pos[1], z);
        }
        glEnd();

        // Stopping point
        int i_last = self->rrt_plan->n_stopped - 1;
        if (i_last >= 0) {
            glPointSize (8.0);
            glBegin (GL_POINTS);
            glVertex3f (self->rrt_plan->steps[i_last].pos[0], 
                        self->rrt_plan->steps[i_last].pos[1], z);
            glEnd();
        }
    }

    // Controller sclsc
    if (self->rrt_plan_lsc) {
        // Line
        glPointSize (1.0);
        glLineWidth (2.0);
        // change color based on the direction
        if (self->rrt_plan_lsc->is_forward) {
            glColor3f (1.0, 0.7, 0.0);
        } else {
            glColor3f (0.7, 1.0, 0.0);
        }
        glBegin (GL_LINE_STRIP);
        for (int i = 0; i < self->rrt_plan_lsc->n; i++) {
            glVertex3f (self->rrt_plan_lsc->steps[i].pos[0], 
                        self->rrt_plan_lsc->steps[i].pos[1], z);
        }
        glEnd();

        // Points
        glPointSize (2.0);
        glBegin (GL_POINTS);
        for (int i = 0; i < self->rrt_plan_lsc->n; i++) {
            glVertex3f (self->rrt_plan_lsc->steps[i].pos[0], 
                        self->rrt_plan_lsc->steps[i].pos[1], z);
        }
        glEnd();

        // Stopping point
        int i_last = self->rrt_plan_lsc->n - 1;
        if (i_last >= 0) {
            glPointSize (8.0);
            glBegin (GL_POINTS);
            glVertex3f (self->rrt_plan_lsc->steps[i_last].pos[0], 
                        self->rrt_plan_lsc->steps[i_last].pos[1], z);
            glEnd();
        }
    }
}

static void
on_rrt_plan (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_rrt_plan_t *msg, void *user_data)
{
    RendererMotionPlan *self = (RendererMotionPlan *)user_data;

    if (!ctrans_have_pose (self->ctrans))
        return;

    if (self->rrt_plan)
        lcmtypes_rrt_plan_t_destroy (self->rrt_plan);
    self->rrt_plan  = lcmtypes_rrt_plan_t_copy (msg);

    if (self->rrt_plan->n >= 2)
        viewer_request_redraw (self->viewer);
}

static void
on_rrt_plan_lsc (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_rrt_plan_lsc_t *msg, 
                 void *user_data)
{
    RendererMotionPlan *self = (RendererMotionPlan *)user_data;

    if (!ctrans_have_pose (self->ctrans))
        return;

    if (self->rrt_plan_lsc)
        lcmtypes_rrt_plan_lsc_t_destroy (self->rrt_plan_lsc);
    self->rrt_plan_lsc  = lcmtypes_rrt_plan_lsc_t_copy (msg);

    if (self->rrt_plan_lsc->n >= 2)
        viewer_request_redraw (self->viewer);
}

void
setup_renderer_motion_plan (Viewer *viewer, int priority)
{
    RendererMotionPlan *self = (RendererMotionPlan *)calloc (1, 
                                                sizeof (RendererMotionPlan));

    Renderer *renderer = &self->renderer;

    renderer->draw = my_draw;
    renderer->destroy = my_free;
    renderer->name = "Motion Plan";
    renderer->enabled = 1;
    renderer->user = self;

    self->lc = globals_get_lcm();
    self->ctrans = globals_get_ctrans();
    self->viewer = viewer;
    self->rrt_plan = NULL;

    lcmtypes_rrt_plan_t_subscribe (self->lc, "RRT_PLAN", on_rrt_plan, self);
    lcmtypes_rrt_plan_lsc_t_subscribe (self->lc, "RRT_PLAN_LSC", on_rrt_plan_lsc, 
                                    self);

    viewer_add_renderer (viewer, renderer, priority);
}

