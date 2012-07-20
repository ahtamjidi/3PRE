#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <glib.h>

#include "gu_minheap.h"

struct _GUMinheapNode 
{
    int index;
    void *ptr;
    double score;
};

struct _GUMinheap 
{
    GPtrArray *nodes;
};

#define _get_node(mh, ind) ((GUMinheapNode*) g_ptr_array_index(mh->nodes,ind))

static inline void 
_swap_nodes (GUMinheap *mh, int a, int b)
{
    GUMinheapNode *nodea = _get_node (mh, a);
    GUMinheapNode *nodeb = _get_node (mh, b);
    
    g_ptr_array_index (mh->nodes, a) = nodeb;
    g_ptr_array_index (mh->nodes, b) = nodea;

    nodeb->index = a;
    nodea->index = b;
}

static void 
fixup (GUMinheap *mh, int parent_ind)
{
    if (parent_ind >= mh->nodes->len) return;

    int left_ind = parent_ind*2 + 1;
    int right_ind = left_ind + 1;

    // leaf node, exit.
    if (left_ind >= mh->nodes->len) return;

    GUMinheapNode *node_parent = _get_node (mh, parent_ind);
    assert (node_parent->index == parent_ind);

    // only left node is valid
    if (right_ind >= mh->nodes->len) {
        GUMinheapNode *node_left = _get_node (mh, left_ind);
        assert (node_left->index == left_ind);

        // do we need to swap the parent and the leaf?
        // we don't need to call fixupMaxheap recursively since the
        // left node must be a leaf.
        if (node_left->score < node_parent->score) {
            _swap_nodes(mh, left_ind, parent_ind);
            return;
        }
        return;
    }

    // general node case: both right and left are valid nodes.
    GUMinheapNode *node_left = _get_node (mh, left_ind);
    GUMinheapNode *node_right = _get_node (mh, right_ind);
    assert (node_left->index == left_ind);
    assert (node_right->index == right_ind);
    
    // if parent is the minimum, we're done.
    if (node_parent->score < node_left->score &&
        node_parent->score < node_right->score)
        return;
    
    // parent is greater than either the left, right, or both
    // children
    if (node_left->score < node_right->score)  {
        _swap_nodes(mh, left_ind, parent_ind);
        fixup(mh, left_ind);
    } else {
        _swap_nodes(mh, right_ind, parent_ind);
        fixup(mh, right_ind);
    }
}

GUMinheap *gu_minheap_new()
{
    return gu_minheap_sized_new (8);
}

GUMinheap *gu_minheap_sized_new(int capacity)
{
    if (capacity < 8) capacity = 8;

    GUMinheap *mh = g_slice_new (GUMinheap);
    mh->nodes = g_ptr_array_sized_new (capacity);
    return mh;
}

void gu_minheap_free(GUMinheap *mh)
{
    for (int i=0; i<mh->nodes->len; i++) {
        GUMinheapNode *node = _get_node (mh, i);
        g_slice_free (GUMinheapNode, node);
    }
    g_ptr_array_free (mh->nodes, TRUE);
    g_slice_free (GUMinheap, mh);
}

GUMinheapNode *
gu_minheap_add(GUMinheap *mh, void *ptr, double score)
{
    GUMinheapNode *node = g_slice_new (GUMinheapNode);
    int node_ind = mh->nodes->len;
    node->index = node_ind;
    node->ptr = ptr;
    node->score = score;
    g_ptr_array_add (mh->nodes, node);

    do {
        node_ind = (node_ind - 1)/2;
        fixup (mh, node_ind);
    } while (node_ind);
    return node;
}

int gu_minheap_size(GUMinheap *mh)
{
    return mh->nodes->len;
}

void *
gu_minheap_remove_min (GUMinheap *mh, double *score)
{
    if (!mh->nodes->len) return NULL;
    GUMinheapNode *root = 
        (GUMinheapNode*) g_ptr_array_remove_index_fast (mh->nodes, 0);
    void *result = root->ptr;
    if (score) *score = root->score;
    g_slice_free (GUMinheapNode, root);

    if (mh->nodes->len) {
        GUMinheapNode *replacement = _get_node (mh, 0);
        replacement->index = 0;
        fixup(mh, 0);
    }

    return result;
}

void 
gu_minheap_decrease_score (GUMinheap *mh, GUMinheapNode *node, double score)
{
    if (score > node->score) {
        g_warning ("GUMinHeap: refusing to increase the score of a node\n");
        return;
    }
    node->score = score;
    assert (_get_node (mh, node->index) == node);
    int node_ind = node->index;
    do {
        node_ind = (node_ind - 1)/2;
        fixup (mh, node_ind);
    } while (node_ind);
}

gboolean 
gu_minheap_is_empty (GUMinheap *mh)
{
    return mh->nodes->len == 0;
}
