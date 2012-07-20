#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gu_circular.h"

GUCircular *
gu_circular_new (int capacity, int element_size)
{
    GUCircular * circular = malloc (sizeof (GUCircular));
    if (!circular)
        return NULL;
    memset (circular, 0, sizeof (GUCircular));

    circular->array = calloc (capacity, element_size);
    if (!circular->array) {
        free (circular);
        return NULL;
    }
    circular->capacity = capacity;
    circular->element_size = element_size;
    return circular;
}

void
gu_circular_free (GUCircular * circular)
{
    free (circular->array);
    memset (circular, 0, sizeof (GUCircular));
    free (circular);
}

void
gu_circular_clear (GUCircular * circular)
{
    circular->head = 0;
    circular->len = 0;
}

int
gu_circular_push_head (GUCircular * circular, const void * data)
{
    circular->head--;
    if (circular->head < 0)
        circular->head = circular->capacity - 1;

    if (circular->len < circular->capacity)
        circular->len++;

    memcpy (gu_circular_peek_nth (circular, 0), data, circular->element_size);

    return 0;
}

int
gu_circular_pop_tail (GUCircular * circular, void * data)
{
    if (gu_circular_is_empty (circular))
        return -1;

    if (data)
        memcpy (data, gu_circular_peek_nth (circular, circular->len - 1),
                circular->element_size);

    circular->len--;

    return 0;
}
