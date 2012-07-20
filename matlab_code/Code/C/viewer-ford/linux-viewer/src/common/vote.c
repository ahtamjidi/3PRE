#include <stdlib.h>
#include <string.h>

#include <common/math_util.h>

#include "vote.h"

struct entry
{
    int64_t id;
    int votes;
};

struct vote
{
    struct entry *entries;
    int capacity;

    int64_t winner_id;
    int     winner_votes;
};

static int round_up_to_power_of_two(int size)
{
    int v = 1;
    while ( v < size)
        v*=2;

    return imax(16, v);
}

vote_t *vote_create(int capacity)
{
    capacity = round_up_to_power_of_two(capacity*2 + 16);

    vote_t *v = (vote_t*) calloc(1, sizeof(vote_t));
    v->entries = (struct entry*) calloc(capacity, sizeof(struct entry));
    v->capacity = capacity;
    return v;
}

void vote_destroy(vote_t *v)
{
    free(v->entries);
    free(v);
}

void vote_cast_vote(vote_t *v, int64_t id)
{
    int idx = id & (v->capacity - 1);

    while (v->entries[idx].votes > 0 && v->entries[idx].id != id)
        idx = (idx+1) & (v->capacity - 1);

    v->entries[idx].id = id;
    v->entries[idx].votes++;

    if (v->entries[idx].votes > v->winner_votes) {
        v->winner_id = id;
        v->winner_votes = v->entries[idx].votes;
    }
}

void vote_reset(vote_t *v)
{
    memset(v->entries, 0, v->capacity * sizeof(struct entry));
    v->winner_votes = 0;
    v->winner_id = -1;
}

int64_t vote_get_winner(vote_t *v, int *nvotes)
{
    *nvotes = v->winner_votes;
    return v->winner_id;
}
