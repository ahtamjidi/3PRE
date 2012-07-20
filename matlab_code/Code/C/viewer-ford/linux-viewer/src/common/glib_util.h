#ifndef __glib_util_h__
#define __glib_util_h__

#include <stdint.h>

// useful convenience functions and data structures that are missing from glib

#include <glib.h>
#include "gu_circular.h"
#include "gu_ptr_circular.h"
#include "gu_minheap.h"
#include "gu_set.h"
#include "gu_disjoint_set_forest.h"

#ifdef __cplusplus
extern "C" {
#endif

// calls func on every element of the specified list, and then
// frees the list
void gu_list_free_with_func (GList *list, GDestroyNotify func);
    
// calls func on every element of the specified queue, and then
// frees the queue
void gu_queue_free_with_func (GQueue *queue, GDestroyNotify func);

// calls func on every element of the specified pointer array, and then
// frees the pointer array.
void gu_ptr_array_free_with_func (GPtrArray *a, GDestroyNotify func);

// creates a newly allocated copy of a GPtrArray
GPtrArray * gu_ptr_array_new_copy (const GPtrArray *a);
    
// returns a newly allocated list containing all the keys in the specified
// hash table.  The calling function is responsible for calling
// g_ptr_array_free on the list when no longer needed.
GPtrArray * gu_hash_table_get_keys (GHashTable *hash_table);

// returns a newly allocated list containing all the values in the specified
// hash table.  The calling function is responsible for calling
// g_ptr_array_free on the list when no longer needed.
GPtrArray * gu_hash_table_get_vals (GHashTable *hash_table);

/**
 * Returns: 1 if time1 is after time2
 *          0 if time1 and time2 are equal
 *         -1 if time1 is before time2
 */
int gu_time_val_compare (const GTimeVal *time1, const GTimeVal *time2);

#ifndef g_ptr_array_size
#define g_ptr_array_size(ptrarray) ((ptrarray)->len)
#endif

#ifndef g_ptr_array_set
#define g_ptr_array_set(ptrarray, idx, val) (ptrarray)->pdata[(idx)] = (val);
#endif

int g_ptr_array_find_index(GPtrArray *a, gconstpointer v);
#define gu_ptr_array_find_index g_ptr_array_find_index

guint pint64_hash(gconstpointer _key);
gboolean pint64_equal(gconstpointer _a, gconstpointer _b);

#ifdef __cplusplus
}
#endif

#endif
