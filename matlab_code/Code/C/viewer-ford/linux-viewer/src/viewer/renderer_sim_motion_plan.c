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
#include <common/config.h>
#include <common/fasttrig.h>
#include <common/geometry.h>
#include <common/math_util.h>
#include <common/timestamp.h>

#include <dgc/globals.h>
#include <dgc/config_util.h>

#include <lcmtypes/lcmtypes_motion_plan_t.h>
#include <lcmtypes/lcmtypes_motion_plan_action_t.h>

#include <libviewer/viewer.h>

#define PARAM_SPEED "Speed (m/s)"
struct point
{
    double xy[2];
    double speed;
    int dir;
};

typedef struct _RendererSimMP {
    Renderer     renderer;
    EventHandler ehandler;
    Config       *config;
    lcm_t         *lc;
    Viewer       *viewer;
    GtkuParamWidget *pw;

    CTrans        *ctrans;
    double         last_xy[2];

    GPtrArray      *points;

    struct point   *edit_point;
    struct point   *hover_point;

    pthread_mutex_t mutex;
    int64_t        sent_utime;

} RendererSimMP;


void
transmit_sim_mp(gpointer _user)
{
    RendererSimMP *self = (RendererSimMP*) _user;

    pthread_mutex_lock(&self->mutex);

    lcmtypes_motion_plan_t mp;
    mp.n = g_ptr_array_size(self->points);
    mp.actions = (lcmtypes_motion_plan_action_t *)malloc( mp.n * sizeof(lcmtypes_motion_plan_action_t));
    mp.utime = timestamp_now();

    for (unsigned int i = 0; i < g_ptr_array_size(self->points); i++) {
        struct point *pnt = g_ptr_array_index(self->points, i);
        mp.actions[i].x = pnt->xy[0];
        mp.actions[i].y = pnt->xy[1];
        mp.actions[i].speed  = pnt->speed;
        mp.actions[i].dir = pnt->dir;
    }
    pthread_mutex_unlock(&self->mutex);

    lcmtypes_motion_plan_t_publish(self->lc,"MOTION_PLAN", &mp);
    free(mp.actions);
}

static void on_clear_button(GtkWidget *button, RendererSimMP *self)
{
    pthread_mutex_lock(&self->mutex);
    g_ptr_array_free(self->points,TRUE);
    self->points = g_ptr_array_new();    
    pthread_mutex_unlock(&self->mutex);
    viewer_request_redraw(self->viewer);
}

#if 0
static struct point *find_point(RendererSimMP *self, const double xy[2])
{
    double best_distance = HUGE;
    struct point *best_point = NULL;

    for (unsigned int i = 0; i < g_ptr_array_size(self->points); i++) {
        struct point *point = g_ptr_array_index(self->points, i);

        double dx = xy[0] - point->xy[0];
        double dy = xy[1] - point->xy[1];
        double distance = sqrt(sq(dx) + sq(dy));

        if (distance < best_distance) {
            best_distance = distance;
            best_point = point;
        }
    }
    return best_point;
}
#endif

static void my_free( Renderer *renderer )
{
    RendererSimMP *self = (RendererSimMP*) renderer->user;

    free( self );
}




static void my_draw( Viewer *viewer, Renderer *renderer )
{
    RendererSimMP *self = (RendererSimMP*) renderer->user;

    glLineWidth(1.0);
    glColor3f(1, 1, 0);
    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 0; i < g_ptr_array_size(self->points); i++) {
            struct point *p = (struct point*) g_ptr_array_index(self->points, i);
            glVertex3f(p->xy[0], p->xy[1], 0);
       }
    glEnd();
    glBegin(GL_POINTS);
    for (unsigned int i = 0; i < g_ptr_array_size(self->points); i++) {
            struct point *p = (struct point*) g_ptr_array_index(self->points, i);
            glVertex3f(p->xy[0], p->xy[1], 0);
       }
    glEnd();
}


static int key_press (Viewer *viewer, EventHandler *ehandler, const GdkEventKey *event)
{
    RendererSimMP *self = (RendererSimMP*) ehandler->user;

    if (event->keyval == 'm' || event->keyval == 'M') {
        if (viewer_picking(viewer))
            return 0;
    
        viewer_request_pick(viewer, ehandler);
        if (!ehandler->picking)
            return 0;

        return 1;
    }

    if ((event->keyval == GDK_Delete || event->keyval == GDK_BackSpace) && ehandler->picking) {
        //ehandler->picking = 0;
        g_ptr_array_remove(self->points, self->edit_point);
        free(self->edit_point);
        self->edit_point = NULL;
        return 1;
    }
    
    if (event->keyval == GDK_Escape && ehandler->picking) {
        ehandler->picking = 0;
        self->edit_point = NULL;
        return 1;
    }
    
    return 0;
}

static int mouse_press (Viewer *viewer, EventHandler *ehandler,
                        const double ray_start[3], const double ray_dir[3], const GdkEventButton *event)
{
    RendererSimMP *self = (RendererSimMP*) ehandler->user;

    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D (ray_start), POINT3D (ray_dir), 
            0, POINT2D (xy));

    int consumed = 0;


    // adding point
    if (event->button == 1 && ehandler->picking) {
        struct point *p = (struct point*) calloc(1, sizeof (struct point));
        memcpy(p->xy, xy, 2 * sizeof(double));
        double speed = gtku_param_widget_get_double(self->pw, PARAM_SPEED);
        if (speed<0) {
            p->dir=LCMTYPES_SHIFT_ENUM_T_REVERSE;
            p->speed=-speed;
        }
        else {
            p->dir=LCMTYPES_SHIFT_ENUM_T_DRIVE;
            p->speed=speed;
        }
        g_ptr_array_add(self->points, p);
        consumed = 1;
    }

    if (event->button == 2 && ehandler->picking) {
        transmit_sim_mp(self);
        consumed = 1;
    }

    memcpy(self->last_xy, xy, 2 * sizeof(double));

    return consumed;
}

static int mouse_motion (Viewer *viewer, EventHandler *ehandler,
                         const double ray_start[3], const double ray_dir[3], const GdkEventMotion *event)
{
    RendererSimMP *self = (RendererSimMP*) ehandler->user;
    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D (ray_start), POINT3D (ray_dir), 
            0, POINT2D (xy));
    int consumed = 0;
 
    // only handle mouse button 1.
    if (!(event->state & GDK_BUTTON1_MASK))
        return 0;

    if (!self->edit_point)
        return 0;

    pthread_mutex_lock(&self->mutex);

    // translate
    memcpy(self->edit_point->xy, xy, 2 * sizeof(double));
    consumed = 1;

    pthread_mutex_unlock(&self->mutex);

    viewer_request_redraw(viewer);
    return consumed;
}

static int mouse_release (Viewer *viewer, EventHandler *ehandler,
                          const double ray_start[3], const double ray_dir[3], const GdkEventButton *event)
{
    RendererSimMP *self = (RendererSimMP*) ehandler->user;
    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D (ray_start), POINT3D (ray_dir), 
            0, POINT2D (xy));

    //ehandler->picking = 0;
    self->edit_point = NULL;
    return 0;
}

static double pick_query(Viewer *viewer, EventHandler *ehandler, const double ray_start[3], const double ray_dir[3])
{/*
    RendererSimMP *self = (RendererSimMP*) ehandler->user;

    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D (ray_start), POINT3D (ray_dir), 
            0, POINT2D (xy), NULL);

    self->hover_point = find_point(self, xy);
    if (!self->hover_point)
        return -1;

    double dist = sqrt(sq(self->hover_point->xy[0] - xy[0]) + sq(self->hover_point->xy[1] - xy[1]));

    return dist;
 */
    return -1;
}

static void on_param_widget_changed (GtkuParamWidget *pw, const char *name, void *user)
{
//    RendererSimMP *self = (RendererSimMP*) user;
}

void setup_renderer_sim_motion_plan(Viewer *viewer, int priority) 
{
    RendererSimMP *self = 
        (RendererSimMP*) calloc(1, sizeof(RendererSimMP));

    Renderer *renderer = &self->renderer;

    self->pw = GTKU_PARAM_WIDGET(gtku_param_widget_new());

    renderer->draw = my_draw;
    renderer->destroy = my_free;
    renderer->name = "Sim Motion Planner";
    renderer->widget = gtk_vbox_new(FALSE, 0);
    //renderer->widget = GTK_WIDGET(self->pw);
    renderer->enabled = 1;
    renderer->user = self;

    self->ehandler.name = "SimMP";
    self->ehandler.enabled = 1;
    self->ehandler.mouse_press = mouse_press;
    self->ehandler.mouse_motion = mouse_motion;
    self->ehandler.mouse_release = mouse_release;
    self->ehandler.pick_query = pick_query;
    self->ehandler.hover_query = pick_query;
    self->ehandler.key_press = key_press;
    self->ehandler.user = self;

    pthread_mutex_init(&self->mutex, NULL);

    self->lc = globals_get_lcm();
    self->ctrans = globals_get_ctrans();
    self->config = globals_get_config();
    self->viewer = viewer;
    self->points = g_ptr_array_new();

    gtk_box_pack_start(GTK_BOX(renderer->widget), GTK_WIDGET(self->pw), TRUE, TRUE, 0);

    gtku_param_widget_add_double (self->pw, PARAM_SPEED, 
            GTKU_PARAM_WIDGET_SLIDER, -15, 15, 0.1, 5);

    GtkWidget *clear_button = gtk_button_new_with_label("Clear plan");
    gtk_box_pack_start(GTK_BOX(renderer->widget), clear_button, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(clear_button), "clicked", G_CALLBACK (on_clear_button), self);

    gtk_widget_show_all(renderer->widget);

    g_signal_connect (G_OBJECT (self->pw), "changed", 
                      G_CALLBACK (on_param_widget_changed), self);

    viewer_add_renderer(viewer, renderer, priority);
    viewer_add_event_handler(viewer, &self->ehandler, priority);

}


