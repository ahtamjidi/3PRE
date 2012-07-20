#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <gdk/gdkkeysyms.h>

#include <lcm/lcm.h>
#include <gtk_util/gtk_util.h>
#include <common/small_linalg.h>
#include <common/glib_util.h>
#include <common/fasttrig.h>
#include <common/geometry.h>
#include <common/math_util.h>
#include <common/timestamp.h>

#include <dgc/globals.h>

#include <lcmtypes/lcmtypes_goal_t.h>
#include <lcmtypes/lcmtypes_goal_list_t.h>

#include <libviewer/viewer.h>

#define TRANSMIT_GOALS_HZ 20

typedef struct _RendererGoal {
    Renderer     renderer;
    EventHandler ehandler;
     lcm_t         *lc;
    Viewer       *viewer;
    CTrans       *ctrans;

    double         last_xy[2];

    GArray        *my_goals;
    lcmtypes_goal_t        *edit_goal;
    lcmtypes_goal_t        *hover_goal;
    int            transmitter;

    GPtrArray     *recv_goal_lists;

    pthread_mutex_t mutex;
    int64_t        sender_id;
    int64_t        goal_id;

    int            empty_count;
} RendererGoal;

void activate_simgoals_transmitter(RendererGoal *self);

static void on_sim_goals_new(const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_goal_list_t *msg, void *_user)
{
    RendererGoal *self = (RendererGoal*) _user;

    pthread_mutex_lock(&self->mutex);

    g_array_set_size(self->my_goals,msg->num_goals);
    memcpy(self->my_goals->data,msg->goals,msg->num_goals*sizeof(lcmtypes_goal_t));
    activate_simgoals_transmitter(self);

    pthread_mutex_unlock(&self->mutex);


    viewer_request_redraw(self->viewer);
}


static void on_goals(const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_goal_list_t *msg, void *_user)
{
    RendererGoal *self = (RendererGoal*) _user;

    int i;
    for (i = 0; i < self->recv_goal_lists->len; i++) {
        lcmtypes_goal_list_t * l = g_ptr_array_index (self->recv_goal_lists, i);
        if (l->sender_id == msg->sender_id) {
            lcmtypes_goal_list_t_destroy (l);
            g_ptr_array_remove_index_fast (self->recv_goal_lists, i);
            break;
        }
    }

    g_ptr_array_add (self->recv_goal_lists, lcmtypes_goal_list_t_copy (msg));
    viewer_request_redraw(self->viewer);
}



gboolean on_transmit_goals(gpointer _user)
{
    RendererGoal *self = (RendererGoal*) _user;

    pthread_mutex_lock(&self->mutex);

    lcmtypes_goal_list_t glist;
    glist.num_goals = self->my_goals->len;
    glist.goals = (lcmtypes_goal_t *) self->my_goals->data;
    glist.utime = timestamp_now();
    glist.sender_id = self->sender_id;

    lcmtypes_goal_list_t_publish(self->lc, "GOALS", &glist);
    pthread_mutex_unlock(&self->mutex);

    if (glist.num_goals == 0)
        self->empty_count++;
    else
        self->empty_count = 0;

    /* If goal list is empty, stop transmitting shortly afterwards */
    if (self->empty_count > 20) {
        self->transmitter = 0;
        return FALSE;
    }

    return TRUE;
}

void
activate_simgoals_transmitter(RendererGoal *self)
{
    if (!self->transmitter)
        self->transmitter = g_timeout_add(1000 / TRANSMIT_GOALS_HZ,
                on_transmit_goals, self);
}


static lcmtypes_goal_t *find_goal(RendererGoal *self, const double xy[2])
{
    double best_distance = HUGE;
    lcmtypes_goal_t *best_goal = NULL;

    for (unsigned int i = 0; i < self->my_goals->len; i++) {
        lcmtypes_goal_t * goal = &g_array_index(self->my_goals, lcmtypes_goal_t, i);

        double dx = xy[0] - goal->pos[0];
        double dy = xy[1] - goal->pos[1];
        double distance = sqrt(sq(dx) + sq(dy));

        double tx = cos(-goal->theta)*dx - sin(-goal->theta)*dy;
        double ty = sin(-goal->theta)*dx + cos(-goal->theta)*dy;

        if (distance < best_distance && fabs(tx) < goal->size[0]/2 &&
                fabs(ty) < goal->size[1]/2) {
            best_distance = distance;
            best_goal = goal;
        }
    }

    return best_goal;
}

static void draw_goal (RendererGoal *self, lcmtypes_goal_t * goal)
{
    double carpos[3] = { 0, 0, 0 };
    ctrans_local_pos (self->ctrans, carpos);
    glPushMatrix();
    glTranslated(goal->pos[0], goal->pos[1], carpos[2]);
    glRotated(to_degrees(goal->theta), 0, 0, 1);
    
    glEnable(GL_BLEND);
    glBegin(GL_QUADS);
    glVertex2d( - goal->size[0]/2,  - goal->size[1]/2);
    glVertex2d( - goal->size[0]/2,  + goal->size[1]/2);
    glVertex2d( + goal->size[0]/2,  + goal->size[1]/2);
    glVertex2d( + goal->size[0]/2,  - goal->size[1]/2);
    glEnd();    
    
    if (goal->use_theta) {
        glBegin(GL_LINES);
        glColor3f(.8, 1, .8);
        glVertex2d( - goal->size[0]/2, 0);
        glVertex2d( + goal->size[0]/2, 0);
        glVertex2d( 0, goal->size[1]/2);
        glVertex2d( + goal->size[0]/2, 0);
        glVertex2d( 0, -goal->size[1]/2);
        glVertex2d( + goal->size[0]/2, 0);
        glEnd();
    } else {
        glBegin(GL_LINES);
        glColor3f(.8, 1, .8);
        glVertex2d( - goal->size[0]/2, - goal->size[1]/2);
        glVertex2d( goal->size[0]/2, goal->size[1]/2);
        glVertex2d( - goal->size[0]/2, goal->size[1]/2);
        glVertex2d( goal->size[0]/2, - goal->size[1]/2);
        glEnd();
    }

    glPopMatrix();
}

static void my_free( Renderer *renderer )
{
    RendererGoal *self = (RendererGoal*) renderer->user;

    free( self );
}

static void my_draw( Viewer *viewer, Renderer *renderer )
{
    RendererGoal *self = (RendererGoal*) renderer->user;

    for (int i = 0; i < self->recv_goal_lists->len; i++) {
        lcmtypes_goal_list_t * l = g_ptr_array_index (self->recv_goal_lists, i);
        for (int j = 0; j < l->num_goals; j++) {
            lcmtypes_goal_t * goal = l->goals + j;

            int edit_idx = -1;
            int hover_idx = -1;
            if (l->sender_id == self->sender_id && self->edit_goal)
                edit_idx = self->edit_goal - &g_array_index (self->my_goals,
                        lcmtypes_goal_t, 0);
            if (l->sender_id == self->sender_id && self->hover_goal)
                hover_idx = self->hover_goal - &g_array_index (self->my_goals,
                        lcmtypes_goal_t, 0);
            if (j == edit_idx)
                glColor4f(.5, 1, .5, .5);
            else if (self->ehandler.hovering && j == hover_idx)
                glColor4f(.5, .8, .5, .5);
            else
                glColor4f(0, .5, .5, 0.3);

            draw_goal (self, goal);
        }
    }
}

static int key_press (Viewer *viewer, EventHandler *ehandler,
        const GdkEventKey *event)
{
    RendererGoal *self = (RendererGoal*) ehandler->user;

    if (event->keyval == 'g' || event->keyval == 'G') {
        if (viewer_picking(viewer))
            return 0;
    
        viewer_request_pick(viewer, ehandler);
        if (!ehandler->picking)
            return 0;

        return 1;
    }

    if ((event->keyval == GDK_Delete || event->keyval == GDK_BackSpace) && ehandler->picking) {
        ehandler->picking = 0;
        int idx = self->edit_goal - &g_array_index (self->my_goals, lcmtypes_goal_t, 0);
        g_array_remove_index_fast (self->my_goals, idx);
        self->edit_goal = NULL;
        return 1;
    }
    
    if (event->keyval == GDK_Escape && ehandler->picking) {
        ehandler->picking = 0;
        self->edit_goal = NULL;
        return 1;
    }
    
    return 0;
}

static int mouse_press (Viewer *viewer, EventHandler *ehandler,
                        const double ray_start[3], const double ray_dir[3],
                        const GdkEventButton *event)
{
    RendererGoal *self = (RendererGoal*) ehandler->user;

    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D (ray_start), POINT3D (ray_dir), 
            0, POINT2D (xy));

    int consumed = 0;

    int control = event->state & GDK_CONTROL_MASK;

    pthread_mutex_lock (&self->mutex);

    // select a previously selected goal?
    if (event->button == 1 && ehandler->picking) {
        self->edit_goal = find_goal(self, xy);
        consumed = 1;
    } 

    if (event->button == 1 && control && ehandler->picking &&
            self->edit_goal == NULL) {
        // create a new goal
        lcmtypes_goal_t g = {
            .pos = { xy[0], xy[1] },
            .size = { 1, 1 },
            .theta = 0,
            .id = self->goal_id++,
            .use_theta = 0,
            .speed = 0,
            .speed_tol = 0,
            .min_speed = 0,
            .max_speed = 10,
            .wait_until_reachable = FALSE,
        };
        g_array_append_val (self->my_goals, g);
        activate_simgoals_transmitter(self);
        self->edit_goal = &g_array_index (self->my_goals, lcmtypes_goal_t,
                self->my_goals->len - 1);
        consumed = 1;
        viewer_request_pick(viewer, ehandler);
    }
    
    if (self->edit_goal && event->type == GDK_2BUTTON_PRESS) {
        self->edit_goal->use_theta ^= 1;
        if (self->edit_goal->use_theta)
            self->edit_goal->heading_tol=atan(self->edit_goal->size[1]/self->edit_goal->size[0])/2.0;
        self->edit_goal->id = self->goal_id++;
    }
    pthread_mutex_unlock (&self->mutex);

    memcpy(self->last_xy, xy, 2 * sizeof(double));
    return consumed;
}

static int mouse_motion (Viewer *viewer, EventHandler *ehandler,
                         const double ray_start[3], const double ray_dir[3], const GdkEventMotion *event)
{
    RendererGoal *self = (RendererGoal*) ehandler->user;
    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D (ray_start), POINT3D (ray_dir), 
            0, POINT2D (xy));
    int consumed = 0;
    int control = event->state & GDK_CONTROL_MASK;
    int shift = event->state & GDK_SHIFT_MASK;
 
    // only handle mouse button 1.
    if (!(event->state & GDK_BUTTON1_MASK))
        return 0;

    if (!self->edit_goal)
        return 0;

    pthread_mutex_lock(&self->mutex);

    if (control) {
        // resize
        double sx = xy[0] - self->edit_goal->pos[0];
        double sy = xy[1] - self->edit_goal->pos[1];
        
        double tx = cos(-self->edit_goal->theta)*sx - sin(-self->edit_goal->theta)*sy;
        double ty = sin(-self->edit_goal->theta)*sx + cos(-self->edit_goal->theta)*sy;
        
        self->edit_goal->size[0] = fabs(2*tx);
        self->edit_goal->size[1] = fabs(2*ty);
        if (self->edit_goal->use_theta)
            self->edit_goal->heading_tol=atan(self->edit_goal->size[1]/self->edit_goal->size[0])/2.0;

        self->edit_goal->id = self->goal_id++;
        consumed = 1;
    } else if (shift) {
        // rotate
        double dtheta = 
            atan2(xy[1] - self->edit_goal->pos[1],
                  xy[0] - self->edit_goal->pos[0]) - 
            atan2(self->last_xy[1] - self->edit_goal->pos[1],
                  self->last_xy[0] - self->edit_goal->pos[0]);

        memcpy(self->last_xy, xy, 2 * sizeof(double));        
        self->edit_goal->theta += dtheta;
        
        self->edit_goal->id = self->goal_id++;
        consumed = 1;
    } else {
        // translate
        memcpy(self->edit_goal->pos, xy, 2 * sizeof(double));
        self->edit_goal->id = self->goal_id++;
        consumed = 1;
    }

    pthread_mutex_unlock(&self->mutex);

    return consumed;
}

static int mouse_release (Viewer *viewer, EventHandler *ehandler,
                          const double ray_start[3], const double ray_dir[3],
                          const GdkEventButton *event)
{
    RendererGoal *self = (RendererGoal*) ehandler->user;
    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D (ray_start), POINT3D (ray_dir), 
            0, POINT2D (xy));

    ehandler->picking = 0;
    self->edit_goal = NULL;
    return 0;
}

static double pick_query(Viewer *viewer, EventHandler *ehandler,
        const double ray_start[3], const double ray_dir[3])
{
    RendererGoal *self = (RendererGoal*) ehandler->user;

    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D (ray_start), POINT3D (ray_dir), 
            0, POINT2D (xy));

    pthread_mutex_lock (&self->mutex);
    self->hover_goal = find_goal(self, xy);
    if (!self->hover_goal) {
        pthread_mutex_unlock (&self->mutex);
        return -1;
    }

    double dist = sqrt(sq(self->hover_goal->pos[0] - xy[0]) +
            sq(self->hover_goal->pos[1] - xy[1]));
    pthread_mutex_unlock (&self->mutex);

    return dist;
}

void setup_renderer_goal(Viewer *viewer, int priority) 
{
    RendererGoal *self = 
        (RendererGoal*) calloc(1, sizeof(RendererGoal));

    Renderer *renderer = &self->renderer;

    renderer->draw = my_draw;
    renderer->destroy = my_free;
    renderer->name = "Goal";
    renderer->widget = NULL;
    renderer->enabled = 1;
    renderer->user = self;

    self->ehandler.name = "Goal";
    self->ehandler.enabled = 1;
    self->ehandler.mouse_press = mouse_press;
    self->ehandler.mouse_motion = mouse_motion;
    self->ehandler.mouse_release = mouse_release;
    self->ehandler.pick_query = pick_query;
    self->ehandler.hover_query = pick_query;
    self->ehandler.key_press = key_press;
    self->ehandler.user = self;
    self->sender_id = timestamp_now() + rand();

    pthread_mutex_init(&self->mutex, NULL);

    self->lc = globals_get_lcm();
    self->ctrans = globals_get_ctrans ();
    self->viewer = viewer;

    self->my_goals = g_array_new (FALSE, FALSE, sizeof (lcmtypes_goal_t));
    self->recv_goal_lists = g_ptr_array_new ();

    lcmtypes_goal_list_t_subscribe(self->lc, "GOALS", on_goals, self);
    lcmtypes_goal_list_t_subscribe(self->lc, "SIM_GOALS_NEW", on_sim_goals_new, self);

    viewer_add_renderer(viewer, renderer, priority);
    viewer_add_event_handler(viewer, &self->ehandler, priority);

}
