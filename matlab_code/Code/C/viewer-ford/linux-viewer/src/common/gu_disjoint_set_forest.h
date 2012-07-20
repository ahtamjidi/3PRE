#ifndef __gu_disjoint_set_forest_h__
#define __gu_disjoint_set_forest_h__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * GUDisjointSetForest:
 *
 * implementation of a Disjoint-set forest data structure with union by rank
 * and path compression.
 * 
 * Requires the total number of elements to be known beforehand, and that each
 * element be indexed.
 */
typedef struct _GUDisjointSetForest GUDisjointSetForest;

GUDisjointSetForest * gu_disjoint_set_forest_new (int num_elems);

void gu_disjoint_set_forest_destroy (GUDisjointSetForest *forest);

int gu_disjoint_set_forest_find (GUDisjointSetForest *forest, int i);

void gu_disjoint_set_forest_union (GUDisjointSetForest *forest, int i, int j);

#ifdef __cplusplus
}
#endif

#endif
