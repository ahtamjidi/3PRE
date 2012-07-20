#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/mdf.h>
#include <lcm/lcm.h>
#include <dgc/globals.h>
#include <dgc/footprint.h>
#include <lcmtypes/lcmtypes_navigator_plan_t.h>
#include <glutil/glutil.h>

#include <libviewer/viewer.h>

typedef struct _RendererNavigator {
    Renderer renderer;

    Config *config;
    lcm_t *lc;
    RndfRouteNetwork * rndf;
    lcmtypes_navigator_plan_t *nav;

    Viewer *viewer;

    CTrans * ctrans;
} RendererNavigator;

static void my_free( Renderer *renderer )
{
    RendererNavigator *self = (RendererNavigator*) renderer->user;

    if( self->nav )
        lcmtypes_navigator_plan_t_destroy( self->nav );
        
    free( self );
}

static void my_draw( Viewer *viewer, Renderer *renderer )
{
    RendererNavigator *self = (RendererNavigator*) renderer->user;

    if (!self->nav) return;

    lcmtypes_pose_t pose;
    if (footprint_get_front_bumper_pose (self->ctrans, self->config, &pose) < 0)
        return;
    if (!ctrans_have_gps_to_local (self->ctrans))
        return;
    double z = pose.pos[2];

    glPushAttrib (GL_ENABLE_BIT);
    glEnable (GL_BLEND);
    glLineWidth (4.0);
    glBegin (GL_LINE_STRIP);
    glColor4f (0.85, 0.85, 0.0, 0.5);
    glVertex3f (pose.pos[0], pose.pos[1], z);
    int i;
    double xyz[3];
    double label_xyz[3];
    int found_checkpoint = 0;
    for (i = 0; i < self->nav->num_waypoints; i++) {
        lcmtypes_waypoint_id_t * wp = self->nav->waypoints + i;
        RndfWaypoint * w = rndf_find_waypoint_by_id (self->rndf,
                wp->segment, wp->lane, wp->point);
        if (!w) {
            fprintf (stderr, "Warning: waypoint %d.%d.%d not found, do you "
                    "have the correct RNDF in your config file?\n",
                    wp->segment, wp->lane, wp->point);
            continue;
        }
        double gps[3] = { w->lat, w->lon, 0 };
        ctrans_gps_to_local (self->ctrans, gps, xyz, NULL);
        glVertex3f (xyz[0], xyz[1], z);
        if (i == self->nav->checkpoint_index) {
            label_xyz[0] = xyz[0];
            label_xyz[1] = xyz[1];
            label_xyz[2] = z;
            found_checkpoint = 1;
        }
    }
    glEnd ();

    if (found_checkpoint) {
        char str[32];
        sprintf (str, "%d remaining", self->nav->checkpoints_remaining);
        glutil_draw_text (label_xyz, GLUT_BITMAP_HELVETICA_12, str, 0);
    }

    glPopAttrib ();
}

static void on_navigator_plan (const lcm_recv_buf_t *rbuf, const char *channel, 
        const lcmtypes_navigator_plan_t *msg, void *user_data )
{
    RendererNavigator *self = (RendererNavigator*) user_data;

    if (self->nav)
        lcmtypes_navigator_plan_t_destroy (self->nav);
    self->nav = lcmtypes_navigator_plan_t_copy(msg);

    viewer_request_redraw(self->viewer);
}

void setup_renderer_navigator(Viewer *viewer, int priority) 
{
    RendererNavigator *self = 
        (RendererNavigator*) calloc(1, sizeof(RendererNavigator));    

    Renderer *renderer = &self->renderer;

    renderer->draw = my_draw;
    renderer->destroy = my_free;
    renderer->name = "Navigator";
    renderer->enabled = 1;
    renderer->user = self;

    self->lc = globals_get_lcm();
    self->ctrans = globals_get_ctrans();
    self->rndf = globals_get_rndf ();
    self->config = globals_get_config ();
    self->viewer = viewer;
    self->nav = NULL;
    
    lcmtypes_navigator_plan_t_subscribe(self->lc, "NAVIGATOR_PLAN",
            on_navigator_plan, self);

    viewer_add_renderer(viewer, renderer, priority);
}
