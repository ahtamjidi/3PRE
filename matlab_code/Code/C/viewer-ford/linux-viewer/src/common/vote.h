#ifndef _VOTE_H
#define _VOTE_H

#include <stdint.h>

/** This object allows you to vote for a number of alternatives
 * (specified by unique ids) and then determine the id of the
 * most-voted-for id
 **/
typedef struct vote vote_t;

// Capacity should be the maximum number of elements. This will be
// increased internally for efficiency reasons.
vote_t *vote_create(int capacity);
void vote_destroy(vote_t *v);
void vote_cast_vote(vote_t *v, int64_t id);

// returns -1 if there were no votes.
int64_t vote_get_winner(vote_t *v, int *nvotes);
void vote_reset(vote_t *v);

#endif
