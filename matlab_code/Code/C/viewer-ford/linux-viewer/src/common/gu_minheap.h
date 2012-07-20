#ifndef __GU_MINHEAP_H__
#define __GU_MINHEAP_H__

#include <glib.h>

typedef struct _GUMinheap GUMinheap;
typedef struct _GUMinheapNode GUMinheapNode;


GUMinheap *gu_minheap_new();

GUMinheap *gu_minheap_sized_new(int capacity);

void gu_minheap_free(GUMinheap *mh);

GUMinheapNode *gu_minheap_add (GUMinheap *mh, void *data, double score);

void gu_minheap_decrease_score (GUMinheap *mh, GUMinheapNode *node, 
        double score);

void *gu_minheap_remove_min (GUMinheap *mh, double *score);

int gu_minheap_size (GUMinheap *mh);

gboolean gu_minheap_is_empty (GUMinheap *mh);

#endif
