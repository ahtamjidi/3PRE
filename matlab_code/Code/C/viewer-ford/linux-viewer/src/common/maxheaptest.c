#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "maxheap.h"

#define MAXSIZE 1000

maxheap_t *h;
double vals[MAXSIZE];
int    sz;

static float randf()
{
    return ((float) rand()) / (RAND_MAX + 1.0);
}

static void add_item()
{
    if (sz == MAXSIZE)
        return;

    // find space
    int idx = -1;
    for (int i = 0; i < MAXSIZE; i++) {
        if (vals[i] == -HUGE) {
            idx = i;
            break;
        }
    }
    assert (idx >=0);
    
    // insert
    vals[idx] = randf();
    maxheap_add(h, &vals[idx], vals[idx]);
    sz++;
    
    //    printf("+");
    fflush(NULL);
}

static void remove_item()
{
    if (sz == 0)
        return;
    
    double max = -HUGE;
    int    max_idx = -1;
    
    for (int i = 0; i < MAXSIZE; i++) {
        if (vals[i] > max) {
            max = vals[i];
            max_idx = i;
        }
    }
    
    double *p = maxheap_remove_max(h);
    
    assert(*p == vals[max_idx]);
    *p = -HUGE;
    sz--;
    
    //    printf("-");
    fflush(NULL);
}

int main(int argc, char *argv[])
{
    assert(argc > 1);

    srand(atoi(argv[1]));

    h = maxheap_create(8);
    
    for (int i = 0; i < MAXSIZE; i++)
        vals[i] = -HUGE;

    for (int iters = 0; ; iters++) {
        
        if (iters %100 == 0)
            printf("\r%5d %10d", iters, maxheap_size(h));
        
        double prob = randf();
        
        if (prob < .8) {
            add_item();
            continue;
        }

        if (prob < .99) {
            remove_item();
            continue;
        }

        for (int i = 0; i < sz; i++)
            remove_item();
        //        printf("0");
    }
}
