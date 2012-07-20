#ifndef _GU_PTR_ARRAY_QUEUE_H
#define _GU_PTR_ARRAY_QUEUE_H

typedef struct gu_ptr_array_queue GUPtrArrayQueue;

struct gu_ptr_array_queue
{
    void **p;  // storage
    int  next_head; // where the next add_head element would go
    int  next_tail; // where the next add_tail element would go

    int  capacity;
    int  size;
};

GUPtrArrayQueue *gu_ptr_array_queue_new(unsigned int capacity);
void gu_ptr_array_queue_destroy(GUPtrArrayQueue *q);

// returns -1 if the array was full, in which case the add does not
// occur.
int gu_ptr_array_queue_add_head(GUPtrArrayQueue *q, void *p);
int gu_ptr_array_queue_add_tail(GUPtrArrayQueue *q, void *p);

// current number of elements in the queue.
int gu_ptr_array_queue_size(GUPtrArrayQueue *q);
int gu_ptr_array_queue_capacity(GUPtrArrayQueue *q);

// returns NULL if empty
void *gu_ptr_array_queue_remove_head(GUPtrArrayQueue *q);
void *gu_ptr_array_queue_remove_tail(GUPtrArrayQueue *q);

// get the Nth element from the head. 0 = the head.
void *gu_ptr_array_queue_get_from_head(GUPtrArrayQueue *q, int offset);

// get the Nth element from the tail. 0 = the tail.
void *gu_ptr_array_queue_get_from_tail(GUPtrArrayQueue *q, int offset);


#endif
