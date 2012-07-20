#ifndef _MAXHEAP_H
#define _MAXHEAP_H

typedef struct maxheap maxheap_t;


maxheap_t *maxheap_create(int capacity);
void maxheap_destroy(maxheap_t *h);
void maxheap_add(maxheap_t *h, void *ptr, double score);
int maxheap_size(maxheap_t *h);
void *maxheap_remove_max(maxheap_t *h);

#endif
