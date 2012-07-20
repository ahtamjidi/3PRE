#ifndef __viewer_h__
#define __viewer_h__

#include <gtk/gtk.h>
#include <glib-object.h>

#include <gtk_util/gtk_util.h>
#include <common/ezavi.h>
#include <zlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define USE_ZMOV 1

typedef struct _Viewer Viewer;
typedef struct _ViewHandler ViewHandler;
typedef struct _EventHandler EventHandler;

struct ViewerMode
{
    int           mode;
    const char   *name;
    GtkMenuItem  *menu_item;
    
};

#define FOLLOW_POS 1
#define FOLLOW_YAW 2
#define FOLLOW_ORIENTATION 4

struct _ViewHandler
{
    // view updating methods
    void (*update_gl_matrices)  (Viewer *viewer, ViewHandler *vhandler);

    void (*get_eye_look)        (ViewHandler *vhandler, double eye[3], 
                                 double lookat[3], double up[3]);
    void (*update_follow_target)(ViewHandler *vhandler, const double pos[3], 
                                 const double quat[4]);
    void (*set_look_at)         (ViewHandler *vhandler, const double eye[3], 
                                 const double lookat[3], const double up[3]);
    
    void (*set_camera_perspective) (ViewHandler *vhandler, double fov_degrees);
    void (*set_camera_orthographic) (ViewHandler *vhandler);
    
    void (*destroy)             (ViewHandler *vhandler);

    int  follow_mode;
    void *user;
};

struct _EventHandler
{
    /* We support a couple different modes of operation. First, is
       "picking": picking allows an event handler to capture user
       input when the user clicks on some object (managed by the event
       handler). This allows the object to be manipulated in an
       event-handler-specific way (e.g., moved, rotated).

       "Hovering" is similar to picking, but hovering queries occur
       when the mouse is merely moved, and does not affect the routing
       of events. This allows a renderer to display an object
       differently, so that the user knows the object can be clicked
       on (thus causing the object to be "picked"). 

       Both "picking" and "hovering" call a query function; it should
       return the distance from the click to an object. The event
       handler that returns the smallest distance will have its
       "picking" or "hovering" flag set. When "picking", the event
       handler will have first "dibs" on any event that subsequently
       occurs, regardless of the event handler's priority. If the
       event handler has no object near the click, it should return <
       0.

       Finally, when an event occurs:

       If no one is picking, and the event is mouse_down, pick_query
       is called for all event handlers. The winner will have its
       picking flag set, which will give it first dibs on subsequent
       events.

       If no one is picking, and the event is mouse_motion, the
       hover_query methods are called, and the winning event handler
       has its "hovering" flag set.
       
       Finally, we find an event handler to consume the event. The
       picking event handler, if any, gets first dibs. All other
       handlers are then called in order of decreasing priority.
       
       Only one handler "consumes" an event: if an event handler
       returns TRUE, it ends the event processing.
    */

    // a name used in the menu 
    char *name;

    // Completely enable/disable the event handler.
    int enabled;

    // Higher priority handlers are updated first. Do not set this directly;
    // only via set_priority.
    int priority;

    // set when a picker wins the pick_query competition and the user clicks.
    // when set, it indicates that the handler has "first dibs" on any events.
    // Only one handler can be picking simulatenously.
    int picking;

    // set when a picker wins the hover_query competition and the user
    // hovers. Typically, this indicates that a click would cause the picking
    // flag to be set. Only one handler can be hovering simultaneously.
    int hovering;

    // When a mouse press event occurs, all EventHandlers will compete
    // for the sequence of events that follow. The EventHandler that
    // returns the smallest distance will receive a pick_notify event.
    // Return < 0 if the renderer has no object to pick.
    double (*pick_query)(Viewer *viewer, EventHandler *ehandler, 
            const double ray_start[3], const double ray_dir[3]);

    // can usually be the same function pointer as pick_query. 
    double (*hover_query)(Viewer *viewer, EventHandler *ehandler, 
            const double ray_start[3], const double ray_dir[3]);

    // event handling methods. Return non-zero if the event was consumed
    // by this handler (and further processing should stop)
    int (*mouse_press)   (Viewer *viewer, EventHandler *ehandler,
                          const double ray_start[3], const double ray_dir[3], 
                          const GdkEventButton *event);

    // if you register a pick_query handler, you should almost certainly
    // register a mouse_release function to set picking = 0.
    int (*mouse_release) (Viewer *viewer, EventHandler *ehandler,
                          const double ray_start[3], const double ray_dir[3], 
                          const GdkEventButton *event);
    
    int (*mouse_motion)  (Viewer *viewer, EventHandler *ehandler,
                          const double ray_start[3], const double ray_dir[3], 
                          const GdkEventMotion *event);
    
    int (*mouse_scroll)  (Viewer *viewer, EventHandler *ehandler,
                          const double ray_start[3], const double ray_dir[3], 
                          const GdkEventScroll *event);
    
    int  (*key_press)     (Viewer *viewer, EventHandler *ehandler, 
            const GdkEventKey  *event);
    
    void (*destroy)       (EventHandler *ehandler);

    void *user;

    GtkWidget         *cmi;            // enable checkbox
};

typedef struct _Renderer Renderer;
struct _Renderer {

    int        enabled;

    // the priority with which this renderer will be drawn
    int        priority;

    char       *name;
    GtkWidget  *widget;

    // drawing methods
    void (*draw)          (Viewer *viewer, Renderer *renderer);
    void (*destroy)   (Renderer *renderer);
    void *user;

    // state used by viewer
    int expanded;
    GtkWidget         *cmi;            // enable checkbox
    GtkWidget         *expander;
    GtkWidget         *control_frame;
};

#define TYPE_VIEWER  viewer_get_type()
#define VIEWER(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_VIEWER, Viewer))
#define VIEWER_VIEWER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
            TYPE_VIEWER, ViewerClass ))
#define IS_VIEWER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_VIEWER))
#define IS_VIEWER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_VIEWER))

typedef struct _ViewerClass ViewerClass;

struct _Viewer {
    GObject parent;

    float backgroundColor[4];

    GtkuGLDrawingArea *gl_area;
    GtkTooltips       *tips;

    // event handlers sorted by priority (decreasing). 
    GPtrArray         *event_handlers;
    GPtrArray         *event_handlers_sorted;

    // The last-known picking handler (also a member of event_handlers)
    EventHandler      *picking_handler;

    // renderers sorted by priority (decreasing)
    GPtrArray         *renderers;      

    // renderers sorted by name (alphabetical)
    GPtrArray         *renderers_sorted; 

    // just those renderers with control widgets, sorted by name (alphabetical)
    GPtrArray         *renderers_sorted_with_controls;

    GPtrArray         *modes;          // user-defined (opaque) modes
    GSList            *modes_group;    // for the radio-list of modes
    int               mode;            // current mode

    ViewHandler       *view_handler;
    ViewHandler       *default_view_handler;

    GtkWidget         *controls_box;

    GtkWidget         *record_button;
    GtkWidget         *mode_menu;
    GtkWidget         *menu_bar;

    uint8_t           *mov_bgr_buf;
    GtkWidget         *fps_spin;

    char              *movie_path;
    uint8_t           *movie_buffer;
    int               movie_width, movie_height, movie_stride;
    int               movie_draw_pending;
    int               movie_frames;
    int64_t           movie_frame_last_utime;
    double            movie_actual_fps;
    double            movie_desired_fps;
#ifdef USE_ZMOV
    gzFile            *movie_gzf;
#else
    ezavi_t           *ezavi;
    FBGLDrawingArea   *fb_area;
#endif

    guint             render_timer_id;
    int               is_recording;

    GtkWidget         *file_menu;
    GtkWidget         *renderers_menu;
    GtkWidget         *event_handlers_menu;

    int64_t           last_draw_utime;  // when did we last render the gl view?
    int               redraw_timer_pending; // is a call to on_redraw_timer pending?

    int               prettier_flag;

    GtkWidget         *window;
    GtkWidget         *status_bar;
    GtkToolbar         *toolbar;

    char              *status_bar_message;
    int               simulation_flag;
};

struct _ViewerClass
{
    GObjectClass parent_class;
};

GType viewer_get_type ();

Viewer *viewer_new ();
void viewer_unref(Viewer *viewer);

void viewer_set_window_title (Viewer *viewer, const char *window_name);

// request a redraw (which will occur asynchronously)
void viewer_request_redraw (Viewer *viewer);

// Renderers are called at render time, in the order in which they
// were added to the viewer.
void viewer_add_renderer (Viewer *viewer, Renderer *plugin, int priority);
void viewer_remove_renderer(Viewer *viewer, Renderer *plugin);

/**
 * viewer_set_view_handler:
 * A viewer has a single "view handler" that manages the camera projection
 * and position. It is typically also an event handler that can respond to
 * user pan/zoom/rotate commands.
 *
 * Setting the view handler to NULL activates the default view handler.
 */
void viewer_set_view_handler(Viewer *viewer, ViewHandler *vhandler);

// If you want key/mouse events, you need to add an event handler.
void viewer_add_event_handler (Viewer *viewer, EventHandler *ehandler, 
        int priority);
void viewer_remove_event_handler(Viewer *viewer, EventHandler *ehandler);

// the priority of an event handler can be changed dynamically. This
// is useful, for example, when executing stateful input
// sequences. E.g., a user clicks on an object which is handled by a
// relatively low-priority event handler, and wants to get the *next*
// click as well (even if it would ordinarily be consumed by some
// other event handler). The event handler can thus temporarily
// increase its priority.
void viewer_event_handler_set_priority (Viewer *viewer, EventHandler *ehandler, 
        int priority);

void viewer_set_status_bar_message (Viewer *viewer, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

// Render modes are opaque to the viewer; they are simply state that
// is available to the renderers that can be used to modify their
// operation. In DGC, we use this to change coordinate systems between
// global and local frame.
void viewer_add_render_mode(Viewer *viewer, int id, const char *name);

// returns non-zero if an event handler is currently picking
int viewer_picking(Viewer *viewer);

// If a event handler wants to begin a pick operation, it can request it. 
// Returns non-zero if the request fails.
int viewer_request_pick(Viewer *viewer, EventHandler *ehandler);

void viewer_load_preferences (Viewer *viewer, const char *fname);

void viewer_save_preferences (Viewer *viewer, const char *fname);

#ifdef __cplusplus
}
#endif

#endif
