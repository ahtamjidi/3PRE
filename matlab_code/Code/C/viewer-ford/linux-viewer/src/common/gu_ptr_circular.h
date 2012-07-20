#ifndef _GU_PTR_CIRCULAR_H
#define _GU_PTR_CIRCULAR_H

typedef void (*GUPtrCircularDestroy)(void *user, void *p);

/** A circular Buffer, implemented with a PtrArray **/
typedef struct gcircular GUPtrCircular;
struct gcircular
{
    void **p;     // storage
    unsigned int size;     // number of valid elements in the buffer
    unsigned int next;     // where the next add will go
    unsigned int capacity; // maximum allowed capacity

    void *user;
    GUPtrCircularDestroy handler;
};

// create a new circular buffer; the destroy handler will be called
// whenever an element is evicted. (NULL means no handler).
GUPtrCircular *gu_ptr_circular_new(unsigned int capacity, GUPtrCircularDestroy handler, void *user);

// call destroy() on any elements, then deallocate the circular buffer
void gu_ptr_circular_destroy(GUPtrCircular *circ);

// adds a new element to the buffer, possibly evicting the oldest
void gu_ptr_circular_add(GUPtrCircular *circ, void *p);

// return the number of valid elements in the buffer
unsigned int gu_ptr_circular_size(GUPtrCircular *circ);

// An index of zero corresponds to the most recently added item.
void *gu_ptr_circular_index(GUPtrCircular *circ, unsigned int idx);

// resize the circular buffer, freeing elements as required
void gu_ptr_circular_resize(GUPtrCircular *circ, unsigned int capacity);

// remove all elements from the buffer.
void gu_ptr_circular_clear(GUPtrCircular *circ);

#endif
