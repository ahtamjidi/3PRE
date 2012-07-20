#ifndef _MESSAGE_BUFFER_H
#define _MESSAGE_BUFFER_H

#include <pthread.h>

typedef void (*message_buffer_free_func_t)(void *p);

typedef struct 
{
    void *pending;
    void *data;
    
    message_buffer_free_func_t freefun;

    pthread_mutex_t mutex;

} message_buffer_t;

// Create a new thread-safe message buffer.  It holds a single
// message, which can be (re)given as often as desired-- older
// messages are automatically freed. A call to get returns the most
// recent message.
static inline message_buffer_t *message_buffer_create(message_buffer_free_func_t freefun)
{
    message_buffer_t *mb = (message_buffer_t*) calloc(1, sizeof(message_buffer_t));
    mb->freefun = freefun;
    pthread_mutex_init(&mb->mutex, NULL);

    return mb;
}

static inline void message_buffer_destroy(message_buffer_t *mb)
{
    if (mb->data)
        mb->freefun(mb->data);
    if (mb->pending)
        mb->freefun(mb->pending);

    pthread_mutex_destroy(&mb->mutex);

    free(mb);
}

// Give p to the buffer. For LC apps, p should typically be a copy of
// the message. This pointer will be freed by the message buffer at
// some point in the future.
static inline void message_buffer_give(message_buffer_t *mb, void *p)
{
    pthread_mutex_lock(&mb->mutex);

    if (mb->pending)
        mb->freefun(mb->pending);
 
    mb->pending = p;

    pthread_mutex_unlock(&mb->mutex);
}

// Get the most recently given buffer.
static inline void *message_buffer_get(message_buffer_t *mb)
{
    pthread_mutex_lock(&mb->mutex);

    if (mb->pending) {
        if (mb->data)
            mb->freefun(mb->data);
        mb->data = mb->pending;
        mb->pending = NULL;
    }

    pthread_mutex_unlock(&mb->mutex);

    return mb->data;
}

#endif
