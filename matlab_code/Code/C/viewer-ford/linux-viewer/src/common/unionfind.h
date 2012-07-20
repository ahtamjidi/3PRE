#ifndef _UNIONFIND_H
#define _UNIONFIND_H

#include <stdint.h>

typedef struct unionfind unionfind_t;

/** Create a new unionfind object that will use no more than size distinct ID values. **/
unionfind_t *unionfind_create(int size);

void unionfind_destroy(unionfind_t *uf);

/** Find the lowest id number of the component that contains the given ID.**/
int64_t unionfind_representative(unionfind_t *uf, int64_t id);

/** Add an edge between the two ids. This merges the groups. **/
void unionfind_union(unionfind_t *uf, int64_t id_a, int64_t id_b);

#endif
