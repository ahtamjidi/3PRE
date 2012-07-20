#ifndef __GU_CIRCULAR_H__
#define __GU_CIRCULAR_H__

/**
 * SECTION:gu_circular
 *
 * GUCircular is a hybrid of the glib types GArray and GQueue.  GUCircular
 * acts like a GQueue in the sense that you can push on one end and pop
 * from the other.  It acts like a GArray in the sense that its contents
 * are statically allocated rather than pointers to user-allocated buffers.
 * For this reason, its size is fixed and allocated when the GUCircular
 * is created (TODO: set_size function).  If a new element is pushed
 * when the GUCircular is already full, the last element on the tail is
 * automatically overwritten.
 */
typedef struct _GUCircular GUCircular;

struct _GUCircular {
    int len;
    int capacity;
    int element_size;
    int head;
    void * array;
};

GUCircular *
gu_circular_new (int capacity, int element_size);
void
gu_circular_free (GUCircular * circular);
void
gu_circular_clear (GUCircular * circular);
int
gu_circular_push_head (GUCircular * circular, const void * data);
int
gu_circular_pop_tail (GUCircular * circular, void * data);

#define gu_circular_is_empty(a) ((a)->len == 0)

#define gu_circular_is_full(a) ((a)->len >= (a)->capacity)

#define gu_circular_peek_nth(a,i) \
    ((a)->array + (((a)->head + (i)) % (a)->capacity) * (a)->element_size)

#endif
