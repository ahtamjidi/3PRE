#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "glib_util.h"

void 
gu_list_free_with_func (GList *list, GDestroyNotify functor)
{
    GList *iter;
    for (iter=list; iter; iter=iter->next) {
        functor (iter->data);
    }
    g_list_free (list);
}

void 
gu_queue_free_with_func (GQueue *queue, GDestroyNotify func)
{
    while (! g_queue_is_empty (queue)) {
        func (g_queue_pop_head (queue));
    }
    g_queue_free (queue);
}

void
gu_ptr_array_free_with_func (GPtrArray *a, GDestroyNotify func)
{
    for (int i=0; i<a->len; i++) {
        func (g_ptr_array_index (a, i));
    }
    g_ptr_array_free (a, TRUE);
}

// creates a newly allocated copy of a GPtrArray
GPtrArray * gu_ptr_array_new_copy (const GPtrArray *a)
{
    GPtrArray *result = g_ptr_array_new ();
    g_ptr_array_set_size (result, a->len);
    memcpy (result->pdata, a->pdata, sizeof (gpointer) * a->len);
    return result;
}

static void
get_all_keys_helper (gpointer key, gpointer value, gpointer user_data)
{
    GPtrArray *keys = (GPtrArray*) user_data;

    g_ptr_array_add(keys, key);    
}

GPtrArray * 
gu_hash_table_get_keys (GHashTable *hash_table)
{
    GPtrArray *keys = g_ptr_array_sized_new(g_hash_table_size(hash_table));
    g_hash_table_foreach (hash_table, get_all_keys_helper, keys);
    return keys;
}

static void
get_all_vals_helper (gpointer key, gpointer value, gpointer user_data)
{
    GPtrArray *vals = (GPtrArray*) user_data;

    g_ptr_array_add(vals, value);    
}

GPtrArray * gu_hash_table_get_vals (GHashTable *hash_table)
{
    GPtrArray *vals = g_ptr_array_sized_new(g_hash_table_size(hash_table));
    g_hash_table_foreach (hash_table, get_all_vals_helper, vals);
    return vals;
}

int 
gu_time_val_compare (const GTimeVal *time1, const GTimeVal *time2)
{
    if (time1->tv_sec > time2->tv_sec) return 1;
    if (time1->tv_sec < time2->tv_usec) return -1;
    if (time1->tv_usec > time2->tv_usec) return 1;
    if (time1->tv_usec < time2->tv_usec) return -1;
    return 0;
}

int g_ptr_array_find_index(GPtrArray *a, gconstpointer v)
{
    for (unsigned int i = 0; i < g_ptr_array_size(a); i++) {
        if (g_ptr_array_index(a, i) == v)
            return i;
    }
    return -1;
}

// hash/equal functions for GHashTable and (pointers to) int64s as keys.
guint pint64_hash(gconstpointer _key)
{
    int64_t *key = (int64_t*) _key;

    return *key;
}

gboolean pint64_equal(gconstpointer _a, gconstpointer _b)
{
    int64_t *a = (int64_t*) _a;
    int64_t *b = (int64_t*) _b;

    return (*a) == (*b);
}
