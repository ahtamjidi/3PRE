// gcc -o gu_minheap_tester gu_minheap_tester.c ../../lib/libcommon.a `pkg-config --cflags --libs glib-2.0` -std=gnu99

#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#include "glib_util.h"

int main (int argc, char **argv)
{
    for (int iter=0; iter<100; iter++) {
        int n = 50;

        GUMinheap *mh = gu_minheap_new ();
        printf ("adding...\n");
        for (int i=0; i<n; i++) {
            uint32_t v = g_random_int () % 1000;
            gu_minheap_add (mh, GUINT_TO_POINTER(v), v);
            printf ("%d ", v);
        }
        printf ("\n");

        printf ("removing...\n");
        uint32_t last_removal = 0;
        while (!gu_minheap_is_empty(mh)) {
            void *ptr = gu_minheap_remove_min (mh, NULL);
            uint32_t v = GPOINTER_TO_UINT (ptr);
            printf ("%d ", v);
            assert (v >= last_removal);
            last_removal = v;
        }
        printf ("\n");
        gu_minheap_free (mh);

        struct elem {
            uint32_t v;
            GUMinheapNode *node;
        };

        mh = gu_minheap_new ();
        printf ("adding... \n");
        struct elem elems[n];
        for (int i=0; i<n; i++) {
            elems[i].v = g_random_int () % 1000;
            elems[i].node = gu_minheap_add (mh, &elems[i], elems[i].v);
            printf ("%d ", elems[i].v);
        }

        printf ("\nmodifying...\n");
        for (int i=0; i<n; i++) {
            elems[i].v = elems[i].v ? (g_random_int () % 1000) % elems[i].v : 0;
            gu_minheap_decrease_score (mh, elems[i].node, elems[i].v);
            printf ("%d ", elems[i].v);
        }

        printf ("\nremoving... \n");
        last_removal = 0;
        while (!gu_minheap_is_empty (mh)) {
            struct elem *e = (struct elem*) gu_minheap_remove_min (mh, NULL);
            printf ("%d ", e->v);
            assert (e->v >= last_removal);
            last_removal = e->v;
        }
        printf ("\n");
        gu_minheap_free (mh);
    }

    return 0;
}
