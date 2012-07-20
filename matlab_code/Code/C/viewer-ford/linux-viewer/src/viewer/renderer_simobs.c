#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <gdk/gdkkeysyms.h>

#include <libviewer/viewer.h>
#include <common/math_util.h>
#include <common/glib_util.h>
#include <common/geometry.h>
#include <common/gps_linearize.h>
#include <common/timestamp.h>
#include <common/rotations.h>
#include <common/fasttrig.h>
#include <dgc/globals.h>
#include <pthread.h>

#include <lcmtypes/lcmtypes_rect_list_t.h>

#include <lcm/lcm.h>

#include <libviewer/viewer.h>


#define TRANSMIT_OBSTACLES_HZ 20
#define MIN_SIZE 0.3

#define PARAM_PEPPER "Pepper" 
#define NUM_PEPPER_GRAINS 8 

typedef struct _RendererSimObs {
    Renderer     renderer;
    EventHandler ehandler;
    Viewer        *viewer;
    lcm_t          *lc;
    CTrans        *ctrans;
    GtkuParamWidget *pw;
 
    GArray        *rects;
    double         rects_xy[2];
    pthread_mutex_t mutex;

    // this is the rect that is being edited (depending
    // on the state)
    int edit_idx;

    // last rect that we found in a query
    int hover_idx;
    double lastxy[2];

    // set to one if the user manually gave us "pick" by pressing a key
    int         key_pick;
    int transmitter;
} RendererSimObs;

void activate_simobs_transmitter(RendererSimObs *self);


static void on_sim_rects_new(const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_rect_list_t *msg, void *_user)
{
    RendererSimObs *self = (RendererSimObs*) _user;
 
    pthread_mutex_lock(&self->mutex);

    // clear our state first
    self->ehandler.picking = 0;
    self->edit_idx = -1;
    self->hover_idx = -1;
    g_array_set_size(self->rects, msg->num_rects);

    // set up.
    self->rects_xy[0] = msg->xy[0];
    self->rects_xy[1] = msg->xy[1];
    memcpy(self->rects->data,msg->rects,sizeof(lcmtypes_rect_t)*self->rects->len);

    if (!self->transmitter) {
        activate_simobs_transmitter(self);
    }
 
    pthread_mutex_unlock(&self->mutex);
}



static int find_idx(RendererSimObs *self, const double xy[2])
{
    double best_distance = HUGE;
    int best_idx = -1;

    for (unsigned int i = 0; i < self->rects->len; i++) {
        lcmtypes_rect_t *rect = &g_array_index(self->rects, lcmtypes_rect_t, i);
        double dx = xy[0] - (self->rects_xy[0] + rect->dxy[0]);
        double dy = xy[1] - (self->rects_xy[1] + rect->dxy[1]);
        double distance = sqrt(sq(dx) + sq(dy));
        double c,s;
        sincos(-rect->theta,&s,&c);
        double tx = c*dx - s*dy;
        double ty = s*dx + c*dy;

        if (distance < best_distance && fabs(tx) < rect->size[0]/2 && fabs(ty) < rect->size[1]/2) {
            best_distance = distance;
            best_idx = i;
        }
    }

    return best_idx;
}

static int key_press (Viewer *viewer, EventHandler *ehandler, const GdkEventKey *event)
{
    RendererSimObs *self = (RendererSimObs*) ehandler->user;

    if (event->keyval == 'o' || event->keyval == 'O') {
        viewer_request_pick(viewer, ehandler);
        self->key_pick = 1;
        return 1;
    }


    if (self->edit_idx>-1 && (event->keyval == GDK_Delete || 
                           event->keyval == GDK_BackSpace)) {
        
        pthread_mutex_lock(&self->mutex);
        // find rect index
        if (self->edit_idx>-1)
            g_array_remove_index_fast(self->rects, self->edit_idx);
        self->edit_idx = -1;
        self->hover_idx = -1;
        pthread_mutex_unlock(&self->mutex);
        viewer_request_redraw(viewer);
        return 1;
    }


    if (event->keyval == GDK_Escape) {
        ehandler->picking = 0;
        self->key_pick = 0;
    }

    return 0;
}

static int mouse_press (Viewer *viewer, EventHandler *ehandler,
                        const double ray_start[3], const double ray_dir[3], const GdkEventButton *event)
{
    RendererSimObs *self = (RendererSimObs*) ehandler->user;

    double xy[2];
    int consumed = 0;

    geom_ray_z_plane_intersect_3d(POINT3D(ray_start), POINT3D(ray_dir), 
            0, POINT2D(xy));

    memcpy(self->lastxy, xy, 2 * sizeof(double));
    int control = event->state & GDK_CONTROL_MASK;

    // it could be a pick (which begins a move)
    // or creation of a new click

    // selecting a previously-created rect
    if (event->button == 1 && ehandler->picking) {
        self->edit_idx = find_idx(self, xy);
        if (self->edit_idx >= 0)
            consumed = 1;
    } 

    if (ehandler->picking && self->edit_idx <0 && event->button == 1 && control) {
        // creating a new rect
        lcmtypes_rect_t rect;

        rect.dxy[0] = xy[0] - self->rects_xy[0];
        rect.dxy[1] = xy[1] - self->rects_xy[1];
        rect.size[0] = 1;
        rect.size[1] = 1;
        rect.theta = 0;
        g_array_append_val(self->rects, rect);
        self->edit_idx = self->rects->len-1;
        consumed = 1;
        viewer_request_pick(viewer, ehandler);
        activate_simobs_transmitter(self);
    } 

    viewer_request_redraw(viewer);

    return consumed;
}

static int mouse_motion (Viewer *viewer, EventHandler *ehandler,
                         const double ray_start[3], const double ray_dir[3], const GdkEventMotion *event)
{
    RendererSimObs *self = (RendererSimObs*) ehandler->user;
 
    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D(ray_start), POINT3D(ray_dir), 
            0, POINT2D(xy));

    int consumed = 0;
    int control = event->state & GDK_CONTROL_MASK;
    int shift = event->state & GDK_SHIFT_MASK;

    // only handle mouse button 1.
    if (!(event->state & GDK_BUTTON1_MASK))
        return 0;

    if (self->edit_idx<0)
        return 0;

    lcmtypes_rect_t *rect = &g_array_index(self->rects, lcmtypes_rect_t, self->edit_idx);
    double real_xy[2] = {self->rects_xy[0] + rect->dxy[0], self->rects_xy[1] + rect->dxy[1]};
    if (event->state==GDK_BUTTON1_MASK) {
      
        //  gps_linearize_to_lat_lon(self->gpslin, xy, self->edit_rect->ll);
        rect->dxy[0]= xy[0] - self->rects_xy[0];
        rect->dxy[1]= xy[1] - self->rects_xy[1];
        consumed = 1;
        
    } else if (control) { 
      
        double sx = xy[0] - real_xy[0];
        double sy = xy[1] - real_xy[1];
        
        double tx = cos(-rect->theta)*sx - sin(-rect->theta)*sy;
        double ty = sin(-rect->theta)*sx + cos(-rect->theta)*sy;
        
        rect->size[0] = fmax(fabs(2*tx), MIN_SIZE);
        rect->size[1] = fmax(fabs(2*ty), MIN_SIZE);
        consumed = 1;
        
    } else if (shift) {
        
        double dtheta = 
            atan2(xy[1] - real_xy[1],
                  xy[0] - real_xy[0]) - 
            atan2(self->lastxy[1] - real_xy[1],
                  self->lastxy[0] - real_xy[0]);
        
        memcpy(self->lastxy, xy, 2 * sizeof(double));
        
        rect->theta += dtheta;
        
        consumed = 1;
    } 
    
    viewer_request_redraw(viewer);
    return consumed;
}

static int mouse_release (Viewer *viewer, EventHandler *ehandler,
                          const double ray_start[3], const double ray_dir[3], const GdkEventButton *event)
{
    RendererSimObs *self = (RendererSimObs*) ehandler->user;

    self->edit_idx = -1;
    if (!self->key_pick)
        ehandler->picking = 0;
    
    viewer_request_redraw(viewer);
    return 1;
}

static double pick_query(Viewer *viewer, EventHandler *ehandler, const double ray_start[3], const double ray_dir[3])
{
    RendererSimObs *self = (RendererSimObs*) ehandler->user;

    double xy[2];
    geom_ray_z_plane_intersect_3d(POINT3D(ray_start), 
            POINT3D(ray_dir), 0, POINT2D(xy));

    self->hover_idx = find_idx(self, xy);
    if (self->hover_idx<0)
        return -1;
    
    lcmtypes_rect_t *rect = &g_array_index(self->rects, lcmtypes_rect_t, self->hover_idx);

    return sqrt(sq(self->rects_xy[0] + rect->dxy[0] - xy[0]) + 
                sq(self->rects_xy[0] + rect->dxy[1] - xy[1]));
}

static void draw_rect (RendererSimObs *self, lcmtypes_rect_t *rect, int draw_orient)
{
    //double cxy[2];
//    gps_linearize_to_xy(self->gpslin, rect->ll, cxy);

    glPushMatrix();
    double real_xy[2] = { self->rects_xy[0] + rect->dxy[0], self->rects_xy[1] + rect->dxy[1]};
 
    glTranslated(real_xy[0], real_xy[1], 0);
    glRotated(to_degrees(rect->theta), 0, 0, 1);
    glTranslated(-real_xy[0], -real_xy[1], 0);

    glEnable(GL_BLEND);
    glBegin(GL_QUADS);
    glVertex2d(real_xy[0] - rect->size[0]/2, real_xy[1] - rect->size[1]/2);
    glVertex2d(real_xy[0] - rect->size[0]/2, real_xy[1] + rect->size[1]/2);
    glVertex2d(real_xy[0] + rect->size[0]/2, real_xy[1] + rect->size[1]/2);
    glVertex2d(real_xy[0] + rect->size[0]/2, real_xy[1] - rect->size[1]/2);
    glEnd();    
    glPopMatrix();
    
    if (draw_orient) {
        glBegin(GL_LINES);
        glColor3f(1, 1, 0);
        glVertex2d(real_xy[0], real_xy[1]);
        double dx = cos(rect->theta)*rect->size[0]/2 - sin(rect->theta)*rect->size[1]/2;
        double dy = sin(rect->theta)*rect->size[0]/2 + cos(rect->theta)*rect->size[1]/2;
        glVertex2d(real_xy[0] + dx, real_xy[1] + dy);
        glEnd();
    }
}

static void my_draw( Viewer *viewer, Renderer *super )
{
    RendererSimObs *self = (RendererSimObs*) super->user;

    for (unsigned int i = 0; i < self->rects->len; i++) {
        lcmtypes_rect_t *rect = &g_array_index(self->rects, lcmtypes_rect_t, i);
        
        if (self->edit_idx == i)
            glColor4f(1, 1, .5, .5);
        else if (self->ehandler.hovering && self->hover_idx == i)
            glColor4f(.5, 1, 1, .5);
        else
            glColor4f(1, 1, 1, 0.3);

        draw_rect(self, rect, 0);
    }
}    

gboolean on_transmit_obs(gpointer _user)
{
    RendererSimObs *self = (RendererSimObs*) _user;
    
    if (!ctrans_have_pose(self->ctrans))
        return TRUE;
    
    lcmtypes_pose_t pose;
    ctrans_local_pose(self->ctrans,&pose);
    
    int pepper = 0;
    if (gtku_param_widget_get_bool(self->pw, PARAM_PEPPER))
        pepper = NUM_PEPPER_GRAINS;

    lcmtypes_rect_list_t *rects = (lcmtypes_rect_list_t*) calloc(1, (pepper?pepper:1)*sizeof(lcmtypes_rect_list_t));
    rects->utime = timestamp_now();
    rects->num_rects = self->rects->len*(pepper?pepper:1);
    rects->rects = (lcmtypes_rect_t*) calloc(rects->num_rects, sizeof(lcmtypes_rect_t));

    double cxyz[3] = { self->rects_xy[0], self->rects_xy[1], 0};

    rects->xy[0] = cxyz[0];
    rects->xy[1] = cxyz[1];
    int rect_idx =0;
    for (unsigned int i = 0; i < self->rects->len; i++) {
        lcmtypes_rect_t *myrect = & g_array_index(self->rects, lcmtypes_rect_t,i);
        if (pepper) {
            double pose_dir = fasttrig_atan2(pose.pos[1]-(cxyz[1] +myrect->dxy[1]) ,pose.pos[0]-(cxyz[0] +myrect->dxy[0]));
            double s,c;
            fasttrig_sincos(-myrect->theta,&s,&c);
            double s2,c2;
            fasttrig_sincos(myrect->theta-pose_dir,&s2,&c2);
            for (int p=0;p<pepper;p++) {
                lcmtypes_rect_t *rect = &rects->rects[rect_idx++];
                double dx=0.0;
                double dy=0.0;
                if (p<NUM_PEPPER_GRAINS/2) { 
                    dx=-0.5*myrect->size[0]+myrect->size[0]*rand()/((double)RAND_MAX);
                    dy+=0.5*myrect->size[1]*((-s2+c2)<0?-1:1);

                }
                else {
                    dx+=0.5*myrect->size[0]*((c2+s2)<0?-1:1);
                    dy=-0.5*myrect->size[1]+myrect->size[1]*rand()/((double)RAND_MAX);
                }
                rect->dxy[0]     = myrect->dxy[0] - cxyz[0] + c*dx+s*dy;
                rect->dxy[1]     = myrect->dxy[1] - cxyz[1] - s*dx+c*dy;
                rect->size[0]   = 0.27;
                rect->size[1]   = 0.27;
                rect->theta     = 0.0;// + gps_to_local.lat_lon_el_theta[3];
            }
        }
        else {
            lcmtypes_rect_t *rect = &rects->rects[rect_idx++];
            //double xyz[3];
            //double lle[3] = { rect->ll[0], myrect->ll[1], 0 };
            //ctrans_gps_to_local (self->ctrans, lle, xyz, NULL);
            
            rect->dxy[0]     = myrect->dxy[0] - cxyz[0];
            rect->dxy[1]     = myrect->dxy[1] - cxyz[1];
            rect->size[0]   = myrect->size[0];
            rect->size[1]   = myrect->size[1];
            rect->theta     = myrect->theta;// + gps_to_local.lat_lon_el_theta[3];
        }
    }
    lcmtypes_rect_list_t_publish(self->lc, "SIM_RECTS", rects);
    free(rects->rects);

    // Stop transmitting if there are no Rects
    if (rects->num_rects)
        return TRUE;
    else {
        self->transmitter = 0;
        return FALSE;
    }
}


void activate_simobs_transmitter(RendererSimObs *self)
{
    if (!self->transmitter)
        self->transmitter = g_timeout_add(1000 / TRANSMIT_OBSTACLES_HZ, on_transmit_obs, self);
}


void setup_renderer_simobs(Viewer *viewer, int priority)
{
    if (!viewer->simulation_flag)
        return;

    RendererSimObs *self = (RendererSimObs*) calloc(1, sizeof(RendererSimObs));

    self->viewer = viewer;
    self->ctrans = globals_get_ctrans();
    self->lc = globals_get_lcm();
    self->rects = g_array_new(FALSE, FALSE, sizeof(lcmtypes_rect_t));
    self->edit_idx =-1;
    self->hover_idx =-1;
    self->pw = GTKU_PARAM_WIDGET(gtku_param_widget_new());
    self->renderer.widget = GTK_WIDGET(self->pw);

    self->renderer.enabled = 1;
    self->renderer.name = "SimObstacle";
    self->renderer.draw = my_draw;
    self->renderer.user = self;

   
    self->ehandler.name = "SimObstacle";
    self->ehandler.enabled = 1;
    self->ehandler.mouse_press = mouse_press;
    self->ehandler.mouse_motion = mouse_motion;
    self->ehandler.mouse_release = mouse_release;
    self->ehandler.pick_query = pick_query;
    self->ehandler.hover_query = pick_query;
    self->ehandler.key_press = key_press;
    self->ehandler.user = self;
    

    gtku_param_widget_add_booleans (self->pw, 0, PARAM_PEPPER, 0, NULL);

    lcmtypes_rect_list_t_subscribe(self->lc, "SIM_RECTS_NEW", on_sim_rects_new, self);

    viewer_add_event_handler(viewer, &self->ehandler, priority);
    viewer_add_renderer(viewer, &self->renderer, priority);

}
