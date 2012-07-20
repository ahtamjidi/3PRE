#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include <common/math_util.h>

#include "unionfind.h"

/**
   Based on:
   http://en.wikipedia.org/wiki/Union-find

   A hashtable is used so that int64s can be used as ids (as opposed
   to having to allocate max(id) entries.) We also guarantee that the
   representative ID for a connected component is the smallest id in
   that component.
**/

struct unionfind_entry
{
    int64_t id;             // the id of this node.
    int64_t parent;         // our parent. if == id, we're a root.
    int64_t representative; // if we're a root, the representative id for this union
    int     rank;           // how deep is the tree beneath us?
};

struct unionfind
{
    int    size;        // power of two
    struct unionfind_entry *entries;
};

static int round_up_to_power_of_two(int size)
{
    int v = 1;
    while ( v < size)
        v*=2;

    return imax(16, v);
}

unionfind_t *unionfind_create(int size)
{
    size = round_up_to_power_of_two((size+16) * 4); // bigger multiple --> fewer collisions --> faster

    unionfind_t *uf = (unionfind_t*) calloc(1, sizeof(unionfind_t));
    uf->size  = size;
    uf->entries = (struct unionfind_entry*) calloc(size, sizeof(struct unionfind_entry));
    return uf;
}

void unionfind_destroy(unionfind_t *uf)
{
    free(uf->entries);
    free(uf);
}

/** find (allocating if necessary) the entry for the given id. **/
static inline struct unionfind_entry *find_entry(unionfind_t *uf, int64_t id)
{
    int idx = id & (uf->size - 1);
    
    while (uf->entries[idx].id && uf->entries[idx].id != id)
        idx = (idx+1) & (uf->size - 1);

    // MakeSet
    if (uf->entries[idx].id == 0) {
        uf->entries[idx].id = id;
        uf->entries[idx].parent = id;
        uf->entries[idx].representative = id;
        uf->entries[idx].rank = 0;
    }
        
    return &uf->entries[idx];
}

/** find the root for entry a. **/
inline struct unionfind_entry *unionfind_find_rec(unionfind_t *uf, struct unionfind_entry *a)
{
    struct unionfind_entry *node = a;

    while (1) {
        if (node->parent == node->id)
            return node; // we found the root!

        struct unionfind_entry *node_parent = find_entry(uf, node->parent);
        // Flatten.
        node->parent = node_parent->id;
        node = node_parent;
    }
}

int64_t unionfind_representative(unionfind_t *uf, int64_t id)
{
    struct unionfind_entry *a = find_entry(uf, id);
    struct unionfind_entry *aroot = unionfind_find_rec(uf, a);

    return aroot->representative;
}

void unionfind_union(unionfind_t *uf, int64_t id_a, int64_t id_b)
{
    struct unionfind_entry *a = find_entry(uf, id_a);
    struct unionfind_entry *b = find_entry(uf, id_b);

    struct unionfind_entry *aroot = unionfind_find_rec(uf, a);
    struct unionfind_entry *broot = unionfind_find_rec(uf, b);

    // the two clusters are already joined.
    if (aroot->id == broot->id)
        return;

    if (aroot->rank > broot->rank) {
        broot->parent = aroot->id;
        aroot->representative = imin64(aroot->representative, broot->representative);
    } else if (aroot->rank < broot->rank) {
        aroot->parent = broot->id;
        broot->representative = imin64(aroot->representative, broot->representative);
    } else {
        broot->parent = aroot->id;
        aroot->rank++;
        aroot->representative = imin64(aroot->representative, broot->representative);
    }
}


