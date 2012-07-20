#include <stdio.h>
#include <stdlib.h>

#include "gu_set.h"

struct _GUSet {
    GHashTable *hash_table;
    GHashFunc hash_func;
    GEqualFunc equal_func;
    GDestroyNotify element_destroy_func;
};

GUSet * 
gu_set_new (GHashFunc hash_func, GEqualFunc equal_func)
{
    return gu_set_new_full (hash_func, equal_func, NULL);
}

GUSet * 
gu_set_new_full (GHashFunc hash_func, GEqualFunc equal_func,
        GDestroyNotify element_destroy_func)
{
    GUSet *set = g_slice_new (GUSet);
    set->hash_table = g_hash_table_new_full (hash_func, equal_func,
            element_destroy_func, NULL);
    set->hash_func = hash_func;
    set->equal_func = equal_func;
    set->element_destroy_func = element_destroy_func;
    return set;
}

static void 
_new_union_add (gpointer key, gpointer value, void *user_data)
{
    GUSet *set = user_data;
    gu_set_add (set, key);
}

GUSet *
gu_set_new_union (const GUSet *set1, const GUSet *set2)
{
    if (set1->hash_func != set2->hash_func) {
        g_error ("Can't union GUSet objects with different hash functions");
        return NULL;
    }
    if (set1->equal_func != set2->equal_func) {
        g_error ("Can't union GUSet objects with different equal functions");
        return NULL;
    }
    if (set1->element_destroy_func != set2->element_destroy_func) {
        g_error ("Can't union GUSet objects with different element destroy functions");
        return NULL;
    }
    GUSet *result = gu_set_new_full (set1->hash_func, set1->equal_func,
            set1->element_destroy_func);
    g_hash_table_foreach (set1->hash_table, _new_union_add, result);
    g_hash_table_foreach (set2->hash_table, _new_union_add, result);
    return result;
}

struct _set_intersect_data {
    const GUSet *other_set;
    GUSet *new_set;
};

static void
_new_intersect_add (gpointer key, gpointer value, void *user_data)
{
    struct _set_intersect_data *d = user_data;
    if (gu_set_contains (d->other_set, key)) gu_set_add (d->new_set, key);
}

GUSet *
gu_set_new_intersection (const GUSet *set1, const GUSet *set2)
{
    if (set1->hash_func != set2->hash_func) {
        g_error ("Can't instersect GUSet objects with different hash functions");
        return NULL;
    }
    if (set1->equal_func != set2->equal_func) {
        g_error ("Can't instersect GUSet objects with different equal functions");
        return NULL;
    }
    if (set1->element_destroy_func != set2->element_destroy_func) {
        g_error ("Can't instersect GUSet objects with different element destroy functions");
        return NULL;
    }
    GUSet *result = gu_set_new_full (set1->hash_func, set1->equal_func,
            set1->element_destroy_func);
    if (gu_set_size (set1) < gu_set_size (set2)) {
        struct _set_intersect_data d = { set2, result };
        g_hash_table_foreach (set1->hash_table, _new_intersect_add, &d);
    } else {
        struct _set_intersect_data d = { set1, result };
        g_hash_table_foreach (set2->hash_table, _new_intersect_add, &d);
    }
    return result;
}

void 
gu_set_destroy (GUSet *set)
{
    g_hash_table_destroy (set->hash_table);
    g_slice_free (GUSet, set);
}

void 
gu_set_add (GUSet *set, gpointer element)
{
    g_hash_table_insert (set->hash_table, element, element);
}

void 
gu_set_remove (GUSet *set, gpointer element)
{
    g_hash_table_remove (set->hash_table, element);
}

void 
gu_set_remove_all (GUSet *set)
{
    g_hash_table_remove_all (set->hash_table);
}

int
gu_set_size (const GUSet *set)
{
    return g_hash_table_size (set->hash_table);
}

gboolean 
gu_set_contains (const GUSet *set, gpointer element)
{
    return g_hash_table_lookup (set->hash_table, element) == element;
}

struct _foreach_data {
    GUSFunc func;
    gpointer user_data;
};

static void
_foreach_func (gpointer key, gpointer value, gpointer user_data)
{
    struct _foreach_data *d = user_data;
    d->func (key, d->user_data);
}

void 
gu_set_foreach (GUSet *set, GUSFunc func, gpointer user_data)
{
    struct _foreach_data d = { func, user_data };
    g_hash_table_foreach (set->hash_table, _foreach_func, &d);
}

static void
_get_elements_foreach (gpointer key, gpointer value, gpointer user_data)
{
    g_ptr_array_add ((GPtrArray*)user_data, key);
}

GPtrArray *
gu_set_get_elements (GUSet *set)
{
    GPtrArray *result = 
        g_ptr_array_sized_new (g_hash_table_size (set->hash_table));
    g_hash_table_foreach (set->hash_table, _get_elements_foreach, result);
    return result;
}
