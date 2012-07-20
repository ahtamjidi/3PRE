#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <gtk/gtk.h>

#include <gtk_util/gtk_util.h>

#include <common/getopt.h>
#include <common/glib_util.h>
#include <common/udp_util.h>
#include <dgc/globals.h>
#include <dgc/config_util.h>

#include "libviewer/viewer.h"
#include "dgcviewer.h"
#include "camera_view_handler.h"

static int logplayer_remote_on_key_press(Viewer *viewer, EventHandler *ehandler, const GdkEventKey *event)
{
    int keyval = event->keyval;

    switch (keyval)
    {
    case 'P':
    case 'p':
        udp_send_string("127.0.0.1", 53261, "PLAYPAUSETOGGLE");
        break;
    case 'N':
    case 'n':
        udp_send_string("127.0.0.1", 53261, "STEP");
        break;
    case '=':
    case '+':
        udp_send_string("127.0.0.1", 53261, "FASTER");
        break;
    case '_':
    case '-':
        udp_send_string("127.0.0.1", 53261, "SLOWER");
        break;
    case '[':
        udp_send_string("127.0.0.1", 53261, "BACK5");
        break;
    case ']':
        udp_send_string("127.0.0.1", 53261, "FORWARD5");
        break;
    default:
        return 0;
    }

    return 1;
}

/////////////////////////////////////////////////////////////

static void on_perspective_item(GtkMenuItem *mi, void *user)
{
    Viewer *viewer = VIEWER (user);
    viewer_set_view_handler (viewer, NULL);
    ViewHandler *vhandler = viewer->view_handler;

    vhandler->set_camera_perspective(vhandler, 60);
}

static void on_orthographic_item(GtkMenuItem *mi, void *user)
{
    Viewer *viewer = VIEWER (user);
    viewer_set_view_handler (viewer, NULL);
    ViewHandler *vhandler = viewer->view_handler;

    vhandler->set_camera_orthographic(vhandler);
}

void setup_renderer_grid(Viewer *viewer, int render_priority);
void setup_renderer_car(Viewer *viewer, int render_priority);
void setup_renderer_laser(Viewer *viewer, int render_priority);
//void setup_renderer_velodyne(Viewer *viewer, int render_priority);
void setup_renderer_textured(Viewer *viewer, int render_priority);
void setup_renderer_compass(Viewer *viewer, int priority);
void setup_renderer_debug(Viewer *viewer, int priority);
void setup_renderer_goal(Viewer *viewer, int priority);
void setup_renderer_lcgl(Viewer *viewer, int priority);
void setup_renderer_lanes(Viewer *viewer, int priority);
void setup_renderer_scrolling_plots(Viewer *viewer, int priority);
void setup_renderer_adu(Viewer *viewer, int priority);
void setup_renderer_rndf (Viewer *viewer, int render_priority);
void setup_renderer_cam_thumb (Viewer *viewer, int render_priority);
//void setup_renderer_radar (Viewer *viewer, int render_priority);
void setup_renderer_motion_plan (Viewer *viewer, int render_priority);
void setup_renderer_rrt_debug (Viewer *viewer, int render_priority);
void setup_renderer_navigator (Viewer *viewer, int render_priority);
void setup_renderer_sim_motion_plan (Viewer *viewer, int render_priority);
void setup_renderer_simobs (Viewer *viewer, int render_priority);
void setup_renderer_sensor_placement (Viewer *viewer, int render_priority);
void setup_menu_simstate (Viewer *viewer, int render_priority);
void setup_renderer_tracks(Viewer *viewer, int priority);
void setup_renderer_console(Viewer *viewer, int priority);
void setup_renderer_simtraffic(Viewer *viewer, int priority);
void setup_renderer_statusline(Viewer *viewer, int priority);
//void setup_renderer_procman (Viewer *viewer, int render_priority);
void setup_renderer_simcurblines (Viewer *viewer, int render_priority);

static void
on_camera_view_handler_rmi_activate (GtkRadioMenuItem *rmi, void *user_data)
{
    Viewer *viewer = VIEWER (user_data);
    ViewHandler *vh = 
        (ViewHandler*) g_object_get_data (G_OBJECT (rmi), "Viewer:cvh");
    viewer_set_view_handler (viewer, vh);
    viewer_request_redraw (viewer);
}

static void
add_camera_view_handler (Viewer *viewer, const char *cam_name,
        GSList *group, GtkMenu *view_menu)
{
    CameraViewHandler *cvh = camera_view_handler_new (viewer, cam_name);

    if (!cvh) return;
//    self->camera_view_handlers = g_list_append (self->camera_view_handlers,
//            cvh);

    char *label = g_strdup_printf ("Camera POV: %s", cam_name);
    GtkWidget *cvh_rmi = gtk_radio_menu_item_new_with_label (group, label);
    free (label);
    gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), cvh_rmi);
    gtk_widget_show (cvh_rmi);

    g_object_set_data (G_OBJECT (cvh_rmi), "Viewer:cvh", cvh);
    g_signal_connect (G_OBJECT (cvh_rmi), "activate", 
            G_CALLBACK (on_camera_view_handler_rmi_activate), viewer);
}

static void
add_view_handlers (Viewer *viewer)
{
    GtkWidget *view_menuitem = gtk_menu_item_new_with_mnemonic("_View");
    gtk_menu_bar_append(GTK_MENU_BAR(viewer->menu_bar), view_menuitem);

    GtkWidget *view_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_menuitem), view_menu);

    GSList *view_list = NULL;
    GtkWidget *perspective_item = gtk_radio_menu_item_new_with_label(view_list, 
            "Perspective");
    view_list = 
        gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(perspective_item));
    gtk_menu_append(GTK_MENU(view_menu), perspective_item);
    g_signal_connect(G_OBJECT(perspective_item), "activate", 
            G_CALLBACK(on_perspective_item), viewer);

    GtkWidget *orthographic_item = 
        gtk_radio_menu_item_new_with_label(view_list, "Orthographic");
    view_list = 
        gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(orthographic_item));
    gtk_menu_append(GTK_MENU(view_menu), orthographic_item);
    g_signal_connect(G_OBJECT(orthographic_item), "activate", 
            G_CALLBACK(on_orthographic_item), viewer);

    // add camera view handlers
    Config *config = globals_get_config ();
    char **cam_names = config_util_get_all_camera_names (config); 
    for (int i=0; cam_names && cam_names[i]; i++) {
        char *fqkey = g_strjoin ("", "calibration.cameras.", cam_names[i], 
                NULL);
        if (config_get_num_subkeys (config, fqkey) > 0) {
            view_list = gtk_radio_menu_item_get_group (
                    GTK_RADIO_MENU_ITEM(perspective_item));
            add_camera_view_handler (viewer, cam_names[i], view_list,
                    GTK_MENU (view_menu));
        }
        free (fqkey);
    }
    g_strfreev (cam_names);
    globals_release_config (config);

    gtk_widget_show_all(view_menuitem);
}

typedef struct {
    GtkLabel *label;
    CTrans *ctrans;
} mouse_pos_info_data_t;

static int
mouse_pos_info_on_mouse_motion (Viewer *viewer, EventHandler *ehandler,
        const double ray_start[3], const double ray_dir[3], 
        const GdkEventMotion *event)
{
    mouse_pos_info_data_t *mpid = ehandler->user;

    double carpos[3];
    if (ctrans_have_pose (mpid->ctrans)) {
        ctrans_local_pos (mpid->ctrans, carpos);
    } else {
        carpos[0] = carpos[1] = carpos[2] = 0;
    }

    point2d_t p;
    int status = geom_ray_z_plane_intersect_3d(POINT3D (ray_start),
            POINT3D (ray_dir), carpos[2], &p);
    if (0 == status) {
        char buf[40];
        snprintf (buf, sizeof (buf), "<%.2f, %.2f>", p.x, p.y);
        gtk_label_set_text (mpid->label, buf);
        gtk_widget_set_sensitive (GTK_WIDGET (mpid->label), TRUE);
    } else {
        gtk_label_set_text (mpid->label, "<??, \?\?>");
        gtk_widget_set_sensitive (GTK_WIDGET (mpid->label), FALSE);
    }
    return 0;
}

static void
add_mouse_position_info_event_handler (Viewer *viewer)
{
    GtkToolItem *toolitem = gtk_tool_item_new ();
    GtkWidget *label = gtk_label_new ("<??, \?\?>");
    gtk_widget_set_sensitive (label, FALSE);
    gtk_container_add (GTK_CONTAINER (toolitem), label);
    GtkToolItem *separator = gtk_separator_tool_item_new ();
    gtk_widget_show (GTK_WIDGET (separator));
    gtk_toolbar_insert (viewer->toolbar, separator, -1);
    gtk_toolbar_insert (viewer->toolbar, toolitem, -1);
    gtk_widget_show_all (GTK_WIDGET (toolitem));

    EventHandler *ehandler = (EventHandler*) calloc(1, sizeof(EventHandler));
    ehandler->name = "Mouse Position Info";
    ehandler->enabled = 1;
    ehandler->mouse_motion = mouse_pos_info_on_mouse_motion;
    viewer_add_event_handler(viewer, ehandler, 0);
    mouse_pos_info_data_t *mpid = g_slice_new0 (mouse_pos_info_data_t);
    mpid->label = GTK_LABEL (label);
    mpid->ctrans = globals_get_ctrans ();
    ehandler->user = mpid;
}

int main(int argc, char *argv[])
{
    gtk_init (&argc, &argv);
    glutInit (&argc, argv);
    g_thread_init (NULL);

    setlinebuf (stdout);

    getopt_t *gopt = getopt_create();
    getopt_add_bool(gopt, 'h',"help", 0,"Show this");
    getopt_add_bool(gopt, 0,"simulation", 0,"Enable simulation functions (Teleport, simobstacles)");
    getopt_add_bool(gopt, 'w', "white", 0, "Use white background color");

    if (!getopt_parse(gopt, argc, argv, 1) || getopt_get_bool(gopt,"help")) {
        printf("Usage: %s [options]\n\n", argv[0]);
        getopt_do_usage(gopt);
        //return 0;
    }

    Viewer *viewer = viewer_new();
    viewer_set_window_title (viewer, "Viewer");

    if (getopt_get_bool(gopt, "white")) {
        viewer->backgroundColor[0] = 1;
        viewer->backgroundColor[1] = 1;
        viewer->backgroundColor[2] = 1;
        viewer->backgroundColor[3] = 1;
    }
        
    if (getopt_get_bool(gopt,"simulation")) {
        viewer->simulation_flag=1;
        fprintf(stderr,"viewer: WARNING: SIMULATION OPTIONS ENABLED\n");
    }

    viewer_add_render_mode(viewer, RENDER_MODE_LOCAL, "Local");

    setup_renderer_grid(viewer, 1);
    setup_renderer_car(viewer, 0);
    setup_renderer_laser(viewer, 1);
//    setup_renderer_radar(viewer, 1);
    setup_renderer_velodyne(viewer, 1);
    setup_renderer_textured(viewer, 1);
    setup_renderer_compass(viewer, 0);
    setup_renderer_debug(viewer, 1);
    setup_renderer_goal(viewer, 1);
    setup_renderer_lanes(viewer, 1);
    setup_renderer_lcgl(viewer, 1);
    setup_renderer_scrolling_plots(viewer, 1);
    setup_renderer_adu(viewer, 0);
    setup_renderer_rndf (viewer, 1);
    setup_renderer_cam_thumb (viewer, 1);
    setup_renderer_motion_plan (viewer, 1);
    setup_renderer_rrt_debug (viewer, 1);
    setup_renderer_navigator (viewer, 1);
    setup_renderer_sim_motion_plan (viewer, 1);
    setup_renderer_simobs (viewer, 1);
    setup_renderer_sensor_placement (viewer, 0);
    setup_menu_simstate (viewer, 1);
    setup_renderer_tracks (viewer, 0);
    setup_renderer_console (viewer, 0);
    setup_renderer_statusline (viewer, 0);
    setup_renderer_simcurblines (viewer, 0);
    if (viewer->simulation_flag) {
        setup_renderer_simtraffic (viewer, 0);
    }
//    setup_renderer_procman (viewer, 1);

    // logplayer controls
    EventHandler *ehandler = (EventHandler*) calloc(1, sizeof(EventHandler));
    ehandler->name = "LogPlayer Remote";
    ehandler->enabled = 1;
    ehandler->key_press = logplayer_remote_on_key_press;
    viewer_add_event_handler(viewer, ehandler, 0);

    add_view_handlers (viewer);

    add_mouse_position_info_event_handler (viewer);

    char *fname = g_build_filename (g_get_user_config_dir(), ".dgcviewerrc", 
            NULL);
    viewer_load_preferences (viewer, fname);
//    g_signal_connect (G_OBJECT (viewer->window), "delete-event", 
//            G_CALLBACK (on_viewer_delete), viewer);
    gtk_main ();  
    viewer_save_preferences (viewer, fname);
    free (fname);
    viewer_unref (viewer);
}
