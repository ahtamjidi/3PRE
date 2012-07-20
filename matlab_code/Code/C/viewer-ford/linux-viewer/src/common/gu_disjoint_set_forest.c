#include <glib.h>
#include "gu_disjoint_set_forest.h"

typedef struct _node {
    int parent;
    int rank;
} node_t;

struct _GUDisjointSetForest {
    GArray *nodes;
};

GUDisjointSetForest * 
gu_disjoint_set_forest_new (int num_elems)
{
    GUDisjointSetForest * self = g_slice_new (GUDisjointSetForest);
    self->nodes = g_array_sized_new (FALSE, FALSE, sizeof (node_t), num_elems);
    for (int i=0; i<num_elems; i++) {
        node_t node = {
            i, 1
        };
        g_array_append_val (self->nodes, node);
    }
    return self;
}

void 
gu_disjoint_set_forest_destroy (GUDisjointSetForest * self)
{
    g_array_free (self->nodes, TRUE);
    g_slice_free (GUDisjointSetForest, self);
}

int
gu_disjoint_set_forest_find (GUDisjointSetForest *self, int elem)
{
    int par = elem;
    node_t *node = &g_array_index (self->nodes, node_t, par);
    while (node->parent != par) {
        par = node->parent;
        node = &g_array_index (self->nodes, node_t, par);
    }
    node->parent = par;
    return par;
}

void 
gu_disjoint_set_forest_union (GUDisjointSetForest *self, int elem1, int elem2)
{
    int par1 = gu_disjoint_set_forest_find (self, elem1);
    int par2 = gu_disjoint_set_forest_find (self, elem2);
    if (par1 == par2) return;

    node_t *node1 = &g_array_index (self->nodes, node_t, par1);
    node_t *node2 = &g_array_index (self->nodes, node_t, par2);
    if (node1->rank > node2->rank) {
        node2->parent = par1;
    } else if (node2->rank > node1->rank) {
        node1->parent = par2;
    } else {
        node1->parent = par2;
        node2->rank ++;
    }
}
