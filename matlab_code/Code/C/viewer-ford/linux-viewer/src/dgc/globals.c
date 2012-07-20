#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include <glib.h>

#include <common/mdf.h>

#include "globals.h"
#include "lcm_util.h"
#include "config_util.h"

//#define dbg(...) fprintf(stderr, __VA_ARGS__)
#define dbg(...) 

#define err(...) fprintf(stderr, __VA_ARGS__)

#define MAX_REFERENCES ((1ULL << 60))

static lcm_t *global_lcm;
static int64_t global_lcm_refcount;

static Config *global_config;
static int64_t global_config_refcount;

static CTrans *global_ctrans;
static int64_t global_ctrans_refcount;

static RndfRouteNetwork* global_rndf;
static int64_t global_rndf_refcount;

static GHashTable *_lcgl_hashtable;

static GStaticRecMutex _mutex = G_STATIC_REC_MUTEX_INIT;

lcm_t * 
globals_get_lcm ()
{
    g_static_rec_mutex_lock (&_mutex);

    if (global_lcm_refcount == 0) {
        assert (! global_lcm);

        global_lcm = lcm_create (NULL);
        if (! global_lcm) { goto fail; }

        lcmu_glib_mainloop_attach_lcm (global_lcm);
    }

    assert (global_lcm);

    if (global_lcm_refcount < MAX_REFERENCES) global_lcm_refcount++;
    lcm_t *result = global_lcm;
    g_static_rec_mutex_unlock (&_mutex);
    return result;
fail:
    g_static_rec_mutex_unlock (&_mutex);
    return NULL;
}

void 
globals_release_lcm (lcm_t *lcm)
{
    g_static_rec_mutex_lock (&_mutex);
    if (global_lcm_refcount == 0) {
        fprintf (stderr, "ERROR: singleton LC refcount already zero!\n");
        g_static_rec_mutex_unlock (&_mutex);
        return;
    }
    if (lcm != global_lcm) {
        fprintf (stderr, "ERROR: %p is not the singleton LC (%p)\n",
                lcm, global_lcm);
    }
    global_lcm_refcount--;

    if (global_lcm_refcount == 0) {
        lcmu_glib_mainloop_detach_lcm (global_lcm);
        lcm_destroy (global_lcm);
        global_lcm = NULL;
    }
    g_static_rec_mutex_unlock (&_mutex);
}


Config * 
globals_get_config ()
{
    g_static_rec_mutex_lock (&_mutex);

    if (global_config_refcount == 0) {
        assert (! global_config);

        global_config = config_parse_default ();
        if (! global_config) { goto fail; }
    }

    assert (global_config);

    if (global_config_refcount < MAX_REFERENCES) global_config_refcount++;
    Config *result = global_config;
    g_static_rec_mutex_unlock (&_mutex);
    return result;
fail:
    g_static_rec_mutex_unlock (&_mutex);
    return NULL; 
}

void 
globals_release_config (Config *config)
{
    g_static_rec_mutex_lock (&_mutex);
    if (global_config_refcount == 0) {
        fprintf (stderr, "ERROR: singleton config refcount already zero!\n");
        g_static_rec_mutex_unlock (&_mutex);
        return;
    }
    if (config != global_config) {
        fprintf (stderr, "ERROR: %p is not the singleton Config (%p)\n",
                config, global_config);
    }
    global_config_refcount--;

    if (global_config_refcount == 0) {
        config_free (global_config);
        global_config = NULL;
    }
    g_static_rec_mutex_unlock (&_mutex);
}

CTrans * 
globals_get_ctrans ()
{
    lcm_t *lcm = globals_get_lcm ();
    Config *config = globals_get_config ();

    g_static_rec_mutex_lock (&_mutex);

    if (global_ctrans_refcount == 0) {
        assert (! global_ctrans);

        global_ctrans = ctrans_create (lcm, config, 300);
        if (! global_ctrans) { goto fail; }
    }

    assert (global_ctrans);

    if (global_ctrans_refcount < MAX_REFERENCES) global_ctrans_refcount++;
    CTrans *result = global_ctrans;
    g_static_rec_mutex_unlock (&_mutex);
    return result;
fail:
    g_static_rec_mutex_unlock (&_mutex);
    if (config) globals_release_config (config);
    if (lcm) globals_release_lcm (lcm);
    return NULL;
}

void 
globals_release_ctrans (CTrans *ctrans)
{
    g_static_rec_mutex_lock (&_mutex);

    assert (global_lcm);
    assert (global_config);

    if (global_ctrans_refcount == 0) {
        fprintf (stderr, "ERROR: singleton ctrans refcount already zero!\n");
        g_static_rec_mutex_unlock (&_mutex);
        return;
    }
    if (ctrans != global_ctrans) {
        fprintf (stderr, 
                "ERROR: %p is not the singleton CTrans (%p)\n",
                ctrans, global_ctrans);
    }
    global_ctrans_refcount--;

    if (global_ctrans_refcount == 0) {
        ctrans_destroy (global_ctrans);
        global_ctrans = NULL;
    }
    g_static_rec_mutex_unlock (&_mutex);

    globals_release_lcm (global_lcm);
    globals_release_config (global_config);
}

lcgl_t *
globals_get_lcgl (const char *name, const int create_if_missing)
{
    g_static_rec_mutex_lock (&_mutex);

    if (!global_lcm_refcount)
        globals_get_lcm();

    if (!_lcgl_hashtable) {
        _lcgl_hashtable = g_hash_table_new (g_str_hash, g_str_equal);
    }

    lcgl_t *lcgl = g_hash_table_lookup (_lcgl_hashtable, name);
    if (!lcgl&&create_if_missing) {
        lcgl = lcgl_init (global_lcm, name);
        g_hash_table_insert (_lcgl_hashtable, strdup(name), lcgl);
    }

    g_static_rec_mutex_unlock (&_mutex);
    return lcgl;
}

GHashTable *
globals_get_lcgl_hashtable ()
{
    return _lcgl_hashtable;
}

RndfRouteNetwork * 
globals_get_rndf ()
{
    Config *config = globals_get_config ();
    g_static_rec_mutex_lock (&_mutex);

    if (global_rndf_refcount == 0) {
        assert (! global_rndf);

        char fname[4096];
        if (0 != config_util_get_rndf_absolute_path (config, fname,
                sizeof (fname))) { goto fail; }

        global_rndf = rndf_new_from_file (fname);

        if (! global_rndf) { goto fail; }
    }

    assert (global_rndf);

    global_rndf_refcount++;
    RndfRouteNetwork *result = global_rndf;
    g_static_rec_mutex_unlock (&_mutex);
    return result;
fail:
    fprintf (stderr, 
            "ERROR: (%s:%d) unable to load singleton RNDF\n",
            __FILE__, __LINE__);
    g_static_rec_mutex_unlock (&_mutex);
    if (config) globals_release_config (config);
    return NULL;
}

void 
globals_release_rndf (RndfRouteNetwork *rndf)
{
    g_static_rec_mutex_lock (&_mutex);
    if (global_rndf_refcount == 0) {
        fprintf (stderr, "ERROR: singleton RNDF refcount already zero!\n");
        g_static_rec_mutex_unlock (&_mutex);
        return;
    }
    if (rndf != global_rndf) {
        fprintf (stderr, "ERROR: %p is not the singleton RNDF (%p)\n",
                rndf, global_rndf);
    }
    global_rndf_refcount--;

    if (global_rndf_refcount == 0) {
        dbg ("destroying singleton RNDF\n");
        rndf_destroy (global_rndf);
        global_rndf = NULL;
    }
    g_static_rec_mutex_unlock (&_mutex);
}
