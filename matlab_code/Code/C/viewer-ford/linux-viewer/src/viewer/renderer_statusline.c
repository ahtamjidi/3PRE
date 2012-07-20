#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <sys/types.h>
#include <regex.h>

#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <common/timestamp.h>
#include <common/glib_util.h>
#include <lcm/lcm.h>
#include <dgc/globals.h>
#include <glutil/glutil.h>
#include <lcmtypes/lcmtypes_raw_t.h>
#include <lcmtypes/lcmtypes_raw_t.h>

#include <gtk_util/gtk_util.h>

#include "libviewer/viewer.h"

#define RENDERER_NAME "Status Line"

#define PARAM_COMMENTS "Comments"
#define PARAM_LAMBDA "Decay Lambda"
#define PARAM_PROCMAN_WARNINGS "Procman Warnings"

#define DEFAULT_LAMBDA 0.5

typedef struct _RendererStatusline RendererStatusline;

struct _RendererStatusline {
    Renderer renderer;

    GtkuParamWidget    *pw;

    GLUtilConsole *console;

    lcm_t *lc;
    lcmtypes_raw_t_subscription_t *nav_status_handler;
    lcmtypes_raw_t_subscription_t *mm_string_handler;
    lcmtypes_raw_t_subscription_t *logger_status_handler;

    lcmtypes_raw_t *last_nav_string;
    lcmtypes_raw_t *last_logger_string;
    lcmtypes_raw_t *last_mm_string;
    int64_t nav_recv_utime;
    int64_t logger_recv_utime;
    int64_t mm_recv_utime;

    Viewer *viewer;
};

static void
statusline_draw (Viewer *viewer, Renderer *renderer)
{
    RendererStatusline *self = (RendererStatusline*) renderer->user;

    int64_t now = timestamp_now ();
    int64_t valid_time_dev = 30000000;

    char mm_str[120] = "\0";
    if (self->last_mm_string && 
            abs (self->mm_recv_utime - now) < valid_time_dev) {
        strncpy (mm_str, (char*) self->last_mm_string->data,
                MIN (sizeof (mm_str), self->last_mm_string->length));
    }

    char nav_string[120] = "\0";
    if (self->last_nav_string &&
            abs (self->nav_recv_utime - now) < valid_time_dev) {
        strncpy (nav_string, (char*)self->last_nav_string->data, 
                MIN (sizeof (nav_string), self->last_nav_string->length));
    }

    char logger_str[80] = "\0";
    if (self->last_logger_string &&
            abs (self->logger_recv_utime - now) < valid_time_dev) {
        strncpy (logger_str, (char*)self->last_logger_string->data, 
                MIN (sizeof (logger_str), self->last_logger_string->length));
    }

    char status_msg[1024];
    snprintf (status_msg, sizeof (status_msg), 
            "%s %s %s", mm_str, nav_string, logger_str);
    g_strchomp (status_msg);
    if (strlen (status_msg)) {
        double xyz[] = { 0.5, 0, 0 };
        glutil_draw_text (xyz, 0, status_msg, GLUTIL_DRAW_TEXT_DROP_SHADOW |
                          GLUTIL_DRAW_TEXT_ANCHOR_HCENTER |
                          GLUTIL_DRAW_TEXT_ANCHOR_TOP |
                          GLUTIL_DRAW_TEXT_JUSTIFY_CENTER |
                          GLUTIL_DRAW_TEXT_NORMALIZED_SCREEN_COORDINATES);
    }
}

static void
on_nav_string (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_raw_t *msg, void *user)
{
    RendererStatusline *self = user;
    if (self->last_nav_string) 
        lcmtypes_raw_t_destroy (self->last_nav_string);
    self->last_nav_string = lcmtypes_raw_t_copy (msg);
    self->nav_recv_utime = timestamp_now ();
}

static void
on_mission_manager_string (const lcm_recv_buf_t *rbuf, const char *channel, 
        const lcmtypes_raw_t *msg, void *user)
{
    RendererStatusline *self = user;
    if (self->last_mm_string) lcmtypes_raw_t_destroy (self->last_mm_string);
    self->last_mm_string = lcmtypes_raw_t_copy (msg);
    self->mm_recv_utime = timestamp_now ();
}

static void
on_logger_status (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_raw_t *msg, void *user)
{
    RendererStatusline *self = user;
    if (self->last_logger_string) 
        lcmtypes_raw_t_destroy (self->last_logger_string);
    self->last_logger_string = lcmtypes_raw_t_copy (msg);
    self->logger_recv_utime = timestamp_now ();
}

void 
statusline_free (Renderer *renderer) 
{
    RendererStatusline *self = (RendererStatusline*) renderer;
    if (self->last_nav_string) 
        lcmtypes_raw_t_destroy (self->last_nav_string);
    lcmtypes_raw_t_unsubscribe (self->lc, self->nav_status_handler);
    globals_release_lcm (self->lc);
    free (renderer);
}

Renderer *renderer_statusline_new (Viewer *viewer)
{
    RendererStatusline *self = 
        (RendererStatusline*) calloc (1, sizeof (RendererStatusline));
    self->viewer = viewer;
    self->renderer.draw = statusline_draw;
    self->renderer.destroy = statusline_free;
    self->renderer.name = RENDERER_NAME;
    self->renderer.user = self;
    self->renderer.enabled = 1;

    self->lc = globals_get_lcm ();
    self->nav_status_handler = 
        lcmtypes_raw_t_subscribe (self->lc, "NAVIGATOR_STRING", on_nav_string, self);
    self->logger_status_handler = 
        lcmtypes_raw_t_subscribe (self->lc, "LOGGER_STRING", on_logger_status, self);
    self->mm_string_handler = lcmtypes_raw_t_subscribe (self->lc, 
                "MISSION_MANAGER_STRING", on_mission_manager_string, self);

    self->logger_recv_utime = 0;
    self->nav_recv_utime = 0;
    self->mm_recv_utime = 0;

    return &self->renderer;
}

void setup_renderer_statusline (Viewer *viewer, int render_priority)
{
    viewer_add_renderer(viewer, 
            renderer_statusline_new(viewer), render_priority);
}
