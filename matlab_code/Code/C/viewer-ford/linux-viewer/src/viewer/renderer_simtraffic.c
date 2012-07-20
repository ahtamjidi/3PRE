#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <gdk/gdkkeysyms.h>

#include <lcm/lcm.h>
#include <gtk_util/gtk_util.h>
#include <common/rotations.h>
#include <common/timestamp.h>
#include <common/small_linalg.h>
#include <common/glib_util.h>
#include <common/math_util.h>
#include <common/geometry.h>
#include <common/rotations.h>
#include <glutil/glutil.h>
#include <lcmtypes/lcmtypes_track_list_t.h>

#include <dgc/globals.h>
#include <dgc/ctrans.h>

#include <libviewer/viewer.h>
#include <libviewer/rwx.h>

#include <lcmtypes/lcmtypes_pose_t.h>

#define VEHICLE_HEIGHT 1.6

#ifndef MESH_MODEL_PATH
#error "MESH_MODEL_PATH is not defined!"
#endif

typedef struct _RendererSimTraffic {
    Renderer renderer;
    EventHandler ehandler;

    CTrans * ctrans;
    lcm_t *lc;


    Viewer      *viewer;
    GtkuParamWidget *pw;

    int          display_detail;

    lcmtypes_track_list_t_subscription_t             *lchandler;
    lcmtypes_track_list_t *tracks;
    int64_t                      selected_vehicle_id;

    double                       last_xy[2];  // last mouse press

    GtkMenu *context_menu;
    int64_t menu_car_id;
} RendererSimTraffic;

static void simtraffic_free (Renderer *super);

static void rand_color(float f[4])
{
again:
    f[0] = randf() * 0.8;
    f[1] = randf() * 0.8;
    f[2] = randf() * 0.8;
    f[3] = 0.6;

    float v = f[0] + f[1] + f[2];

    // reject colors that are too dark
    if (v < 0.3)
        goto again;
}

static void
_draw_track (RendererSimTraffic *self, const lcmtypes_track_t *vd)
{
    // generate a stable non-black, non-white color from object ID
    srand(vd->id);
    GLfloat rgb[4];
    rand_color(rgb);

    glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, rgb);
    glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, rgb);

    // assumes viewing, modeling matrices, etc. set up for local frame
    // leaves these matrices as they were before invocation
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix();

    glTranslated (vd->pos[0], vd->pos[1], 0);

// rotate basis to body coords of vehicle (rotate by theta about Z axis)
    glRotated (to_degrees(vd->theta), 0.0, 0.0, 1.0); // convert rad to deg

    if (self->ehandler.hovering && 
            self->selected_vehicle_id == vd->id) {
        glColor4f (1, 1, 1, 0.6);
    } else {
        glColor4fv(rgb);
    }

//    int use_model =  gtku_param_widget_get_bool(self->pw, PARAM_PRETTY);
//
//    if (use_model) {
//
//        glRotated(90, 0, 0, 1);
//        glRotated(90, 1, 0, 0);
//        
//        double s = 4.5;
//        glScalef(s,s,s);
//        
//        glTranslated(0,.115,0); // car height
//        glTranslated(0,0,.1); // front-to-back offset
//        glBegin(GL_TRIANGLES);
//        for (int i = 0; i < numChevy_vanFaces; i++) {
//            int *vs = Chevy_vanFaces[i];
//            for (int j = 0; j < 3; j++) {
//                glNormal3fv(Chevy_vanVertNorms[vs[j]]);
//                glVertex3fv(Chevy_vanVerts[vs[j]]);
//            }
//            
//        }
//        glEnd();
//    } else {

    glTranslated ( 0, 0, VEHICLE_HEIGHT / 2);

    glScalef (vd->size[0], vd->size[1], VEHICLE_HEIGHT);
    glutil_draw_cube();
//    }

    // restore matrix state
    glPopMatrix();

    if (self->ehandler.picking && self->selected_vehicle_id == vd->id) {
        glColor4f (1, 1, 1, 0.6);
        glPushMatrix ();
        glTranslatef (self->last_xy[0], self->last_xy[1], 0);
        glRotated (to_degrees(vd->theta), 0.0, 0.0, 1.0); // convert rad to deg
        glScalef (vd->size[0], vd->size[1], VEHICLE_HEIGHT);
        glutil_draw_cube_frame ();
        glPopMatrix ();
    }
}

static void 
simtraffic_draw (Viewer *viewer, Renderer *super)
{
    RendererSimTraffic *self = (RendererSimTraffic*) super->user;
    (void) self;

    for (int i=0; self->tracks && i<self->tracks->ntracks; i++) {
        lcmtypes_track_t *vd = &self->tracks->tracks[i];
        _draw_track (self, vd);
    }

}

static void
on_param_widget_changed (GtkuParamWidget *pw, const char *name, void *user)
{
    RendererSimTraffic *self = (RendererSimTraffic*) user;

    viewer_request_redraw (self->viewer);
}


static int key_press (Viewer *viewer, EventHandler *ehandler, 
        const GdkEventKey *event)
{
//    RendererSimTraffic *self = ehandler->user;
    return 0;
}

static int 
mouse_press (Viewer *viewer, EventHandler *ehandler,
                        const double ray_start[3], const double ray_dir[3], 
                        const GdkEventButton *event)
{
    RendererSimTraffic *self = (RendererSimTraffic*) ehandler->user;

    if (event->type == GDK_2BUTTON_PRESS) {
        self->display_detail ^= 1;
    }

    geom_ray_z_plane_intersect_3d((point3d_t*)ray_start, 
            (point3d_t*)ray_dir, 0, (point2d_t*)self->last_xy);

    if (self->ehandler.picking && event->button == 3) {
        gtk_menu_popup (self->context_menu, NULL, NULL, NULL, NULL,
                event->button, event->time);
        self->menu_car_id = self->selected_vehicle_id;
        self->ehandler.picking = 0;
    }

    return 0;
}

static int 
mouse_release(Viewer *viewer, EventHandler *ehandler,
                         const double ray_start[3], const double ray_dir[3], 
                         const GdkEventButton *event)
{
    RendererSimTraffic *self = (RendererSimTraffic*) ehandler->user;

    if (self->ehandler.picking) {
        if (!ctrans_have_pose (self->ctrans) ||
                !ctrans_have_gps_to_local (self->ctrans)) {
            self->ehandler.picking = 0;
            return 0;
        }
        double car_pos[3];
        ctrans_local_pos (self->ctrans, car_pos);

        geom_ray_z_plane_intersect_3d(POINT3D(ray_start), 
                POINT3D(ray_dir), car_pos[2], POINT2D(self->last_xy));
        lcmtypes_track_t msg = {
            .id = self->selected_vehicle_id,
            .pos = { self->last_xy[0], self->last_xy[1] },
            .vel = { 0, 0 },
            .size = { 0, 0 },
            .size_good = TRUE,
            .theta = 0,
            .confidence = 3,
        };
        lcmtypes_track_t_publish (self->lc, "TSIM_MOVE", &msg);
    }
    
    self->ehandler.picking = 0;
    return 0;
}

static int 
mouse_motion (Viewer *viewer, EventHandler *ehandler,
                         const double ray_start[3], const double ray_dir[3], 
                         const GdkEventMotion *event)
{
    RendererSimTraffic *self = (RendererSimTraffic*) ehandler->user;

    if (!self->ehandler.picking || 
            !event->state & GDK_BUTTON1_MASK) return 0;
    if (!ctrans_have_pose (self->ctrans) ||
            !ctrans_have_gps_to_local (self->ctrans)) {
        return 0;
    }
    double car_pos[3];
    ctrans_local_pos (self->ctrans, car_pos);

    geom_ray_z_plane_intersect_3d (POINT3D(ray_start), 
            POINT3D(ray_dir), car_pos[2], POINT2D(self->last_xy));
    
    return 1;
}

static double 
pick_query(Viewer *viewer, EventHandler *ehandler, const double ray_start[3], 
        const double ray_dir[3])
{
    RendererSimTraffic *self = (RendererSimTraffic*) ehandler->user;
    if (!ctrans_have_pose (self->ctrans)) return -1;
    if (!self->tracks) return -1;

    double car_pos[3];
    ctrans_local_pos (self->ctrans, car_pos);
    vec3d_t rd = { ray_dir[0], ray_dir[1], ray_dir[2] };
    geom_vec_normalize_3d (&rd);

    double tmin = INFINITY;

    for (int i=0; i<self->tracks->ntracks; i++) {
        lcmtypes_track_t *vd = &self->tracks->tracks[i];
        point3d_t box_size = { vd->size[0], vd->size[1], VEHICLE_HEIGHT };

        // transform the ray into object frame
        double rs_obj[3] = {
            ray_start[0] - vd->pos[0], 
            ray_start[1] - vd->pos[1],
            ray_start[2] - (car_pos[2] + VEHICLE_HEIGHT / 2)
        };
        double rquat[4];
        double zaxis[3] = { 0, 0, 1 };
        rot_angle_axis_to_quat (vd->theta, zaxis, rquat);
        rot_quat_rotate_rev (rquat, rs_obj);
        double rd_obj[3] = { rd.x, rd.y, rd.z };
        rot_quat_rotate_rev (rquat, rd_obj);
        point3d_t pos = { 0, 0, 0 };

        // in object frame, intersection test is easy
        double t = geom_ray_axis_aligned_box_intersect_3d (POINT3D(rs_obj), 
                POINT3D(rd_obj), &pos, &box_size, NULL);

        if (isfinite (t) && isless (t, tmin)) {
            tmin = t;
            self->selected_vehicle_id = vd->id;
        }
    }

    if (isfinite (tmin)) {
        return tmin;
    }

    self->ehandler.hovering = 0;
    return -1;
}

static void
on_add_bt_clicked (GtkWidget *bt, void *user_data)
{
    RendererSimTraffic *self = user_data;
    lcm_publish (self->lc, "TSIM_NEWCAR", NULL, 0);
}

static void
on_pause_mi_activate (GtkMenuItem *menu_item, void *user_data)
{
    RendererSimTraffic *self = user_data;
    char msg[40];
    sprintf (msg, "%"PRId64, self->menu_car_id);
    lcm_publish (self->lc, "TSIM_PAUSE", (uint8_t*)msg, strlen (msg));
}

static void
on_unpause_mi_activate (GtkMenuItem *menu_item, void *user_data)
{
    RendererSimTraffic *self = user_data;
    char msg[40];
    sprintf (msg, "%"PRId64, self->menu_car_id);
    lcm_publish (self->lc, "TSIM_UNPAUSE", (uint8_t*)msg, strlen (msg));
}

static void
on_track_list (const lcm_recv_buf_t *rbuf, const char *channel, 
        const lcmtypes_track_list_t *msg, void *user_data)
{
    RendererSimTraffic *self = user_data;
    if (self->tracks) {
        lcmtypes_track_list_t_destroy (self->tracks);
    }
    self->tracks = lcmtypes_track_list_t_copy (msg);
}

void setup_renderer_simtraffic(Viewer *viewer, int render_priority)
{
    RendererSimTraffic *self = 
        (RendererSimTraffic*) calloc (1, sizeof (RendererSimTraffic));

    // renderer administrativa
    Renderer *renderer = &self->renderer;

    renderer->draw = simtraffic_draw;
    renderer->destroy = simtraffic_free;

    renderer->widget = gtk_vbox_new(FALSE, 0);
    renderer->name = "SimTraffic";
    renderer->user = self;
    renderer->enabled = 1;

    // event handler
    EventHandler *ehandler = &self->ehandler;
    ehandler->name = "SimTraffic";
    ehandler->enabled = 1;
    ehandler->pick_query = pick_query;
    ehandler->hover_query = pick_query;
    ehandler->mouse_press = mouse_press;
    ehandler->mouse_motion = mouse_motion;
    ehandler->mouse_release = mouse_release;
    ehandler->key_press = key_press;
    ehandler->user = self;

    // member variables
    self->viewer = viewer;
    self->lc = globals_get_lcm();
    self->ctrans = globals_get_ctrans();
    self->tracks = NULL;
    self->lchandler = lcmtypes_track_list_t_subscribe (self->lc, 
            "LIDAR_TRACKS_SIM", on_track_list, self);

    // parameter widget
    self->pw = GTKU_PARAM_WIDGET(gtku_param_widget_new());
    g_signal_connect (G_OBJECT (self->pw), "changed", 
                      G_CALLBACK (on_param_widget_changed), self);
    on_param_widget_changed(self->pw, "", self);

    gtk_box_pack_start(GTK_BOX(renderer->widget), GTK_WIDGET(self->pw), 
            TRUE, TRUE, 0);

    // add car button
    GtkWidget *add_bt = gtk_button_new_with_label ("Add car");
    gtk_box_pack_start (GTK_BOX (renderer->widget), add_bt,
            FALSE, FALSE, 0);
    gtk_widget_show (add_bt);
    g_signal_connect (G_OBJECT(add_bt), "clicked", 
            G_CALLBACK(on_add_bt_clicked), self);

    gtk_widget_show_all(renderer->widget);
    
    // popup menu
    self->context_menu = GTK_MENU (gtk_menu_new ());
    g_object_ref_sink (self->context_menu);

    GtkWidget *pause_mi = gtk_menu_item_new_with_label ("Pause");
    GtkWidget *unpause_mi = gtk_menu_item_new_with_label ("Unpause");
    gtk_menu_shell_append (GTK_MENU_SHELL(self->context_menu), pause_mi);
    gtk_menu_shell_append (GTK_MENU_SHELL(self->context_menu), unpause_mi);
    g_signal_connect (G_OBJECT(pause_mi), "activate", 
            G_CALLBACK(on_pause_mi_activate), self);
    g_signal_connect (G_OBJECT(unpause_mi), "activate", 
            G_CALLBACK(on_unpause_mi_activate), self);
    gtk_widget_show_all (GTK_WIDGET (self->context_menu));

    // administrativa
    viewer_add_renderer(viewer, renderer, render_priority);
    viewer_add_event_handler(viewer, ehandler, render_priority);
}

static void
simtraffic_free (Renderer *super)
{
    RendererSimTraffic *self = (RendererSimTraffic*) super->user;
    if (self->tracks) {
        lcmtypes_track_list_t_destroy (self->tracks);
    }
    lcmtypes_track_list_t_unsubscribe (self->lc, self->lchandler);
    globals_release_ctrans (self->ctrans);
    globals_release_lcm (self->lc);

    g_object_unref (self->context_menu);
    free (self);
}
