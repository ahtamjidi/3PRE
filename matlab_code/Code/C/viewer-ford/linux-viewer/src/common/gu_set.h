#ifndef __gu_set_h__
#define __gu_set_h__

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GUSet GUSet;

GUSet * gu_set_new (GHashFunc hash_func, GEqualFunc equal_func);

GUSet * gu_set_new_full (GHashFunc hash_func, GEqualFunc equal_func,
        GDestroyNotify element_destroy_func);

GUSet * gu_set_new_union (const GUSet *set1, const GUSet *set2);

GUSet * gu_set_new_intersection (const GUSet *set1, const GUSet *set2);

void gu_set_destroy (GUSet *set);

void gu_set_add (GUSet *set, gpointer element);

void gu_set_remove (GUSet *set, gpointer element);

void gu_set_remove_all (GUSet *set);

int gu_set_size (const GUSet *set);

gboolean gu_set_contains (const GUSet *set, gpointer element);

typedef void (*GUSFunc) (gpointer element, gpointer user_data);
void gu_set_foreach (GUSet *set, GUSFunc func, gpointer user_data);

GPtrArray *gu_set_get_elements (GUSet *set);

#ifdef __cplusplus
}
#endif

#endif
