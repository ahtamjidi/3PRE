#include <stdlib.h>
#include <assert.h>
#include <common/math_util.h>

#include "gu_ptr_array_queue.h"

/*
static int round_up_to_power_of_two(int size)
{
    int v = 1;
    while ( v < size)
        v*=2;

    return v;
}
*/

static inline int wrap_around(GUPtrArrayQueue *q, int v)
{
    while (v < 0)
        v += q->capacity;
    while (v >= q->capacity)
        v -= q->capacity;
    return v;
}

GUPtrArrayQueue *gu_ptr_array_queue_new(unsigned int capacity)
{
    assert (capacity > 0);

    GUPtrArrayQueue *q = (GUPtrArrayQueue*) calloc(1, sizeof(GUPtrArrayQueue));
    q->capacity = capacity;
    q->p = (void**) calloc(q->capacity, sizeof(void*));
    q->next_head = 0;
    q->next_tail = wrap_around(q, 1);
    return q;
}

void gu_ptr_array_queue_destroy(GUPtrArrayQueue *q)
{
    free(q->p);
    q->p = NULL;
    free(q);
}

// returns -1 if the array was full, in which case the add does not
// occur.
int gu_ptr_array_queue_add_head(GUPtrArrayQueue *q, void *p)
{
    if (q->size == q->capacity)
        return -1;

    q->p[q->next_head] = p;

    q->next_head = wrap_around(q, q->next_head - 1);
    q->size++;
    return 0;
}

int gu_ptr_array_queue_add_tail(GUPtrArrayQueue *q, void *p)
{
    if (q->size == q->capacity)
        return -1;

    q->p[q->next_tail] = p;

    q->next_tail = wrap_around(q, q->next_tail + 1);
    q->size++;
    return 0;
}

// current number of elements in the queue.
int gu_ptr_array_queue_size(GUPtrArrayQueue *q)
{
    return q->size;
}

int gu_ptr_array_queue_capacity(GUPtrArrayQueue *q)
{
    return q->capacity;
}

void *gu_ptr_array_queue_remove_head(GUPtrArrayQueue *q)
{
    if (q->size == 0)
        return NULL;

    q->next_head = wrap_around(q, q->next_head + 1);
    q->size--;
    return q->p[q->next_head];
}

void *gu_ptr_array_queue_remove_tail(GUPtrArrayQueue *q)
{
    if (q->size == 0)
        return NULL;

    q->next_tail = wrap_around(q, q->next_tail - 1);
    q->size--;
    return q->p[q->next_tail];
}

// get the Nth element from the head. 0 = the head.
void *gu_ptr_array_queue_get_from_head(GUPtrArrayQueue *q, int offset)
{
    assert(offset >= 0);
    assert(offset < q->size);

    return q->p[ wrap_around(q, q->next_head + 1 + offset) ];
}

// get the Nth element from the tail. 0 = the tail.
void *gu_ptr_array_queue_get_from_tail(GUPtrArrayQueue *q, int offset)
{
    assert(offset >= 0);
    assert(offset < q->size);

    return q->p[ wrap_around(q, q->next_tail - 1 - offset)];
}
