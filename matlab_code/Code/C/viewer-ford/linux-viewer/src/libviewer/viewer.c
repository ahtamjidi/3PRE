#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include <zlib.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <gtk/gtk.h>

#include <gtk_util/gtk_util.h>
#include <common/ppm.h>
#include <common/pixels.h>
#include <common/ioutils.h>
#include <common/timestamp.h>
#include <common/getopt.h>
#include <common/glib_util.h>
#include <common/small_linalg.h>

#include "viewer.h"
#include "default_view_handler.h"

//#define dbg(args...) fprintf (stderr, args)
#define dbg(args...) 
#define err(args...) fprintf (stderr, args)

#define MAX_REDRAW_HZ 30

static int g_draws = 0;

enum {
    LOAD_PREFERENCES_SIGNAL,
    SAVE_PREFERENCES_SIGNAL,
    LAST_SIGNAL
};

static guint viewer_signals[LAST_SIGNAL] = { 0 };

static void stop_recording (Viewer *self);
//static gboolean ezavi_timer_callback (void *user_data);

void viewer_request_redraw (Viewer *self)
{
    gtku_gl_drawing_area_invalidate (self->gl_area);
}

#ifndef USE_ZMOV
static void
on_fb_ready (FBGLDrawingArea *fb_area, uint8_t *buffer, void *user_data)
{
    if (!buffer) return;

    ezavi_t *ezavi = (ezavi_t*) user_data;
    ezavi_write_video_bottom_up (ezavi, buffer);
}
#endif

static gboolean
on_render_timer (Viewer * self)
{

#ifdef USE_ZMOV
    self->movie_draw_pending = 1;
    viewer_request_redraw(self);

#else
    if (self->fb_area) {
        fb_gl_drawing_area_begin (self->fb_area);
        render_scene (self);
        fb_gl_drawing_area_end (self->fb_area);
        fb_gl_drawing_area_swap_buffers (self->fb_area);
    }
#endif

    return TRUE;
}

static void 
start_recording (Viewer *self)
{
    if (self->is_recording) {
        err ("viewer: already recording!!\n");
        return;
    }

#ifndef USE_ZMOV
    assert (self->fb_area == NULL);
#endif
    assert (self->movie_buffer == NULL);

    int window_width = GTK_WIDGET (self->gl_area)->allocation.width;
    int window_height = GTK_WIDGET (self->gl_area)->allocation.height;

    self->movie_width = window_width - (window_width % 4);
    self->movie_height = window_height;
    self->movie_stride = self->movie_width * 3;
    self->movie_buffer = (uint8_t*) malloc (self->movie_stride * self->movie_height);
    self->movie_desired_fps = 1000.0 / gtk_spin_button_get_value (GTK_SPIN_BUTTON (self->fps_spin));
    self->movie_actual_fps = self->movie_desired_fps;
    self->movie_frame_last_utime = 0;

/*
    int wwidth, wheight;
    gtk_window_get_size(GTK_WINDOW(self->window), &wwidth, &wheight);
    printf("%5d, %5d\n", self->movie_width, self->movie_height);
    gtk_widget_set_size_request(GTK_WIDGET(self->gl_area), self->movie_width, self->movie_height);
    gtk_window_set_default_size(GTK_WINDOW(self->window), wwidth, wheight);
    gtk_widget_set_size_request(GTK_WIDGET(self->gl_area), self->movie_width, self->movie_height);
    gtk_window_set_resizable(GTK_WINDOW(self->window), FALSE);
*/

#ifdef USE_ZMOV

    self->movie_path = get_unique_filename(NULL, "viewer", 1, "ppms.gz");
    self->movie_gzf = gzopen(self->movie_path, "w");
    gzsetparams(self->movie_gzf, Z_BEST_SPEED, Z_DEFAULT_STRATEGY);

    if (self->movie_gzf == NULL)
        goto abort_recording;
    
    viewer_set_status_bar_message (self, "Recording to: %s", self->movie_path);

#else
    self->fb_area = fb_gl_drawing_area_new (FALSE, mov_width, mov_height, GL_BGR);
    if (!self->fb_area) {
        err ("Couldn't create FramebufferObject offscreen plugin\n");
        gtk_toggle_tool_button_set_active (
                GTK_TOGGLE_TOOL_BUTTON (self->record_button), FALSE);
        free (self->mov_bgr_buf);
        return;
    }

    assert(self->ezavi == NULL);

    ezavi_params_t avi_params = {
        .path = NULL,
        .file_prefix = "viewer",
        .date_in_file = 1,
        .codec = "raw",
        .width = mov_width,
        .height = mov_height,
        .src_stride = mov_width * 3,
        .frame_rate = 30,
        .split_point = 4000000000UL
    };

    self->ezavi = ezavi_new (&avi_params);
    if (!self->ezavi) { 
        err ("Couldn't create AVI file\n");
        goto abort_recording;
    }

    g_signal_connect (G_OBJECT(self->fb_area), "buffer-ready",
            G_CALLBACK (on_fb_ready), self->ezavi);

    viewer_set_status_bar_message (self, "Recording to: %s",
            ezavi_get_filename (self->ezavi));

#endif

    self->render_timer_id = g_timeout_add (1000 /
            gtk_spin_button_get_value (GTK_SPIN_BUTTON (self->fps_spin)),
            (GSourceFunc) on_render_timer, self);
    self->is_recording = 1;
    return;

abort_recording:
#ifndef USE_ZMOV
    g_object_unref (self->fb_area);
#endif
    gtk_toggle_tool_button_set_active (
            GTK_TOGGLE_TOOL_BUTTON (self->record_button),
            FALSE);
    free (self->mov_bgr_buf);
}

static void 
stop_recording (Viewer *self)
{
#ifndef USE_ZMOV
    if (!self->fb_area)
        return;
#endif

    free(self->movie_buffer);
    self->movie_buffer = NULL;

    dbg ("\nRecording stopped\n");
    viewer_set_status_bar_message (self, "Recording stopped");

#ifdef USE_ZMOV
    gzclose(self->movie_gzf);
    self->movie_gzf = NULL;
    free(self->movie_path);
    self->movie_draw_pending = 0;
#else
    fb_gl_drawing_area_flush (self->fb_area);
    ezavi_finish (self->ezavi);
    ezavi_destroy (self->ezavi);
    self->ezavi = NULL;
    g_object_unref (self->fb_area);
    self->fb_area = NULL;
#endif

    g_source_remove (self->render_timer_id);

    self->is_recording = 0;
    gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (self->record_button),
                                       FALSE);
//    gtk_window_set_resizable(GTK_WINDOW(self->window), TRUE);
}

static void update_status_bar(Viewer *viewer)
{
    char buf[1024];
    
    if (viewer->picking_handler && !viewer->picking_handler->picking)
        viewer->picking_handler = NULL;

    int vp[4] = {0,0,0,0};
    if (viewer->gl_area &&
            gtku_gl_drawing_area_set_context (viewer->gl_area) == 0) {
        glGetIntegerv(GL_VIEWPORT, vp);
    }
    int width = vp[2], height = vp[3];

    if (viewer->picking_handler)
        snprintf(buf, sizeof (buf), "%s%d x %d [%s]  %s", 
                (viewer->simulation_flag?"[SIM ENABLED] ":""), width, height, 
                 viewer->picking_handler->name,  viewer->status_bar_message);
    else
        snprintf(buf, sizeof (buf), "%s%d x %d [Idle] %s", 
                (viewer->simulation_flag?"[SIM ENABLED] ":""), width, height,
                 viewer->status_bar_message);

    gtk_statusbar_push(GTK_STATUSBAR(viewer->status_bar), 
                       gtk_statusbar_get_context_id(
                           GTK_STATUSBAR(viewer->status_bar),"info"), buf);
}

void viewer_set_status_bar_message (Viewer *viewer, const char *fmt, ...)
{
    char buf[1024];
    va_list ap;
    va_start (ap, fmt);
    vsnprintf (buf, sizeof (buf), fmt, ap);
    va_end (ap);

    if (viewer->status_bar_message)
        free(viewer->status_bar_message);

    viewer->status_bar_message = strdup(buf);

    update_status_bar(viewer);
}

void
viewer_load_preferences (Viewer *viewer, const char *fname)
{
    GKeyFile *preferences = g_key_file_new ();
    int loaded = 0;
    if (g_file_test (fname, G_FILE_TEST_IS_REGULAR)) {
        loaded = g_key_file_load_from_file (preferences, fname, 0, NULL);
    }

    if (!loaded) goto done;
    dbg ("loading viewer settings from %s\n", fname);
        
    for (int ridx = 0; ridx < viewer->renderers->len; ridx++) {
        Renderer *renderer = g_ptr_array_index (viewer->renderers, ridx);
        GtkCheckMenuItem *cmi = GTK_CHECK_MENU_ITEM (renderer->cmi);

        GError *err = NULL;
        int enabled = g_key_file_get_boolean (preferences,
                "__libviewer_enabled_renderers", renderer->name, &err);

        if (err) {
            err ("%s\n", err->message);
            g_error_free (err);
        } else {
            gtk_check_menu_item_set_active (cmi, enabled);
        }

        if (renderer->widget) {
            if (g_key_file_has_key (preferences, "__libviewer_show_renderers", 
                        renderer->name, NULL)) {
                renderer->expanded = g_key_file_get_boolean (preferences, 
                        "__libviewer_show_renderers", renderer->name, NULL);
                gtk_expander_set_expanded (GTK_EXPANDER (renderer->expander),
                        renderer->expanded);
            }
        }
    }

    g_signal_emit (G_OBJECT(viewer), viewer_signals[LOAD_PREFERENCES_SIGNAL], 0,
            preferences);

done:
    g_key_file_free (preferences);
}

void
viewer_save_preferences (Viewer *viewer, const char *fname)
{
    GKeyFile *preferences = g_key_file_new ();

    dbg ("saving viewer settings...\n");

    for (int ridx = 0; ridx < viewer->renderers->len; ridx++) {
        Renderer *renderer = g_ptr_array_index (viewer->renderers, ridx);

        g_key_file_set_boolean (preferences, "__libviewer_enabled_renderers",
                renderer->name, renderer->enabled);
        if (renderer->widget) {
            g_key_file_set_boolean (preferences, "__libviewer_show_renderers",
                    renderer->name, renderer->expanded);
        }
    }

    FILE *fp = fopen (fname, "w");
    if (!fp) {
        perror ("error saving preferences");
        return;
    }

    g_signal_emit (G_OBJECT (viewer), viewer_signals[SAVE_PREFERENCES_SIGNAL], 
            0, preferences);

    gsize len = 0;
    char *data = g_key_file_to_data (preferences, &len, NULL);
    fwrite (data, len, 1, fp);
    free (data);

    fclose (fp);
    g_key_file_free (preferences);
}

// ============== event handlers ===========

static gboolean
on_gl_configure (GtkWidget *widget, GdkEventConfigure *event, void* user_data)
{
    Viewer * self = (Viewer *) user_data;

    update_status_bar(self); // redraw our window size
    viewer_request_redraw (self);
    return TRUE;
}

static gboolean on_redraw_timer(void *user_data)
{
    Viewer * self = (Viewer *) user_data;
    self->redraw_timer_pending = 0;

    viewer_request_redraw (self);
    return FALSE; // this is a one-shot event.
}

void 
check_gl_errors (const char *label) {
    GLenum errCode = glGetError ();
    const GLubyte *errStr;
    
    while (errCode != GL_NO_ERROR) {
        errStr = gluErrorString(errCode);
        fprintf (stderr, "OpenGL Error on draw #%d (%s)\n", g_draws, label);
        fprintf (stderr, (char*)errStr);
        fprintf (stderr, "\n");
        errCode = glGetError ();
    }
}

static void
render_scene (Viewer *self)
{
//    glClearColor (0.0, 0.0, 0.0, 1.0);
    glClearColor(self->backgroundColor[0], self->backgroundColor[1], 
                 self->backgroundColor[2], self->backgroundColor[3]);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (self->view_handler)
        self->view_handler->update_gl_matrices(self, self->view_handler);

    glMatrixMode (GL_MODELVIEW);

    /* give the ambient light a blue tint to match the blue sky */
    float light0_amb[] = { 0.8, 0.8, 1.0, 1 };
    float light0_dif[] = { 1, 1, 1, 1 };
    float light0_spe[] = { .5, .5, .5, 1 };
    float light0_pos[] = { 100, 100, 100, 0 };
    glLightfv (GL_LIGHT0, GL_AMBIENT, light0_amb);
    glLightfv (GL_LIGHT0, GL_DIFFUSE, light0_dif);
    glLightfv (GL_LIGHT0, GL_SPECULAR, light0_spe);
    glLightfv (GL_LIGHT0, GL_POSITION, light0_pos);
    glEnable (GL_LIGHT0);

    if (self->prettier_flag) {
        glEnable(GL_LINE_STIPPLE);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    }

    for (unsigned int ridx = 0; ridx < g_ptr_array_size(self->renderers); ridx++) {
        Renderer *renderer = g_ptr_array_index(self->renderers, ridx);

        if (renderer->enabled) {
            
            glPushAttrib(GL_ENABLE_BIT | GL_POINT_BIT | GL_POLYGON_STIPPLE_BIT | 
                         GL_POLYGON_BIT | GL_LINE_BIT | GL_FOG_BIT | GL_LIGHTING_BIT );
            glPushMatrix();

            if (renderer->draw)
                renderer->draw (self, renderer);

            check_gl_errors (renderer->name);

            glPopMatrix();
            glPopAttrib();
        }
    }

/*
    glColor3f(1, 1, 1);
    glBegin(GL_POINTS);
    glVertex3f(0,0,0);
    glVertex3f(1,0,0);
    glVertex3f(2,0,0);
    glVertex3f(0,1,0);
    glEnd();
*/
}

static gboolean
on_gl_expose (GtkWidget *widget, GdkEventExpose *event, void *user_data)
{
    Viewer * self = (Viewer *) user_data;

    // if not enough time has elapsed since our last redraw, we
    // schedule a redraw in the future (if one isn't already pending).
    int64_t now = timestamp_now();
    double dt = (now - self->last_draw_utime) / 1000000.0;
    if (dt < (1.0/MAX_REDRAW_HZ)) {
        if (!self->redraw_timer_pending) {
            int delay_ms = (now - self->last_draw_utime)/1000 + 1;
            g_timeout_add(delay_ms, on_redraw_timer, self);
            self->redraw_timer_pending = 1;
        }
        return TRUE;
    }
    self->last_draw_utime = now;

    // If we're making a movie, don't draw any faster than the
    // requested movie FPS rate.
    if (self->movie_gzf && !self->movie_draw_pending)
        return TRUE;

    // set this to 1 in order to cause viewer to exit cleanly after a
    // few hundred frames: useful for generating gprof output.
    g_draws++;
    if (0) {
        int thresh = 300;

        // for profiling PROFILE
        if (g_draws%50 == 0)
            printf("draws: %5i / %i\n", g_draws, thresh);
        if (g_draws == thresh) {
            printf("Profiling is enabled: exiting now\n");
            exit(0);
        }
    }
    
    // we're going to actually draw.
    
    gtku_gl_drawing_area_set_context (self->gl_area);
    render_scene (self);
    gtku_gl_drawing_area_swap_buffers (self->gl_area);
    
    // write a movie frame?
    if (self->movie_draw_pending) {
        assert(self->movie_gzf);

        glReadPixels (0, 0, self->movie_width, self->movie_height, GL_RGB, GL_UNSIGNED_BYTE, self->movie_buffer); 
        
        gzprintf(self->movie_gzf, "P6 %d %d %d\n", self->movie_width, self->movie_height, 255);
        
        for (int h = self->movie_height - 1; h >= 0; h--) {
            int offset = self->movie_stride * h;
            gzwrite(self->movie_gzf, &self->movie_buffer[offset], self->movie_stride);
        }

        self->movie_draw_pending = 0;
        int64_t now = timestamp_now();
        double dt;
        if (self->movie_frame_last_utime == 0)
            dt = 1.0 / self->movie_desired_fps;
        else
            dt = (now - self->movie_frame_last_utime)/1000000.0;
        double fps = 1.0 / dt;
        self->movie_frame_last_utime = now;
        double alpha = 0.8; // higher = lower-pass
        self->movie_actual_fps = alpha * self->movie_actual_fps + (1 - alpha) * fps;
        self->movie_frames++;

        printf("%20s %6d (%5.2f fps)\r", self->movie_path, self->movie_frames, self->movie_actual_fps);
        fflush(NULL);
    }

    return TRUE;
}

static GLint
mygluUnProject (double winx, double winy, double winz,
        const double model[16], const double proj[16], const int viewport[4],
        double * objx, double * objy, double * objz)
{
    double p[16], m[16];
    matrix_transpose_4x4d (proj, p);
    matrix_transpose_4x4d (model, m);
    double t[16];
    matrix_multiply_4x4_4x4 (p, m, t);
    if (matrix_inverse_4x4d (t, m) < 0)
        return GL_FALSE;
    if (viewport[2] == 0 || viewport[3] == 0)
        return GL_FALSE;
    double v[4] = {
        2 * (winx - viewport[0]) / viewport[2] - 1,
        2 * (winy - viewport[1]) / viewport[3] - 1,
        2 * winz - 1,
        1,
    };
    double v2[4];
    matrix_vector_multiply_4x4_4d (m, v, v2);
    if (v2[3] == 0)
        return GL_FALSE;
    *objx = v2[0] / v2[3];
    *objy = v2[1] / v2[3];
    *objz = v2[2] / v2[3];
    return GL_TRUE;
}

static int
_window_coord_to_ray (double x, double y, double ray_start[3], double ray_dir[3])
{
    GLdouble model_matrix[16];
    GLdouble proj_matrix[16];
    GLint viewport[4];

    glGetDoublev (GL_MODELVIEW_MATRIX, model_matrix);
    glGetDoublev (GL_PROJECTION_MATRIX, proj_matrix);
    glGetIntegerv (GL_VIEWPORT, viewport);

    if (mygluUnProject (x, y, 0,
                      model_matrix, proj_matrix, viewport, 
                      &ray_start[0], &ray_start[1], &ray_start[2]) == GL_FALSE) 
        return -1;
    
    double ray_end[3];

    if (mygluUnProject (x, y, 1,
                      model_matrix, proj_matrix, viewport, 
                      &ray_end[0], &ray_end[1], &ray_end[2]) == GL_FALSE) 
        return -1;

    ray_dir[0] = ray_end[0] - ray_start[0];
    ray_dir[1] = ray_end[1] - ray_start[1];
    ray_dir[2] = ray_end[2] - ray_start[2];
            
    vector_normalize_3d(ray_dir);

    return 0;
}

static gboolean 
on_button_press (GtkWidget *widget, GdkEventButton *event, void *user_data)
{
    Viewer *self = (Viewer*) user_data;

    gtku_gl_drawing_area_set_context (self->gl_area);
    double ray_start[3];
    double ray_dir[3];
    _window_coord_to_ray (event->x, widget->allocation.height - event->y, ray_start, ray_dir);

    // find a new picker?
    double best_distance = HUGE;
    EventHandler *best_handler = NULL;

    if (self->picking_handler == NULL || self->picking_handler->picking==0) {
        for (unsigned int eidx = 0; eidx < g_ptr_array_size(self->event_handlers); eidx++) {
            EventHandler *handler = g_ptr_array_index(self->event_handlers, eidx);
            
            if (handler->enabled && handler->pick_query) {
                double this_distance = handler->pick_query(self, handler, ray_start, ray_dir);
                if (this_distance < best_distance && this_distance >= 0) {
                    best_distance = this_distance;
                    best_handler = handler;
                }
            }
        }

        // notify the new handler
        if (best_handler)
            viewer_request_pick(self, best_handler);
    }

    // give picking handler first dibs
    int consumed = 0;
    if (self->picking_handler && !self->picking_handler->picking)
        self->picking_handler = NULL;
    if (self->picking_handler && self->picking_handler->enabled && self->picking_handler->mouse_press) {
        consumed = self->picking_handler->mouse_press(self, self->picking_handler, ray_start, ray_dir, event);
        update_status_bar(self);
    }

    // try all the other handlers in order of priority
    for (unsigned int eidx = 0; !consumed && eidx < g_ptr_array_size(self->event_handlers); eidx++) {
        EventHandler *handler = g_ptr_array_index(self->event_handlers, eidx);
        
        if (handler != self->picking_handler && handler->enabled && handler->mouse_press)
            if (handler->mouse_press(self, handler, ray_start, ray_dir, event))
                break;
    }

    return TRUE;
}

static gboolean
on_button_release (GtkWidget *widget, GdkEventButton *event, void *user_data)
{
    Viewer *self = (Viewer*) user_data;

    gtku_gl_drawing_area_set_context (self->gl_area);
    double ray_start[3];
    double ray_dir[3];
    _window_coord_to_ray (event->x, widget->allocation.height - event->y, ray_start, ray_dir);

    // give picking handler first dibs
    int consumed = 0;
    if (self->picking_handler && !self->picking_handler->picking)
        self->picking_handler = NULL;
    if (self->picking_handler && self->picking_handler->enabled && self->picking_handler->mouse_release) {
        consumed = self->picking_handler->mouse_release(self, self->picking_handler, ray_start, ray_dir, event);
        update_status_bar(self);
    }

    // try all the other handlers in order of priority
    for (unsigned int eidx = 0; !consumed && eidx < g_ptr_array_size(self->event_handlers); eidx++) {
        EventHandler *handler = g_ptr_array_index(self->event_handlers, eidx);
        
        if (handler != self->picking_handler && handler->enabled && handler->mouse_release)
            if (handler->mouse_release(self, handler, ray_start, ray_dir, event))
                break;
    }

    return TRUE;
}

static gboolean 
on_motion_notify (GtkWidget *widget, GdkEventMotion *event, void *user_data)
{
    Viewer *self = (Viewer*) user_data;

    gtku_gl_drawing_area_set_context (self->gl_area);
    double ray_start[3];
    double ray_dir[3];
    _window_coord_to_ray (event->x, widget->allocation.height - event->y, ray_start, ray_dir);
    
    // is anyone hovering?
    if (self->picking_handler == NULL || !self->picking_handler->picking) {

        // find a new hover?
        double best_distance = HUGE;
        EventHandler *best_handler = NULL;
        
        for (unsigned int eidx = 0; eidx < g_ptr_array_size(self->event_handlers); eidx++) {
            EventHandler *handler = g_ptr_array_index(self->event_handlers, eidx);
            
            handler->hovering = 0;

            if (handler->enabled && handler->hover_query) {
                double this_distance = handler->hover_query(self, handler, ray_start, ray_dir);
                if (this_distance < best_distance && this_distance >= 0) {
                    best_distance = this_distance;
                    best_handler = handler;
                }
            }
        }

        // notify the new handler
        if (best_handler)
            best_handler->hovering = 1;

        viewer_request_redraw(self);
    }

    // give picking handler first dibs
    int consumed = 0;
    if (self->picking_handler && !self->picking_handler->picking)
        self->picking_handler = NULL;
    if (self->picking_handler && self->picking_handler->enabled && self->picking_handler->mouse_motion) {
        consumed = self->picking_handler->mouse_motion(self, self->picking_handler, ray_start, ray_dir, event);
        update_status_bar(self);
    }

    // try all the other handlers in order of priority
    for (unsigned int eidx = 0; !consumed && eidx < g_ptr_array_size(self->event_handlers); eidx++) {
        EventHandler *handler = g_ptr_array_index(self->event_handlers, eidx);
        
        if (handler != self->picking_handler && handler->enabled && handler->mouse_motion)
            if (handler->mouse_motion(self, handler, ray_start, ray_dir, event))
                break;
    }

    return TRUE;
}

static gboolean 
on_scroll_notify (GtkWidget *widget, GdkEventScroll *event, void *user_data)
{
    Viewer *self = (Viewer*) user_data;

    gtku_gl_drawing_area_set_context (self->gl_area);
    double ray_start[3];
    double ray_dir[3];
    _window_coord_to_ray (event->x, widget->allocation.height - event->y, ray_start, ray_dir);
    
    // give picking handler first dibs
    int consumed = 0;
    if (self->picking_handler && !self->picking_handler->picking)
        self->picking_handler = NULL;
    if (self->picking_handler && self->picking_handler->enabled && self->picking_handler->mouse_scroll) {
        consumed = self->picking_handler->mouse_scroll(self, self->picking_handler, ray_start, ray_dir, event);
        update_status_bar(self);
    }

    // try all the other handlers in order of priority
    for (unsigned int eidx = 0; !consumed && eidx < g_ptr_array_size(self->event_handlers); eidx++) {
        EventHandler *handler = g_ptr_array_index(self->event_handlers, eidx);
        
        if (handler != self->picking_handler && handler->enabled && handler->mouse_scroll)
            if (handler->mouse_scroll(self, handler, ray_start, ray_dir, event))
                break;
    }

    return TRUE;
}

static gint 
on_main_window_key_press_event (GtkWidget *widget, GdkEventKey *event, 
                                void *user)
{
    Viewer *self = (Viewer*) user;

    // give picking handler first dibs
    int consumed = 0;
    if (self->picking_handler && self->picking_handler->enabled && self->picking_handler->picking &&
        self->picking_handler->key_press) {
        consumed = self->picking_handler->key_press(self, self->picking_handler, event);
        update_status_bar(self);
    }

    // try all the other handlers in order of priority
    for (unsigned int eidx = 0; !consumed && eidx < g_ptr_array_size(self->event_handlers); eidx++) {
        EventHandler *handler = g_ptr_array_index(self->event_handlers, eidx);
        
        if (handler != self->picking_handler && handler->enabled && handler->key_press) {
            consumed = handler->key_press(self, handler, event);
            if (consumed)
                break;
        }
    }

    return consumed;
}

static gboolean
take_screenshot (void *user_data, char *fname)
{
    Viewer *self = (Viewer*) user_data;
    int w = GTK_WIDGET (self->gl_area)->allocation.width;
    int h = GTK_WIDGET (self->gl_area)->allocation.height;
    uint8_t *bgra = (uint8_t*)malloc (w*h*4);
    uint8_t *rgb = (uint8_t*)malloc (w*h*3);
    glReadPixels (0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, bgra); 
    FILE *fp = fopen (fname, "wb");
    if (! fp) {
        perror ("fopen");
        err ("couldn't take screenshot\n");
    return FALSE;
    } else {
        pixel_convert_8u_bgra_to_8u_rgb (rgb, w*3, w, h, bgra, w*4);
        ppm_write_bottom_up (fp, rgb, w, h, w*3);
        fclose (fp);

        dbg ("screenshot saved to %s\n", fname);
        viewer_set_status_bar_message (self, "screenshot saved to %s", fname);
    }
    free (bgra);
    free (rgb);
    return TRUE;
}

static void
on_screenshot_clicked (GtkToolButton *ssbt, void *user_data)
{
    char * fname = get_unique_filename (NULL, "viewer", 1, "ppm");
    take_screenshot (user_data, fname);
    free (fname);
}

/*
static gboolean
on_screenshot_timer (Viewer * self)
{
    return take_screenshot (self, "viewer.ppm");
}
*/

static void
on_record_toggled (GtkToggleToolButton *tb, void *user_data)
{
    Viewer *self = (Viewer*) user_data;
    int record = gtk_toggle_tool_button_get_active (tb);

    if (record && ! self->is_recording) {
        start_recording (self);
    } else {
        stop_recording (self);
    }
}


static void
on_renderer_enabled_toggled (GtkCheckMenuItem *cmi, void *user_data)
{
    Viewer *self = (Viewer*) user_data;
    Renderer *r = (Renderer*) g_object_get_data (G_OBJECT (cmi), 
                                                 "Viewer:plugin");
    r->enabled = gtk_check_menu_item_get_active (cmi);

    if (r->widget) {
        GtkWidget *frame = g_object_get_data (G_OBJECT (r->widget), 
                                              "Viewer:frame");
        if (frame) {
            if (r->enabled) {
                gtk_widget_show (frame);
            } else {
                gtk_widget_hide (frame);
            }
        }
    }
    viewer_request_redraw (self);
}

static void
on_event_handler_enabled_toggled (GtkCheckMenuItem *cmi, void *user_data)
{
    Viewer *self = (Viewer*) user_data;
    EventHandler *ehandler = (EventHandler*) g_object_get_data (G_OBJECT(cmi), "Viewer:plugin");

    ehandler->enabled = gtk_check_menu_item_get_active (cmi);

    viewer_request_redraw (self);
}

static void
on_mode_change(GtkMenuItem *mi, void *user)
{
    Viewer *viewer = (Viewer*) user;

    for (unsigned int i = 0; i < g_ptr_array_size(viewer->modes); i++) {
        struct ViewerMode *vm = (struct ViewerMode*) g_ptr_array_index(viewer->modes, i);

        if (vm->menu_item == mi) {
            viewer->mode = i;
            return;
        }
    }

    assert(0);
}

static void
on_select_all_event_handlers_activate (GtkMenuItem *mi, void *user_data)
{
    Viewer *self = (Viewer*) user_data;

    for (unsigned int eidx = 0; eidx < g_ptr_array_size(self->event_handlers); eidx++) {
        EventHandler *ehandler = g_ptr_array_index(self->event_handlers, eidx);
        ehandler->enabled = 1;
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(ehandler->cmi), ehandler->enabled);
    }
}

static void
on_select_no_event_handlers_activate (GtkMenuItem *mi, void *user_data)
{
    Viewer *self = (Viewer*) user_data;

    for (unsigned int eidx = 0; eidx < g_ptr_array_size(self->event_handlers); eidx++) {
        EventHandler *ehandler = g_ptr_array_index(self->event_handlers, eidx);
        ehandler->enabled = 0;
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(ehandler->cmi), ehandler->enabled);
    }
}

static void
on_select_all_renderers_activate (GtkMenuItem *mi, void *user_data)
{
    Viewer *self = (Viewer*) user_data;

    for (unsigned int ridx = 0; ridx < g_ptr_array_size(self->renderers); ridx++) {
        Renderer *renderer = g_ptr_array_index(self->renderers, ridx);

        renderer->enabled = 1;
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(renderer->cmi), renderer->enabled);
    }
}

static void
on_select_no_renderers_activate (GtkMenuItem *mi, void *user_data)
{
    Viewer *self = (Viewer*) user_data;

    for (unsigned int ridx = 0; ridx < g_ptr_array_size(self->renderers); ridx++) {
        Renderer *renderer = g_ptr_array_index(self->renderers, ridx);

        renderer->enabled = 0;
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(renderer->cmi), renderer->enabled);
    }
}

static void
on_renderer_widget_expander_notify (GObject *object, GParamSpec *param_spec, 
        void *user_data)
{
    GtkExpander *expander = GTK_EXPANDER (object);
    Renderer *renderer = 
        (Renderer*) g_object_get_data (G_OBJECT (expander), "Viewer:plugin");

    renderer->expanded = gtk_expander_get_expanded (expander);
    if (renderer->expanded)
        gtk_widget_show (renderer->widget);
    else
        gtk_widget_hide (renderer->widget);
}


// ================

static gint renderer_name_compare_function(gconstpointer _a, gconstpointer _b)
{
    Renderer *ra = *(Renderer**) _a;
    Renderer *rb = *(Renderer**) _b;

    return strcmp(ra->name, rb->name);
}

// sort in decending order of priority
static gint sort_renderers_priority_decreasing(gconstpointer _a, gconstpointer _b)
{
    Renderer *a = *(Renderer**) _a;
    Renderer *b = *(Renderer**) _b;

    return (a->priority < b->priority) ? 1 : -1;
}

void viewer_add_renderer (Viewer *self, Renderer *renderer, int priority)
{
    renderer->priority = priority;
    renderer->cmi      = gtk_check_menu_item_new_with_label (renderer->name);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (renderer->cmi), renderer->enabled);

    g_object_set_data (G_OBJECT (renderer->cmi), "Viewer:plugin", renderer);
    g_signal_connect (G_OBJECT (renderer->cmi), "toggled", 
                      G_CALLBACK (on_renderer_enabled_toggled), self);
    
    g_ptr_array_add(self->renderers, renderer);
    g_ptr_array_add(self->renderers_sorted, renderer);
    g_ptr_array_sort(self->renderers_sorted, renderer_name_compare_function);

    // What position in the sorted array is this item?
    unsigned int menu_idx = g_ptr_array_find_index(self->renderers_sorted, renderer);

    // add the menu item, accounting for the tear-off tab at index 0
    gtk_menu_shell_insert (GTK_MENU_SHELL (self->renderers_menu), renderer->cmi, menu_idx + 1);
    gtk_widget_show (renderer->cmi);

    // create a control widget
    if (renderer->widget) {
        g_ptr_array_add(self->renderers_sorted_with_controls, renderer);
        g_ptr_array_sort(self->renderers_sorted_with_controls, renderer_name_compare_function);

        unsigned int control_idx = g_ptr_array_find_index(self->renderers_sorted_with_controls, renderer);

        renderer->expander = gtk_expander_new (renderer->name);
        gtk_expander_set_expanded (GTK_EXPANDER (renderer->expander), TRUE);
        renderer->control_frame = gtk_frame_new (NULL);
        
        if (renderer->enabled) {
            gtk_widget_show (renderer->control_frame);
        } else {
            gtk_widget_hide (renderer->control_frame);
        }
        
        gtk_frame_set_label_widget (GTK_FRAME (renderer->control_frame), renderer->expander);
        gtk_container_add (GTK_CONTAINER (renderer->control_frame), renderer->widget);
        
        gtk_box_pack_start (GTK_BOX (self->controls_box), renderer->control_frame,
                            FALSE, TRUE, 0);
        gtk_box_reorder_child (GTK_BOX (self->controls_box), renderer->control_frame, control_idx);

        gtk_widget_show (renderer->expander);
        gtk_widget_show (renderer->widget);
        
        g_signal_connect (G_OBJECT (renderer->expander), "notify::expanded",
                          G_CALLBACK (on_renderer_widget_expander_notify), self);
        g_object_set_data (G_OBJECT (renderer->expander), 
                           "Viewer:plugin", renderer);
        g_object_set_data (G_OBJECT (renderer->widget), 
                           "Viewer:expander", renderer->expander);
        g_object_set_data (G_OBJECT (renderer->widget), 
                           "Viewer:frame", renderer->control_frame);
    }

    g_ptr_array_sort(self->renderers, sort_renderers_priority_decreasing);
}

/*
static void
destroy_plugins (Viewer *self)
{
    for (unsigned int ridx = 0; ridx < g_ptr_array_size(self->renderers); ridx++) {
        Renderer *renderer = g_ptr_array_index(self->renderers, ridx);

        if (renderer->destroy)
            renderer->destroy(renderer);
    }

    g_ptr_array_free(self->plugins, TRUE);
}
*/

void make_menus(Viewer *viewer, GtkWidget *parent)
{
    GtkWidget *menubar = gtk_menu_bar_new();
    viewer->menu_bar = menubar;

    gtk_box_pack_start(GTK_BOX(parent), menubar, FALSE, FALSE, 0);
    
    GtkWidget *file_menuitem = gtk_menu_item_new_with_mnemonic("_File");
    gtk_menu_bar_append(GTK_MENU_BAR(menubar), file_menuitem);

    viewer->file_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menuitem), viewer->file_menu);

    /////////////////////
    GtkWidget *renderers_menuitem = gtk_menu_item_new_with_mnemonic("_Renderers");
    gtk_menu_bar_append(GTK_MENU_BAR(menubar), renderers_menuitem);

    viewer->renderers_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(renderers_menuitem), viewer->renderers_menu);

    if (1) {
        // tearoff
        GtkWidget *tearoff = gtk_tearoff_menu_item_new();
        gtk_menu_append (GTK_MENU(viewer->renderers_menu), tearoff);
        gtk_widget_show (tearoff);

        // separator
        GtkWidget *sep = gtk_separator_menu_item_new ();
        gtk_menu_append (GTK_MENU(viewer->renderers_menu), sep);
        gtk_widget_show (sep);

        // select all item
        GtkWidget *select_all_mi = gtk_menu_item_new_with_mnemonic ("Select _All");
        gtk_menu_append (GTK_MENU(viewer->renderers_menu), select_all_mi);
        gtk_widget_show (select_all_mi);
        
        // remove all item
        GtkWidget *select_none_mi = 
            gtk_menu_item_new_with_mnemonic ("Select _None");
        gtk_menu_append (GTK_MENU(viewer->renderers_menu), select_none_mi);
        gtk_widget_show (select_none_mi);
        g_signal_connect (G_OBJECT (select_all_mi), "activate", 
                          G_CALLBACK (on_select_all_renderers_activate), viewer);
        g_signal_connect (G_OBJECT (select_none_mi), "activate",
                          G_CALLBACK (on_select_no_renderers_activate), viewer);
    }

    /////////////////////
    GtkWidget *event_handlers_menuitem = gtk_menu_item_new_with_mnemonic("_Input");
    gtk_menu_bar_append(GTK_MENU_BAR(menubar), event_handlers_menuitem);

    viewer->event_handlers_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(event_handlers_menuitem), viewer->event_handlers_menu);

    if (1) {
        // tearoff
        GtkWidget *tearoff = gtk_tearoff_menu_item_new();
        gtk_menu_append (GTK_MENU(viewer->event_handlers_menu), tearoff);
        gtk_widget_show (tearoff);

        // separator
        GtkWidget *sep = gtk_separator_menu_item_new ();
        gtk_menu_append (GTK_MENU(viewer->event_handlers_menu), sep);
        gtk_widget_show (sep);

        // select all item
        GtkWidget *select_all_mi = gtk_menu_item_new_with_mnemonic ("Select _All");
        gtk_menu_append (GTK_MENU(viewer->event_handlers_menu), select_all_mi);
        gtk_widget_show (select_all_mi);
        
        // remove all item
        GtkWidget *select_none_mi = 
            gtk_menu_item_new_with_mnemonic ("Select _None");
        gtk_menu_append (GTK_MENU(viewer->event_handlers_menu), select_none_mi);
        gtk_widget_show (select_none_mi);
        g_signal_connect (G_OBJECT (select_all_mi), "activate", 
                          G_CALLBACK (on_select_all_event_handlers_activate), viewer);
        g_signal_connect (G_OBJECT (select_none_mi), "activate",
                          G_CALLBACK (on_select_no_event_handlers_activate), viewer);
    }

    /////////////////////
    GtkWidget *mode_menuitem = gtk_menu_item_new_with_mnemonic("_Mode");
    gtk_menu_bar_append(GTK_MENU_BAR(menubar), mode_menuitem);
    viewer->mode_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mode_menuitem), viewer->mode_menu);
}

void make_toolbar(Viewer *viewer, GtkWidget *parent)
{
    GtkWidget *toolbar = gtk_toolbar_new();
    gtk_box_pack_start(GTK_BOX(parent), toolbar, FALSE, FALSE, 0);

    // add a recording button to the toolbar
    viewer->record_button = GTK_WIDGET(
        gtk_toggle_tool_button_new_from_stock (GTK_STOCK_MEDIA_RECORD));
    gtk_tool_item_set_is_important (GTK_TOOL_ITEM (viewer->record_button), 
            TRUE);
    gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (viewer->record_button), 
            viewer->tips,
            "Record an AVI of the viewport, saved in the current directory",
            NULL);

    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), 
            GTK_TOOL_ITEM (viewer->record_button), 0);
    gtk_widget_show (viewer->record_button);
    g_signal_connect (G_OBJECT (viewer->record_button), "toggled", 
            G_CALLBACK (on_record_toggled), viewer);

    // screenshot button
    GtkToolItem *ssbt = gtk_tool_button_new_from_stock (GTK_STOCK_FLOPPY);
    gtk_tool_button_set_label (GTK_TOOL_BUTTON (ssbt), "Screenshot");
    gtk_tool_item_set_is_important (GTK_TOOL_ITEM (ssbt), TRUE);
    gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (ssbt), viewer->tips,
            "Save a PPM screenshot of the viewport to the current directory",
            NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), ssbt, 1);
    gtk_widget_show (GTK_WIDGET (ssbt));
    g_signal_connect (G_OBJECT (ssbt), "clicked", 
            G_CALLBACK (on_screenshot_clicked), viewer);

    // quit button
    GtkToolItem *quitbt = gtk_tool_button_new_from_stock (GTK_STOCK_QUIT);
    gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (quitbt), viewer->tips,
            "Quit", NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), quitbt, 2);
    gtk_widget_show (GTK_WIDGET (quitbt));
    g_signal_connect (G_OBJECT (quitbt), "clicked", gtk_main_quit, NULL);

    GtkToolItem * sep = gtk_separator_tool_item_new ();
    gtk_widget_show (GTK_WIDGET (sep));
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), sep, 3);

    GtkWidget * hbox = gtk_hbox_new (FALSE, 5);
    GtkWidget * label = gtk_label_new ("Record FPS");
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    viewer->fps_spin = gtk_spin_button_new_with_range (0.1, 120.0, 1.0);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (viewer->fps_spin), 1);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (viewer->fps_spin), 30);
    gtk_box_pack_start (GTK_BOX (hbox), viewer->fps_spin, FALSE, FALSE, 0);

    GtkToolItem * toolitem = gtk_tool_item_new ();
    gtk_container_add (GTK_CONTAINER (toolitem), hbox);
    gtk_widget_show_all (GTK_WIDGET (toolitem));
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), toolitem, 4);
    viewer->toolbar = GTK_TOOLBAR (toolbar);
}

static void viewer_class_init (ViewerClass *klass);

G_DEFINE_TYPE (Viewer, viewer, G_TYPE_OBJECT);

static void
viewer_finalize (GObject *obj)
{
    dbg ("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
    Viewer *self = VIEWER (obj);
    gtk_widget_destroy (GTK_WIDGET (self->window));

    G_OBJECT_CLASS (viewer_parent_class)->finalize(obj);
}

static void
viewer_class_init (ViewerClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = viewer_finalize;
    /**
     * Viewer::load-preferences
     * @keyfile: a GKeyFile* to be used for loading saved preferences
     *
     * The load-preferences signal is emitted when viewer_load_preferences is
     * called on the viewer.  Objects can subscribe to this signal to load data
     * from the GKeyFile.
     */
    viewer_signals[LOAD_PREFERENCES_SIGNAL] = g_signal_new("load-preferences",
            G_TYPE_FROM_CLASS(klass),
            G_SIGNAL_RUN_FIRST,
            0, NULL, NULL, g_cclosure_marshal_VOID__POINTER,
            G_TYPE_NONE, 1, G_TYPE_POINTER);
    /**
     * Viewer::save-preferences
     * @keyfile: a GKeyFile* to be used for saving preferences
     *
     * The save-preferences signal is emitted when viewer_save_preferences is
     * called on the viewer.  Objects can subscribe to this signal to save data
     * to the GKeyFile.
     */
    viewer_signals[SAVE_PREFERENCES_SIGNAL] = g_signal_new("save-preferences",
            G_TYPE_FROM_CLASS(klass),
            G_SIGNAL_RUN_FIRST,
            0, NULL, NULL, g_cclosure_marshal_VOID__POINTER,
            G_TYPE_NONE, 1, G_TYPE_POINTER);
}

static void
viewer_init (Viewer *viewer)
{
    viewer->renderers = g_ptr_array_new();
    viewer->renderers_sorted = g_ptr_array_new();
    viewer->renderers_sorted_with_controls = g_ptr_array_new();
    viewer->event_handlers = g_ptr_array_new();
    viewer->event_handlers_sorted = g_ptr_array_new();
    viewer->modes = g_ptr_array_new();

    viewer->prettier_flag = (getenv("DGC_VIEWER_PRETTIER") != NULL && 
                             atoi(getenv("DGC_VIEWER_PRETTIER"))>0);;
    printf("DGC_VIEWER_PRETTIER: %d\n", viewer->prettier_flag);

    viewer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(viewer->window), "Viewer");
    gtk_window_set_resizable(GTK_WINDOW(viewer->window), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(viewer->window), 800, 540);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(viewer->window), vbox);
    
    make_menus(viewer, vbox);

    viewer->tips = gtk_tooltips_new ();
    make_toolbar(viewer, vbox);

    GtkWidget *hpaned = gtk_hpaned_new();
    gtk_box_pack_start(GTK_BOX(vbox), hpaned, TRUE, TRUE, 0);

    GtkWidget *gl_box = gtk_event_box_new();
    gtk_paned_pack1(GTK_PANED(hpaned), gl_box, TRUE, TRUE);

    GtkWidget *controls_align = gtk_alignment_new(.5, .5, 1, 1);
    gtk_paned_pack2(GTK_PANED(hpaned), controls_align, FALSE, TRUE);

    gtk_paned_set_position(GTK_PANED(hpaned), 560);

    GtkWidget *controls_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(controls_align), controls_scroll);

    GtkWidget *controls_view = gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(controls_scroll), controls_view);

    viewer->controls_box = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(controls_view), viewer->controls_box);

    viewer->status_bar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(vbox), viewer->status_bar, FALSE, FALSE, 0);
    viewer_set_status_bar_message(viewer, "Ready");

    // create the aspect area to maintain a 1:1 aspect ratio
    viewer->gl_area = GTKU_GL_DRAWING_AREA (gtku_gl_drawing_area_new (FALSE));
    gtk_widget_set_events (GTK_WIDGET (viewer->gl_area), 
            GDK_LEAVE_NOTIFY_MASK |
            GDK_BUTTON_PRESS_MASK | 
            GDK_BUTTON_RELEASE_MASK | 
            GDK_POINTER_MOTION_MASK);

    gtk_container_add (GTK_CONTAINER (gl_box), GTK_WIDGET (viewer->gl_area));
    gtk_widget_show (GTK_WIDGET (viewer->gl_area));

    g_signal_connect (G_OBJECT (viewer->gl_area), "configure-event",
            G_CALLBACK (on_gl_configure), viewer);
    g_signal_connect (G_OBJECT (viewer->gl_area), "expose-event",
            G_CALLBACK (on_gl_expose), viewer);
    g_signal_connect (G_OBJECT (viewer->gl_area), "button-press-event",
            G_CALLBACK (on_button_press), viewer);
    g_signal_connect (G_OBJECT (viewer->gl_area), "button-release-event",
            G_CALLBACK (on_button_release), viewer);
    g_signal_connect (G_OBJECT (viewer->gl_area), "motion-notify-event",
            G_CALLBACK (on_motion_notify), viewer);
    g_signal_connect (G_OBJECT (viewer->gl_area), "scroll-event",
            G_CALLBACK (on_scroll_notify), viewer);

    g_signal_connect (G_OBJECT (viewer->window), "key_press_event",
		      G_CALLBACK (on_main_window_key_press_event), viewer);

    g_signal_connect (G_OBJECT (viewer->window), "delete_event",
		      G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect (G_OBJECT (viewer->window), "delete_event",
            G_CALLBACK (gtk_widget_hide_on_delete), NULL);

    g_signal_connect (G_OBJECT (viewer->window), "destroy_event",
		      G_CALLBACK(gtk_main_quit), NULL);

    ////////////////////////////////////////////////////////////////////
    // Create plugins menu


    // plugins will be inserted here as add_plugin is called
 
    gtk_widget_show_all(viewer->window);

    DefaultViewHandler *dvh = default_view_handler_new(viewer);
    viewer->default_view_handler = &dvh->vhandler;

    viewer_request_redraw(viewer);
}

Viewer *viewer_new (const char *window_title)
{
    Viewer *viewer = VIEWER (g_object_new (TYPE_VIEWER, NULL));
    return viewer;
}

void viewer_unref(Viewer *viewer)
{
    g_object_unref (viewer);
}

void viewer_set_window_title (Viewer *viewer, const char *window_title)
{
    gtk_window_set_title(GTK_WINDOW(viewer->window), window_title);
}

void viewer_set_view_handler(Viewer *viewer, ViewHandler *vhandler)
{
    if (vhandler) {
        viewer->view_handler = vhandler;
    } else {
        viewer->view_handler = viewer->default_view_handler;
    }
}

// sort in decending order of priority
static gint sort_ehandler_priority_decreasing(gconstpointer _a, gconstpointer _b)
{
    EventHandler *a = *(EventHandler**) _a;
    EventHandler *b = *(EventHandler**) _b;

    return (a->priority < b->priority) ? 1 : -1;
}

// sort in decending order of priority
static gint sort_ehandler_alphabetical(gconstpointer _a, gconstpointer _b)
{
    EventHandler *a = *(EventHandler**) _a;
    EventHandler *b = *(EventHandler**) _b;

    return strcmp(a->name, b->name);
}

void viewer_add_event_handler(Viewer *viewer, EventHandler *ehandler, int priority)
{
    g_ptr_array_add(viewer->event_handlers, ehandler);
    viewer_event_handler_set_priority(viewer, ehandler, priority);

    g_ptr_array_add(viewer->event_handlers_sorted, ehandler);
    g_ptr_array_sort(viewer->event_handlers_sorted, sort_ehandler_alphabetical);

    ehandler->cmi = gtk_check_menu_item_new_with_label(ehandler->name);
    g_object_set_data (G_OBJECT (ehandler->cmi), "Viewer:plugin", ehandler);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (ehandler->cmi), ehandler->enabled);

    g_signal_connect(G_OBJECT(ehandler->cmi), "toggled",
                     G_CALLBACK(on_event_handler_enabled_toggled), viewer);

    // What position in the sorted array is this item?
    unsigned int menu_idx = g_ptr_array_find_index(viewer->event_handlers_sorted, ehandler);

    // add the menu item, accounting for the tear-off tab at index 0
    gtk_menu_shell_insert (GTK_MENU_SHELL (viewer->event_handlers_menu), ehandler->cmi, menu_idx + 1);
    gtk_widget_show (ehandler->cmi);
}

void viewer_event_handler_set_priority(Viewer *viewer, EventHandler *ehandler, int priority)
{
    ehandler->priority = priority;
    g_ptr_array_sort(viewer->event_handlers, sort_ehandler_priority_decreasing);
}

void viewer_add_render_mode(Viewer *viewer, int mode, const char *name)
{
    struct ViewerMode *m = (struct ViewerMode*) calloc(1, sizeof(struct ViewerMode));
    m->mode      = mode;
    m->name      = strdup(name);
    m->menu_item = GTK_MENU_ITEM(gtk_radio_menu_item_new_with_label(viewer->modes_group, m->name));

    g_ptr_array_add(viewer->modes, m);

    gtk_menu_shell_append(GTK_MENU_SHELL (viewer->mode_menu), GTK_WIDGET(m->menu_item));

    if (g_ptr_array_size(viewer->modes) == 1)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(m->menu_item), TRUE);

    viewer->modes_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(m->menu_item));
    gtk_widget_show(GTK_WIDGET(m->menu_item));
    g_signal_connect(G_OBJECT(m->menu_item), "activate",
		     G_CALLBACK (on_mode_change), viewer);
}

int viewer_picking(Viewer *viewer)
{
    return viewer->picking_handler && viewer->picking_handler->picking;
}

int viewer_request_pick(Viewer *viewer, EventHandler *ehandler)
{
    if (viewer_picking(viewer) && viewer->picking_handler != ehandler)
        return -1;

    ehandler->picking = 1;
    viewer->picking_handler = ehandler;
    update_status_bar(viewer);
    return 0;
}

