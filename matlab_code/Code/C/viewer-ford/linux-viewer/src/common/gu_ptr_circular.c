#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "gu_ptr_circular.h"

// NB: a capacity of 0 is legal!

GUPtrCircular *gu_ptr_circular_new(unsigned int capacity, GUPtrCircularDestroy handler, void *user)
{
    GUPtrCircular *circ = (GUPtrCircular*) calloc(1, sizeof(GUPtrCircular));
    if (capacity > 0)
        circ->p = (void**) calloc(capacity, sizeof(void*));
    else
        circ->p = NULL;
    
    circ->size = 0;
    circ->next = 0;
    circ->capacity = capacity;
    circ->handler = handler;
    circ->user = user;
    
    return circ;
}

// call destroy() on any elements, then deallocate the circular buffer
void gu_ptr_circular_destroy(GUPtrCircular *circ)
{
    for (unsigned int i = 0; i < circ->capacity; i++) {
        if (circ->p[i] && circ->handler)
            circ->handler(circ->user, circ->p[i]);
    }

    if (circ->p)
        free(circ->p);

    circ->p = NULL;
    free(circ);
}

// adds a new element to the buffer, possibly evicting the oldest
void gu_ptr_circular_add(GUPtrCircular *circ, void *p)
{
    if (circ->capacity == 0)
        return;

    // free the old element
    if (circ->size == circ->capacity && circ->handler)
        circ->handler(circ->user, circ->p[circ->next]);

    // add the new one
    circ->p[circ->next] = p;
    circ->next++;
    if (circ->next == circ->capacity)
        circ->next = 0;
    if (circ->size < circ->capacity)
        circ->size++;
}

// return the number of valid elements in the buffer
unsigned int gu_ptr_circular_size(GUPtrCircular *circ)
{
    return circ->size;
}

void *gu_ptr_circular_index(GUPtrCircular *circ, unsigned int idx)
{
    assert (idx < circ->size);

    unsigned int offset = circ->capacity + circ->next - 1 - idx;
    if (offset >= circ->capacity)
        offset -= circ->capacity;

    assert(offset < circ->capacity);

    return circ->p[offset];
}

void gu_ptr_circular_resize(GUPtrCircular *circ, unsigned int new_capacity)
{
    // nothing to do?
    if (new_capacity == circ->capacity)
        return;
    
    // strategy: create a new circular buffer that contains the required data,
    // then swap out our internal representation. We have to be careful
    // about freeing any unneeded data.

    GUPtrCircular *newcirc = gu_ptr_circular_new(new_capacity, circ->handler, circ->user);

    // how many elements in the new one?
    // (so that we are guaranteed to never evict an element while
    // we populate the new data structure)
    unsigned int new_size = circ->size < new_capacity ? circ->size : new_capacity;

    // copy the data into the new structure, oldest first.
    for (unsigned int i = 0; i < new_size; i++)
        gu_ptr_circular_add(newcirc, gu_ptr_circular_index(circ, new_size - 1 - i));

    // free any elements we didn't copy
    for (unsigned int i = new_size; i < circ->size; i++) {
        void *p = gu_ptr_circular_index(circ, i);
        circ->handler(circ->user, p);
    }

    // okay, switch over the data structure
    if (circ->p)
        free(circ->p);
    memcpy(circ, newcirc, sizeof(GUPtrCircular));
    
    // free the new circ container, but not its storage
    free(newcirc);
}

void gu_ptr_circular_clear(GUPtrCircular *circ)
{
    for (unsigned int i = 0; i < circ->size; i++) {
        void *p = gu_ptr_circular_index(circ, i);
        circ->handler(circ->user, p);
    }

    circ->size = 0;
}
