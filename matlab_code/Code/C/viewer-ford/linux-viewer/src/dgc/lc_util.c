#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "lc_util.h"

//#define dbg(...) fprintf (stderr, __VA_ARGS__)
#define dbg(...)

static int
lc_message_ready (GIOChannel *source, GIOCondition cond, lc_t *lc)
{
    lc_handle (lc);
    return TRUE;
}

typedef struct {
    GIOChannel *ioc;
    guint sid;
    lc_t *lc;
} glib_attached_lc_t;

static GHashTable *lc_glib_sources = NULL;
static GStaticMutex lc_glib_sources_mutex = G_STATIC_MUTEX_INIT;

int
lcu_glib_mainloop_attach_lc (lc_t *lc)
{
    g_static_mutex_lock (&lc_glib_sources_mutex);

    if (!lc_glib_sources) {
        lc_glib_sources = g_hash_table_new (g_direct_hash, g_direct_equal);
    }

    if (g_hash_table_lookup (lc_glib_sources, lc)) {
        dbg ("LC %p already attached to mainloop\n", lc);
        g_static_mutex_unlock (&lc_glib_sources_mutex);
        return -1;
    }

    glib_attached_lc_t *galc = 
        (glib_attached_lc_t*) calloc (1, sizeof (glib_attached_lc_t));

    galc->ioc = g_io_channel_unix_new (lc_get_fileno (lc));
    galc->sid = g_io_add_watch (galc->ioc, G_IO_IN, (GIOFunc) lc_message_ready, 
            lc);
    galc->lc = lc;

    dbg ("inserted LC %p into glib mainloop\n", lc);
    g_hash_table_insert (lc_glib_sources, lc, galc);

    g_static_mutex_unlock (&lc_glib_sources_mutex);
    return 0;
}

int
lcu_glib_mainloop_detach_lc (lc_t *lc)
{
    g_static_mutex_lock (&lc_glib_sources_mutex);
    if (!lc_glib_sources) {
        dbg ("no lc glib sources\n");
        g_static_mutex_unlock (&lc_glib_sources_mutex);
        return -1;
    }

    glib_attached_lc_t *galc = 
        (glib_attached_lc_t*) g_hash_table_lookup (lc_glib_sources, lc);

    if (!galc) {
        dbg ("couldn't find matching gaLC\n");
        g_static_mutex_unlock (&lc_glib_sources_mutex);
        return -1;
    }

    dbg ("detaching LC from glib\n");
    g_io_channel_unref (galc->ioc);
    g_source_remove (galc->sid);

    g_hash_table_remove (lc_glib_sources, lc);
    free (galc);

    if (g_hash_table_size (lc_glib_sources) == 0) {
        g_hash_table_destroy (lc_glib_sources);
        lc_glib_sources = NULL;
    }

    g_static_mutex_unlock (&lc_glib_sources_mutex);
    return 0;
}
