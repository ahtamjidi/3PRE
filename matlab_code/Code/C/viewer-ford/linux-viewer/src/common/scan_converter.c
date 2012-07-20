#include "scan_converter.h"
#include <assert.h>
#include <stdio.h>

struct _ScanConverter {
    int roi_x_min;
    int roi_x_max;
    int roi_y_min;
    int roi_y_max;
    int poly_y_max;
    int y;

    point2i_t *a0;
    point2i_t *a1;
    point2i_t *b0;
    point2i_t *b1;

    int a_ind;
    int a_ind_next;
    int b_ind;
    int b_ind_next;

    pointlist2i_t *p;
};

ScanConverter
*scan_converter_create ()
{
    ScanConverter *self = (ScanConverter*)calloc (1, sizeof(ScanConverter));
    self->p = NULL;
    return self;
}

void
scan_converter_destroy (ScanConverter *self)
{
    if (self->p)
        pointlist2i_free (self->p);
    free (self);
}


void
scan_converter_init (ScanConverter *self,
                     const pointlist2d_t *poly,
                     const int x_min, const int x_max,
                     const int y_min, const int y_max)
{
    self->roi_x_min = x_min;
    self->roi_x_max = x_max;
    self->roi_y_min = y_min;
    self->roi_y_max = y_max;

    if (self->p)
        pointlist2i_free (self->p);
    pointlist2i_t *p = pointlist2i_new (poly->npoints);
    for (int i = 0; i < p->npoints; ++i) {
        p->points[i].x = (int)floor(poly->points[i].x + 0.5);
        p->points[i].y = (int)floor(poly->points[i].y + 0.5);
    }
    self->p = p;

    int min_pt_ind = -1;
    int max_pt_ind = -1;
    const point2i_t *min_pt = NULL;
    const point2i_t *max_pt = NULL;

    // find the points with extremal y-coordinates in the polygon, breaking
    // ties with x-coordinate
    for (int i=0; i<p->npoints; i++) {
        if (min_pt_ind < 0 || 
            (p->points[i].y < min_pt->y ||
             (p->points[i].y == min_pt->y &&
              p->points[i].x < min_pt->x))) {
            min_pt_ind = i;
            min_pt = &p->points[i];
        }
        if (max_pt_ind < 0 ||
            (p->points[i].y > max_pt->y ||
             (p->points[i].y == max_pt->y &&
              p->points[i].x > max_pt->x))) {
            max_pt_ind = i;
            max_pt = &p->points[i];
        }
    }

    // since the polygon is convex, we're guaranteed that by traversing the
    // polygon from the point with minimal y coordinate to the point with
    // maximal y coordinate, (along either the left or right chain), the
    // y-coordinates of subsequent points will be monotonically non-decreasing.

    // TODO: while loops are probably not a good idea without
    // checks for degenerate cases that could cause infinite loops

    self->a_ind      = min_pt_ind;
    self->a_ind_next = (self->a_ind + 1) % p->npoints;
    while (p->points[self->a_ind].y == p->points[self->a_ind_next].y) {
        self->a_ind = self->a_ind_next;
        self->a_ind_next = (self->a_ind_next+1) % p->npoints;
    }

    self->b_ind      = min_pt_ind;
    self->b_ind_next = (p->npoints + self->b_ind - 1) % p->npoints;
    while (p->points[self->b_ind].y == p->points[self->b_ind_next].y) {
        self->b_ind = self->b_ind_next;
        self->b_ind_next = (p->npoints+self->b_ind_next-1) % p->npoints;
    }

    self->a0 = &p->points[self->a_ind];
    self->a1 = &p->points[self->a_ind_next];
    self->b0 = &p->points[self->b_ind];
    self->b1 = &p->points[self->b_ind_next];

    self->y = min_pt->y;
    self->poly_y_max = max_pt->y;
}

int
scan_converter_next (ScanConverter *self,
                     int *x_min, int *x_max, int *y, const int clip)
{

    *y = self->y;
    pointlist2i_t *p = self->p;

    // advance?
    if (*y > self->a1->y) {
        self->a_ind = self->a_ind_next;
        self->a_ind_next = (self->a_ind + 1) % p->npoints;
        while (p->points[self->a_ind].y == p->points[self->a_ind_next].y) {
            self->a_ind = self->a_ind_next;
            self->a_ind_next = (self->a_ind_next+1) % p->npoints;
        }
        self->a0 = &p->points[self->a_ind];
        self->a1 = &p->points[self->a_ind_next];
    }
    if (*y > self->b1->y) {
        self->b_ind = self->b_ind_next;
        self->b_ind_next = (p->npoints + self->b_ind - 1) % p->npoints;
        while (p->points[self->b_ind].y == p->points[self->b_ind_next].y) {
            self->b_ind = self->b_ind_next;
            self->b_ind_next = (p->npoints+self->b_ind_next-1) % p->npoints;
        }
        self->b0 = &p->points[self->b_ind];
        self->b1 = &p->points[self->b_ind_next];
    }

    assert (self->a1->y >= *y && self->b1->y >= *y && self->a0->y <= *y && self->b0->y <= *y);

    if (self->a0->y == self->a1->y) {
        // segment a is horizontal
        if (self->a0->x > self->a1->x) {
            *x_min = self->a1->x;
            *x_max = self->a0->x;
        } else {
            *x_min = self->a0->x;
            *x_max = self->a1->x;
        }
    } else if (self->b0->y == self->b1->y) {
        // segment b is horizontal
        if (self->b0->x > self->b1->x) {
            *x_min = self->b1->x;
            *x_max = self->b0->x;
        } else {
            *x_min = self->b0->x;
            *x_max = self->b1->x;
        }
    } else {
    
        // both a and b have nonzero slope

        // determine the x-coordinates where a and b intersect the
        // horizontal line at y

        // the intersection P of a line segment [P1, P2] with the line y=y0
        // where P1_y != P2_y can be given as
        //
        // P = P1 + u (P2 - P1)
        //
        //            y0 - P1_y
        // where u = -----------
        //           P2_y - P1_y

        double u_a = (*y - self->a0->y) / (double)(self->a1->y - self->a0->y);
        double u_b = (*y - self->b0->y) / (double)(self->b1->y - self->b0->y);
        double x_a = self->a0->x + u_a * (self->a1->x - self->a0->x);
        double x_b = self->b0->x + u_b * (self->b1->x - self->b0->x);

        if (x_a < x_b) {
            *x_min = (int) round (x_a);
            *x_max = (int) round (x_b);
        } else {
            *x_min = (int) round (x_b);
            *x_max = (int) round (x_a);
        }
    }

    self->y++;

    if (clip) {
        if ((*y < self->roi_y_min) || (*y > self->roi_y_max)) {
            return -1;
        }
        if ((*x_max < self->roi_x_min) || (*x_min > self->roi_x_max)) {
            return -1;
        }
        if (*x_min < self->roi_x_min) {
            *x_min = self->roi_x_min;
        }
        if (*x_max > self->roi_x_max) {
            *x_max = self->roi_x_max;
        }
    }

    return 0;
}

int
scan_converter_done (ScanConverter *self)
{
    return ((self->y > self->roi_y_max) || (self->y > self->poly_y_max));
}
