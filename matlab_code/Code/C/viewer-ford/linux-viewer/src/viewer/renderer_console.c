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

#include <gtk_util/gtk_util.h>

#include <lcmtypes/lcmtypes_comment_t.h>
#include <lcmtypes/lcmtypes_pmsd_printf_t.h>
#include <lcmtypes/lcmtypes_pmsd_info_t.h>

#include "libviewer/viewer.h"

#define RENDERER_NAME "Console"

#define PARAM_COMMENTS "Comments"
#define PARAM_LAMBDA "Decay Lambda"
#define PARAM_PROCMAN_WARNINGS "Procman Warnings"

#define DEFAULT_LAMBDA 0.5

typedef struct _RendererConsole RendererConsole;

typedef struct  
{
    char *name;
    int32_t id;
    int64_t utime;
} _pm_command_t;

static _pm_command_t * _pm_command_new (const char *name, int id, int64_t utime)
{
    _pm_command_t *pmc = g_slice_new (_pm_command_t);
    pmc->name = strdup (name);
    pmc->id = id;
    pmc->utime = utime;
    return pmc;
}

static void _pm_command_destroy (_pm_command_t *pmc)
{
    free (pmc->name);
    g_slice_free (_pm_command_t, pmc);
}

struct _RendererConsole {
    Renderer renderer;

    GtkuParamWidget    *pw;

    GLUtilConsole *console;

    lcm_t *lc;
    lcmtypes_comment_t_subscription_t *comment_handler;
    lcmtypes_pmsd_printf_t_subscription_t *pmsd_printf_handler;
    lcmtypes_pmsd_info_t_subscription_t *pmsd_info_handler;

    regex_t warnings_regex;

    GHashTable *pm_procs;
    guint upkeep_timer;

    Viewer *viewer;
};

static void
console_draw (Viewer *viewer, Renderer *renderer)
{
    RendererConsole *self = (RendererConsole*) renderer->user;
    glutil_console_render (self->console);
}

static void 
on_param_widget_changed (GtkuParamWidget *pw, const char *name, 
        void *user)
{
    RendererConsole *self = user;
    if (!strcmp (name, PARAM_LAMBDA)) {
        double lambda = gtku_param_widget_get_double (pw, PARAM_LAMBDA);
        glutil_console_set_decay_lambda (self->console, lambda);
    }
    viewer_request_redraw (self->viewer);
}

static void
on_comment (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_comment_t *msg, void *user_data)
{
    RendererConsole *self = user_data;
    if (gtku_param_widget_get_bool (self->pw, PARAM_COMMENTS)) {
        glutil_console_color3f (self->console, 0, 1, 0);
        glutil_console_printf (self->console, msg->comment);
        viewer_request_redraw (self->viewer);
    }
}

static void
on_pmsd_printf (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_pmsd_printf_t *msg, void *user_data)
{
    RendererConsole *self = user_data;
    if (!gtku_param_widget_get_bool (self->pw, PARAM_PROCMAN_WARNINGS)) {
        return;
    }

    if (0 == regexec(&self->warnings_regex, msg->text, 0, NULL, 0)) {
        _pm_command_t *pmc = g_hash_table_lookup (self->pm_procs, 
                GINT_TO_POINTER (msg->sheriff_id));
        if (pmc) {
            glutil_console_color3f (self->console, 1, 1, 1);
            char *text = strdup (msg->text);
            g_strchomp (text);
            glutil_console_printf (self->console, "[%s] %s\n", pmc->name, text);
            free (text);
            pmc->utime = timestamp_now ();
        }
    }
}

static void
on_pmsd_info (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_pmsd_info_t *msg, void *user_data)
{
    RendererConsole *self = user_data;
    int64_t now = timestamp_now ();
    for (int i=0; i<msg->ncmds; i++) {
        const lcmtypes_pmsd_deputy_cmd_t *cmd = &msg->cmds[i];
        void *key = GINT_TO_POINTER (cmd->sheriff_id);
        _pm_command_t *pmc = g_hash_table_lookup (self->pm_procs, key);
        if (!pmc) {
            pmc = _pm_command_new (cmd->name, cmd->sheriff_id, now);
            g_hash_table_insert (self->pm_procs, key, pmc);
        } else if (strcmp (pmc->name, cmd->name)) {
            free (pmc->name);
            pmc->name = strdup (cmd->name);
        }
        pmc->utime = now;
    }
}

static gboolean
upkeep (void *user_data)
{
    RendererConsole *self = user_data;
    GPtrArray *pmcs = gu_hash_table_get_vals (self->pm_procs);
    int64_t now = timestamp_now ();
    int64_t min_keep_utime = now - 60000000;
    for (int i=0; i<pmcs->len; i++) {
        _pm_command_t *pmc = g_ptr_array_index (pmcs, i);
        if (pmc->utime < min_keep_utime) {
            g_hash_table_remove (self->pm_procs, 
                    GINT_TO_POINTER (pmc->id));
        }
    }
    g_ptr_array_free (pmcs, TRUE);
    return TRUE;
}

static void
on_load_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererConsole *self = user_data;
    gtku_param_widget_load_from_key_file (self->pw, keyfile, RENDERER_NAME);
}

static void
on_save_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererConsole *self = user_data;
    gtku_param_widget_save_to_key_file (self->pw, keyfile, RENDERER_NAME);
}

static void
console_free (Renderer *renderer) 
{
    RendererConsole *self = (RendererConsole*) renderer;
    lcmtypes_comment_t_unsubscribe (self->lc, self->comment_handler);
    lcmtypes_pmsd_info_t_unsubscribe (self->lc, self->pmsd_info_handler);
    lcmtypes_pmsd_printf_t_unsubscribe (self->lc, self->pmsd_printf_handler);
    globals_release_lcm (self->lc);
    regfree (&self->warnings_regex);
    g_hash_table_destroy (self->pm_procs);
    g_source_remove (self->upkeep_timer);
    free (renderer);
}

Renderer *renderer_console_new (Viewer *viewer)
{
    RendererConsole *self = 
        (RendererConsole*) calloc (1, sizeof (RendererConsole));
    self->viewer = viewer;
    self->renderer.draw = console_draw;
    self->renderer.destroy = console_free;
    self->renderer.name = RENDERER_NAME;
    self->renderer.user = self;
    self->renderer.enabled = 1;


    self->lc = globals_get_lcm ();
    self->comment_handler = 
        lcmtypes_comment_t_subscribe (self->lc, "COMMENT", on_comment, self);
    self->pmsd_info_handler = 
        lcmtypes_pmsd_info_t_subscribe (self->lc, "PMD_INFO", on_pmsd_info, self);
    self->pmsd_printf_handler = lcmtypes_pmsd_printf_t_subscribe (self->lc, 
            "PMD_PRINTF", on_pmsd_printf, self);

    // setup parameter widget
    self->pw = GTKU_PARAM_WIDGET (gtku_param_widget_new ());
    self->renderer.widget = GTK_WIDGET (self->pw);
    gtk_widget_show GTK_WIDGET (self->pw);

    gtku_param_widget_add_booleans (self->pw, 0, PARAM_COMMENTS, 
            1, NULL);
    gtku_param_widget_add_booleans (self->pw, 0, PARAM_PROCMAN_WARNINGS,
            1, NULL);
    gtku_param_widget_add_double (self->pw, PARAM_LAMBDA, 
            GTKU_PARAM_WIDGET_SLIDER, 0, 5, 0.1, DEFAULT_LAMBDA);

    g_signal_connect (G_OBJECT (self->pw), "changed", 
            G_CALLBACK (on_param_widget_changed), self);

    // setup console text renderer
    self->console = glutil_console_new ();
    glutil_console_set_decay_lambda (self->console, DEFAULT_LAMBDA);

    regcomp (&self->warnings_regex, "warning", REG_ICASE);

    self->pm_procs = g_hash_table_new_full (g_direct_hash, g_direct_equal,
            NULL, (GDestroyNotify) _pm_command_destroy);

    self->upkeep_timer = g_timeout_add (10000, upkeep, self);

    g_signal_connect (G_OBJECT (viewer), "load-preferences", 
            G_CALLBACK (on_load_preferences), self);
    g_signal_connect (G_OBJECT (viewer), "save-preferences",
            G_CALLBACK (on_save_preferences), self);
    return &self->renderer;
}

void setup_renderer_console (Viewer *viewer, int render_priority)
{
    viewer_add_renderer(viewer, renderer_console_new(viewer), render_priority);
}
