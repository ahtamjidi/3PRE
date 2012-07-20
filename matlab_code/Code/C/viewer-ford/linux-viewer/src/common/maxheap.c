#include <stdlib.h>
#include "maxheap.h"

struct entry
{
    void   *ptr;
    double  score;
};

struct maxheap
{
    struct entry *entries;

    int capacity;
    int size;
};

static void swap_nodes(maxheap_t *h, int a, int b)
{
    struct entry tmp = h->entries[a];
    h->entries[a] = h->entries[b];
    h->entries[b] = tmp;
}

static void fixup(maxheap_t *h, int parent)
{
    int left = parent*2 + 1;
    int right = left + 1;

    // leaf node, exit.
    if (left >= h->size)
        return;

    // only left node is valid
    if (right >= h->size) {
        // do we need to swap the parent and the leaf?
        // we don't need to call fixupMaxheap recursively since the
        // left node must be a leaf.
        if (h->entries[left].score > h->entries[parent].score)
        {
            swap_nodes(h, left, parent);
            return;
        }
        return;
    }

    // general node case: both right and left are valid nodes.
    
    // if parent is the maximum, we're done.
    if (h->entries[parent].score > h->entries[left].score &&
        h->entries[parent].score > h->entries[right].score)
        return;
    
    // parent is less than either the left, right, or both
    // children
    if (h->entries[left].score > h->entries[right].score)  {
        swap_nodes(h, left, parent);
        fixup(h, left);
    } else {
        swap_nodes(h, right, parent);
        fixup(h, right);
    }
}

maxheap_t *maxheap_create(int capacity)
{
    if (capacity < 8)
        capacity = 8;

    maxheap_t *h = (maxheap_t*) calloc(1, sizeof(maxheap_t));
    h->capacity = capacity;
    h->size = 0;

    h->entries = (struct entry*) calloc(capacity, sizeof(struct entry));
    return h;
}

void maxheap_destroy(maxheap_t *h)
{
    free(h->entries);
    free(h);
}

void maxheap_add(maxheap_t *h, void *ptr, double score)
{
    if (h->size == h->capacity) {
        // grow.
        h->capacity *= 2;
        h->entries = (struct entry*) realloc(h->entries, sizeof(struct entry)*h->capacity);
    }

    h->entries[h->size].ptr = ptr;
    h->entries[h->size].score = score;
    
    int node = h->size;
    h->size++;

    do {
        node = (node - 1)/2;
        fixup(h, node);
    } while (node != 0);
}

int maxheap_size(maxheap_t *h)
{
    return h->size;
}

void *maxheap_remove_max(maxheap_t *h)
{
    if (h->size == 0)
        return NULL;

    void *ptr = h->entries[0].ptr;
    h->entries[0] = h->entries[h->size - 1];
    h->size--;

    fixup(h, 0);

    return ptr;
}

