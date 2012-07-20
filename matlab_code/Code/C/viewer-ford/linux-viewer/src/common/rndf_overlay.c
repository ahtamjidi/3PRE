#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <assert.h>

#include <common/mdf.h>
#include <common/geometry.h>
#include <common/gps_linearize.h>
#include <common/small_linalg.h>
#include <common/math_util.h>

#include "rndf_overlay.h"

#if 0
#define dbg(...) do { fprintf (stderr, "[%s:%d] ", __FILE__, __LINE__); \
    fprintf (stderr, __VA_ARGS__); } while(0)
#else
#define dbg(...) 
#endif

static void
copy_waypoint (RndfOverlay * r, RndfOverlayParent p, RndfOverlayWaypoint * w,
        RndfWaypoint * rndf_w, RndfOverlayWaypoint * prev)
{
    w->type = rndf_w->type;
    w->parent = p;
    w->is_stop = rndf_w->is_stop;
    w->waypoint = rndf_w;
    rndf_get_waypoint_id (rndf_w, w->id, w->id + 1, w->id + 2);
    snprintf (w->id_str, sizeof (w->id_str), 
            "%d.%d.%d", w->id[0], w->id[1], w->id[2]);
    g_hash_table_insert (r->waypoint_hash, w->id, w);

    w->prev = prev;
    w->expect_left_boundary_next = 0;
    w->expect_right_boundary_next = 0;
    w->expect_left_boundary_prev = 1;
    w->expect_right_boundary_prev = 1;
    if (prev) {
        w->prev->next = w;
        w->prev->expect_left_boundary_next = 1;
        w->prev->expect_right_boundary_next = 1;
    } else {
        w->expect_left_boundary_prev = 0;
        w->expect_right_boundary_prev = 0;
    }
}

static guint
_relative_placement_key_hash (gconstpointer key)
{
    const RndfOverlayLane *olane = key;
    return g_int_hash (&olane->parent->segment->id) ^ 
        g_int_hash (&olane->lane->id);
}

static int
_relative_placement_key_equals (gconstpointer a, gconstpointer b)
{
    const RndfOverlayLane *olane_a = a;
    const RndfOverlayLane *olane_b = b;

    return (olane_a->parent->segment->id == olane_b->parent->segment->id) && 
           (olane_a->lane->id == olane_b->lane->id);
}

static RndfOverlayLaneRelativePlacement *
_lane_relative_placement_new (RndfOverlayRelativePosition rpos, 
        RndfOverlayRelativeDirection rdir)
{
    RndfOverlayLaneRelativePlacement *lrp = 
        g_slice_new (RndfOverlayLaneRelativePlacement);
    lrp->rpos = rpos;
    lrp->rdir = rdir;
    return lrp;
}

static void 
_lane_relative_placement_free (RndfOverlayLaneRelativePlacement *lrp)
{
    g_slice_free (RndfOverlayLaneRelativePlacement, lrp);
}

static void
copy_lane (RndfOverlaySegment * s, RndfOverlayLane * olane, RndfLane * rndf_l)
{
    olane->parent = s;
    olane->lane = rndf_l;
    olane->num_waypoints = rndf_l->num_waypoints;
    olane->waypoints = calloc (rndf_l->num_waypoints, 
            sizeof (RndfOverlayWaypoint));
    olane->relative_positions = 
        g_hash_table_new_full (_relative_placement_key_hash, 
            _relative_placement_key_equals, NULL, 
            (GDestroyNotify) _lane_relative_placement_free);
    for (int i = 0; i < rndf_l->num_waypoints; i++)
        copy_waypoint (s->parent, (RndfOverlayParent) olane, 
                olane->waypoints + i,
                rndf_l->waypoints + i, i > 0 ? olane->waypoints + i - 1 : NULL);
}

// computes the relative position of b with respect to a
static void
_compute_lane_relative_placement (RndfOverlayLane *a, RndfOverlayLane *b)
{
    assert (a != b);
    assert (a->num_waypoints > 1);

    // initialize a GPS linearization about a's first waypoint
    gps_linearize_t gpslin;
    RndfWaypoint *first_wp_a = a->waypoints[0].waypoint;
    double first_wp_a_ll[] = { first_wp_a->lat, first_wp_a->lon };
    gps_linearize_init (&gpslin, first_wp_a_ll);

    // build a pointlist of a's waypoints
    pointlist2d_t *pts_a = pointlist2d_new (a->num_waypoints);
    for (int i=0; i<pts_a->npoints; i++) {
        RndfOverlayWaypoint *owp_a = &a->waypoints[i];
        double latlon_a[2] = { owp_a->waypoint->lat, owp_a->waypoint->lon };
        gps_linearize_to_xy (&gpslin, latlon_a, (double*) &pts_a->points[i]);
    }

    // estimate which side of a that each point of b is on
    int nleft = 0;
    int nright = 0;
    int nsamedir = 0;
    int noppdir = 0;
    for (int i=0; i<b->num_waypoints; i++) {
        // linearize a waypoint of b
        RndfOverlayWaypoint *owp_b = &b->waypoints[i];
        double latlon_b[2] = { owp_b->waypoint->lat, owp_b->waypoint->lon };
        point2d_t xy_b;
        gps_linearize_to_xy (&gpslin, latlon_b, (double*) &xy_b);

        // find the closest segment
        int seg_ind = 0;
        point2d_t seg_closest_pt = { 0, 0 };
        geom_point_polyline_closest_point_2d (&xy_b, pts_a, &seg_ind, NULL, 
                &seg_closest_pt);
        vec2d_t adir;
        if (seg_ind >= pts_a->npoints - 1) {
            adir.x = pts_a->points[seg_ind].x - pts_a->points[seg_ind - 1].x;
            adir.y = pts_a->points[seg_ind].y - pts_a->points[seg_ind - 1].y;
        } else {
            adir.x = pts_a->points[seg_ind + 1].x - pts_a->points[seg_ind].x;
            adir.y = pts_a->points[seg_ind + 1].y - pts_a->points[seg_ind].y;
        }

        // if it's too far, then ignore this waypoint
        if (geom_point_point_distance_2d (&xy_b, &seg_closest_pt) > 
                50) continue;

        // which side of the segment is the waypoint of b on?
        int side = geom_handedness_2d (&pts_a->points[seg_ind], 
                &pts_a->points[seg_ind+1], &xy_b);
        if (side < 0) nleft++;
        if (side > 0) nright++;

        // estimate the direction of travel at waypoint of b
        if (b->num_waypoints < 2) continue;
        point2d_t xy_nb = xy_b;
        point2d_t xy_pb = xy_b;
        vec2d_t bdir = { 0, 0 };
        if (i > 0) {
            RndfOverlayWaypoint *powp_b = &b->waypoints[i-1];
            double latlon_pb[2] = { 
                powp_b->waypoint->lat, 
                powp_b->waypoint->lon 
            };
            gps_linearize_to_xy (&gpslin, latlon_pb, (double*) &xy_pb);
            vec2d_t pdir = { xy_b.x - xy_pb.x, xy_b.y - xy_pb.y };
            geom_vec_normalize_2d (&pdir);
            bdir.x = pdir.x / 2;
            bdir.y = pdir.y / 2;
        }
        if (i < b->num_waypoints - 1) {
            RndfOverlayWaypoint *nowp_b = &b->waypoints[i+1];
            double latlon_nb[2] = { 
                nowp_b->waypoint->lat, 
                nowp_b->waypoint->lon 
            };
            gps_linearize_to_xy (&gpslin, latlon_nb, (double*) &xy_nb);
            vec2d_t ndir = { xy_nb.x - xy_b.x, xy_nb.y - xy_b.y };
            geom_vec_normalize_2d (&ndir);
            bdir.x += ndir.x / 2;
            bdir.y += ndir.y / 2;
        }
        double dmag = geom_vec_magnitude_2d (&bdir);
        assert (dmag > 0);
        if (geom_vec_vec_dot_2d (&bdir, &adir) > 0) {
            nsamedir ++;
        } else {
            noppdir ++;
        }
    }
    pointlist2d_free (pts_a);
   const char *rpos_str[] = {
        "left", "right", "unknown"
    };
    RndfOverlayRelativePosition rpos = 
        (nleft > nright) ? RNDF_OVERLAY_RPOS_LEFT : RNDF_OVERLAY_RPOS_RIGHT;
    if (!nleft && !nright) {
        fprintf (stderr, "%s:%d Unable to determine position of lane %d.%d\n"
                " with respect to %d.%d (%s)\n", 
               __FILE__, __LINE__,
               b->parent->segment->id, b->lane->id,
               a->parent->segment->id, a->lane->id,
               a->parent->segment->name);
        rpos = RNDF_OVERLAY_RPOS_UNKNOWN;
    } else if ((nleft && nright) || (nleft == nright)) {
        fprintf (stderr, "%s:%d Position of lane %d.%d with respect to %d.%d\n"
               "  is ambiguous (%s).  Guessing %s. (%d / %d)\n",
               __FILE__, __LINE__,
               b->parent->segment->id, b->lane->id,
               a->parent->segment->id, a->lane->id,
               a->parent->segment->name,
               rpos_str[rpos], 
               nleft, nright);
    }

    RndfOverlayRelativeDirection rdir;
    if (!nsamedir && !noppdir) {
        fprintf (stderr, "%s:%d Unable to determine direction of lane %d.%d\n"
                " with respect to %d.%d (%s)\n", 
               __FILE__, __LINE__,
               b->parent->segment->id, b->lane->id,
               a->parent->segment->id, a->lane->id,
               a->parent->segment->name);
        rdir = RNDF_OVERLAY_RDIR_UNKNOWN;
    } else if (nsamedir > noppdir) {
        rdir = RNDF_OVERLAY_RDIR_SAME;
    } else {
        rdir = RNDF_OVERLAY_RDIR_OPPOSITE;
    }
    const char *rdir_str[] = { "same direction as", "opposite direction to", 
        "unknown direction" };
    if ((nsamedir && noppdir) || (nsamedir == noppdir)) {
        fprintf (stderr, "%s:%d Direction of lane %d.%d with respect to %d.%d\n"
                " is ambiguous (%s).  Guessing %s. (%d / %d)\n",
               __FILE__, __LINE__,
               b->parent->segment->id, b->lane->id,
               a->parent->segment->id, a->lane->id,
               a->parent->segment->name,
               rdir_str[rdir], 
               nsamedir, noppdir);
    } else {
        dbg ("%d.%d %s of, %s %d.%d\n", 
                b->parent->segment->id, b->lane->id,
                rpos_str[rpos], rdir_str[rdir],
                a->parent->segment->id, a->lane->id);
    }

    RndfOverlayLaneRelativePlacement *rp = 
        _lane_relative_placement_new (rpos, rdir);
    assert (!g_hash_table_lookup (a->relative_positions, b));
    g_hash_table_insert (a->relative_positions, b, rp);
}

static void
copy_segment (RndfOverlay * r, RndfOverlaySegment * s, RndfSegment * rndf_s)
{
    s->parent = r;
    s->segment = rndf_s;
    s->num_lanes = rndf_s->num_lanes;
    s->lanes = calloc (rndf_s->num_lanes, sizeof (RndfOverlayLane));
    dbg ("(%d) %s\n", s->segment->id, s->segment->name);
    int i;
    for (i = 0; i < rndf_s->num_lanes; i++)
        copy_lane (s, s->lanes + i, rndf_s->lanes + i);

    for (i = 0; i < rndf_s->num_lanes; i++) {
        RndfOverlayLane *olane = &s->lanes[i];

        if (olane->num_waypoints < 2 || s->num_lanes < 2) continue;

        for (int j = 0; j < s->num_lanes; j++) {
            if (i == j) continue;

            _compute_lane_relative_placement (olane, &s->lanes[j]);
        }
    }
}

static void
copy_spot (RndfOverlayZone * z, RndfOverlaySpot * s, RndfSpot * rndf_s)
{
    s->parent = z;
    s->spot = rndf_s;
    copy_waypoint (z->parent, (RndfOverlayParent) s, s->waypoints,
            rndf_s->waypoints, NULL);
    copy_waypoint (z->parent, (RndfOverlayParent) s, s->waypoints + 1,
            rndf_s->waypoints + 1, s->waypoints);
}

static void
copy_zone (RndfOverlay * r, RndfOverlayZone * z, RndfZone * rndf_z)
{
    z->parent = r;
    z->zone = rndf_z;
    z->num_spots = rndf_z->num_spots;
    z->spots = calloc (rndf_z->num_spots, sizeof (RndfOverlaySpot));
    int i;
    for (i = 0; i < rndf_z->num_spots; i++)
        copy_spot (z, z->spots + i, rndf_z->spots + i);
    z->num_peripoints = rndf_z->num_peripoints;
    z->peripoints = calloc (rndf_z->num_peripoints,
            sizeof (RndfOverlayWaypoint));
    for (i = 0; i < rndf_z->num_peripoints; i++)
        copy_waypoint (r, (RndfOverlayParent) z, z->peripoints + i,
                rndf_z->peripoints + i, i > 0 ? z->peripoints + i - 1 : NULL);
    if (z->num_peripoints) {
        z->peripoints[0].prev = z->peripoints + z->num_peripoints - 1;
        z->peripoints[z->num_peripoints-1].next = z->peripoints;
    }
}

static guint
hash_waypoint_id (gconstpointer key)
{
    const int32_t * id = key;
    gint val = ((id[0] & 0x3ff) << 22) | ((id[1] & 0xff) << 14) |
        (id[2] & 0x3fff);
    return g_int_hash (&val);
}

static gboolean
equal_waypoint_id (gconstpointer a, gconstpointer b)
{
    const int32_t * id1 = a;
    const int32_t * id2 = b;
    return id1[0] == id2[0] && id1[1] == id2[1] && id1[2] == id2[2];
}

static guint
hash_lane_id (gconstpointer key)
{
    const int32_t * id = key;
    gint val = ((id[0] & 0x3ff) << 22) | ((id[1] & 0xff) << 14);
    return g_int_hash (&val);
}

static gboolean
equal_lane_id (gconstpointer a, gconstpointer b)
{
    const int32_t * id1 = a;
    const int32_t * id2 = b;
    return id1[0] == id2[0] && id1[1] == id2[1];
}

RndfOverlayWaypoint *
rndf_overlay_find_waypoint_by_id (RndfOverlay * r, const int32_t id[3])
{
    return g_hash_table_lookup (r->waypoint_hash, id);
}

RndfOverlayLane *
rndf_overlay_find_lane_by_id (RndfOverlay *r, const int32_t id[2]) {
    return g_hash_table_lookup (r->lane_hash, id);
}

static void
copy_checkpoint (RndfOverlay * r, RndfOverlayCheckpoint * c,
        RndfCheckpoint * rndf_c)
{
    c->checkpoint = rndf_c;
    int id[3];
    rndf_get_waypoint_id (rndf_c->waypoint, id, id+1, id+2);
    c->waypoint = rndf_overlay_find_waypoint_by_id (r, id);
    assert (c->waypoint);
}

static void
add_entrance (RndfOverlayWaypoint * w, RndfOverlayWaypoint * from)
{
    int n = w->num_entrances;
    w->entrances = realloc (w->entrances,
            (n+1) * sizeof (RndfOverlayWaypoint *));
    w->entrances[n] = from;
    w->num_entrances = n + 1;
}

static void
add_exits (gpointer key, gpointer value, gpointer user)
{
    RndfOverlay * r = user;
    RndfOverlayWaypoint * w = value;
    RndfWaypoint * rndf_w = w->waypoint;

    w->num_exits = rndf_w->num_exits;
    w->exits = calloc (rndf_w->num_exits, sizeof (RndfOverlayWaypoint *));
    w->exit_yields = calloc (rndf_w->num_exits, sizeof (GList *));
    int i;
    for (i = 0; i < rndf_w->num_exits; i++) {
        int id[3];
        RndfWaypoint * exit_w = rndf_w->exits[i];
        rndf_get_waypoint_id (exit_w, id, id+1, id+2);
        w->exits[i] = rndf_overlay_find_waypoint_by_id (r, id);
        add_entrance (w->exits[i], w);
    }
}

typedef struct _SegmentCache {
    RndfOverlayWaypoint * w1, * w2;
    double min[2], max[2];
    double p1[2], p2[2];
} SegmentCache;

typedef struct _IntersectionState {
    RndfOverlay * rndf;
    gps_linearize_t gpslin;
    // data: SegmentCache
    GArray * segments_stopline;
    // data: SegmentCache
    GArray * segments_no_stop;
} IntersectionState;

static void
make_segment_cache (gps_linearize_t * gpslin, SegmentCache * segment,
        RndfOverlayWaypoint * w1, RndfOverlayWaypoint * w2)
{
    segment->w1 = w1;
    segment->w2 = w2;
    double ll1[2] = { w1->waypoint->lat, w1->waypoint->lon };
    gps_linearize_to_xy (gpslin, ll1, segment->p1);
    double ll2[2] = { w2->waypoint->lat, w2->waypoint->lon };
    gps_linearize_to_xy (gpslin, ll2, segment->p2);

    segment->min[0] = MIN (segment->p1[0], segment->p2[0]);
    segment->min[1] = MIN (segment->p1[1], segment->p2[1]);
    segment->max[0] = MAX (segment->p1[0], segment->p2[0]);
    segment->max[1] = MAX (segment->p1[1], segment->p2[1]);
}

#if 0
static double
segment_length (gps_linearize_t * gpslin,
        RndfOverlayWaypoint * w1, RndfOverlayWaypoint * w2)
{
    double xy1[2], xy2[2];
    double ll1[2] = { w1->waypoint->lat, w1->waypoint->lon };
    gps_linearize_to_xy (gpslin, ll1, xy1);
    double ll2[2] = { w2->waypoint->lat, w2->waypoint->lon };
    gps_linearize_to_xy (gpslin, ll2, xy2);
    return vector_dist_2d (xy1, xy2);
}
#endif

static GList *
find_intersecting_segments (SegmentCache * match, GArray * list)
{
    GList * result = NULL;
    int i;
    for (i = 0; i < list->len; i++) {
        SegmentCache * c = &g_array_index (list, SegmentCache, i);
        if (c->min[0] >= match->max[0] || c->max[0] <= match->min[0] ||
                c->min[1] >= match->max[1] || c->max[1] <= match->min[1])
            continue;
        if (c->w1 == match->w1 || c->w1 == match->w2 || c->w2 == match->w1 ||
                c->w2 == match->w2)
            continue;
        if (geom_line_seg_line_seg_intersect_test_2d (POINT2D (c->p1),
                    POINT2D (c->p2), POINT2D (match->p1), POINT2D (match->p2)))
            result = g_list_prepend (result, c);
    }
    return result;
}

static void
add_segment (IntersectionState * s, RndfOverlayWaypoint * w1,
        RndfOverlayWaypoint * w2, GArray * list)
{
    SegmentCache segment;
    make_segment_cache (&s->gpslin, &segment, w1, w2);
    g_array_append_val (list, segment);
}

static void
add_segment_cache (gpointer key, gpointer value, gpointer user)
{
    IntersectionState * s = user;
    RndfOverlayWaypoint * w = value;

    if (!s->segments_stopline) {
        s->segments_stopline = g_array_new (FALSE, FALSE,
                sizeof (SegmentCache));
        s->segments_no_stop = g_array_new (FALSE, FALSE,
                sizeof (SegmentCache));
        double ll[2] = { w->waypoint->lat, w->waypoint->lon };
        gps_linearize_init (&s->gpslin, ll);
    }

    GArray * list = s->segments_no_stop;
    if (w->is_stop)
        list = s->segments_stopline;

    for (int i = 0; i < w->num_exits; i++)
        add_segment (s, w, w->exits[i], list);

    if (w->type == RNDF_POINT_WAYPOINT && w->next)
        add_segment (s, w, w->next, list);
}

static void
add_yield (GList ** list, RndfOverlayWaypoint * w,
        RndfOverlayWaypoint * w_next, double d)
{
    RndfOverlayYield * y = g_slice_new (RndfOverlayYield);
    y->w = w;
    y->w_next = w_next;
    y->d = d;
    *list = g_list_prepend (*list, y);
}

static GList *
get_yields (IntersectionState * s, RndfOverlayWaypoint * w,
        RndfOverlayWaypoint * w2)
{
    GList * yields = NULL;
    if (w2->type == RNDF_POINT_WAYPOINT && w2->prev &&
            !w2->prev->is_stop && w2->prev != w) {
        add_yield (&yields, w2, NULL, 0);
        //printf ("%s %s prev yield\n", w->id_str, w2->id_str);
    }
    for (int j = 0; j < w2->num_entrances; j++)
        if (w2->entrances[j] != w && !w2->entrances[j]->is_stop) {
            add_yield (&yields, w2->entrances[j], w2, 1);
            //printf ("%s %s entrance %d yield\n", w->id_str, w2->id_str, j);
        }
    SegmentCache segment;
    make_segment_cache (&s->gpslin, &segment, w, w2);
    GList * list = find_intersecting_segments (&segment,
            s->segments_no_stop);
    for (GList * iter = list; iter; iter = iter->next) {
        SegmentCache * c = iter->data;
        double p[2], u;
        if (geom_line_seg_line_seg_intersect_2d (POINT2D (c->p1),
                    POINT2D (c->p2), POINT2D (segment.p1),
                    POINT2D (segment.p2), POINT2D (p)) == 0)
            continue;
        geom_point_line_closest_point_2d (POINT2D (p),
                POINT2D (c->p1), POINT2D (c->p2), NULL, &u);
        add_yield (&yields, c->w1, c->w2, u);
        //printf ("%s %s cross yield %s %s\n", w->id_str, w2->id_str,
        //        c->w1->id_str, c->w2->id_str);
    }
    g_list_free (list);
    return yields;
}

static void
add_yields (gpointer key, gpointer value, gpointer user)
{
    IntersectionState * s = user;
    RndfOverlayWaypoint * w = value;

    for (int i = 0; i < w->num_exits; i++)
        w->exit_yields[i] = get_yields (s, w, w->exits[i]);
    if (w->is_stop && w->next)
        w->next_yields = get_yields (s, w, w->next);
}

static void
add_to_intersection (IntersectionState * s, RndfOverlayWaypoint * w,
        RndfOverlayIntersection * inter);

static void
add_crossings_to_intersection (IntersectionState * s, RndfOverlayWaypoint * w1,
        RndfOverlayWaypoint * w2, RndfOverlayIntersection * inter)
{
    add_to_intersection (s, w2, inter);
    SegmentCache segment;
    make_segment_cache (&s->gpslin, &segment, w1, w2);
    GList * list = find_intersecting_segments (&segment, s->segments_stopline);
    for (GList * iter = list; iter; iter = iter->next) {
        SegmentCache * seg = iter->data;
        add_to_intersection (s, seg->w1, inter);
    }
    g_list_free (list);
}

static void
add_to_intersection (IntersectionState * s, RndfOverlayWaypoint * w,
        RndfOverlayIntersection * inter)
{
    if (w->intersection)
        return;
    w->intersection = inter;
    inter->waypoints = g_list_prepend (inter->waypoints, w);
    int i;
    for (i = 0; i < w->num_exits; i++)
        add_crossings_to_intersection (s, w, w->exits[i], inter);
    for (i = 0; i < w->num_entrances; i++)
        add_crossings_to_intersection (s, w, w->entrances[i], inter);
    if (w->type == RNDF_POINT_WAYPOINT) {
        if (w->prev && w->prev->is_stop)
            add_crossings_to_intersection (s, w, w->prev, inter);
        if (w->next && w->is_stop)
            add_crossings_to_intersection (s, w, w->next, inter);
    }
}

static void
add_intersections (gpointer key, gpointer value, gpointer user)
{
    IntersectionState * s = user;
    RndfOverlayWaypoint * w = value;

    if (w->intersection)
        return;
    if (w->num_exits == 0 && w->num_entrances == 0 && !w->is_stop &&
            (!w->prev || !w->prev->is_stop))
        return;

    RndfOverlayIntersection * inter = calloc (1,
            sizeof (RndfOverlayIntersection));
    add_to_intersection (s, w, inter);
    
    s->rndf->intersections = g_list_prepend (s->rndf->intersections, inter);
}

static void
waypoint_destroy (gpointer key, gpointer value, gpointer user)
{
    RndfOverlayWaypoint * w = value;
    for (int i = 0; i < w->num_exits; i++) {
        for (GList * iter = w->exit_yields[i]; iter; iter = iter->next)
            g_slice_free (RndfOverlayYield, iter->data);
        g_list_free (w->exit_yields[i]);
    }
    free (w->exit_yields);
    for (GList * iter = w->next_yields; iter; iter = iter->next)
        g_slice_free (RndfOverlayYield, iter->data);
    g_list_free (w->next_yields);
    free (w->exits);
    free (w->entrances);
}

// computes the angle between AB and AC
static double
_check_waypoint_angle (gps_linearize_t *gpslin, RndfOverlayWaypoint *wp_a, 
        RndfOverlayWaypoint *wp_b, RndfOverlayWaypoint *wp_c)
{
    // linearize waypoint coordinates
    double latlon_a[2] = { wp_a->waypoint->lat, wp_a->waypoint->lon };
    double latlon_b[2] = { wp_b->waypoint->lat, wp_b->waypoint->lon };
    double latlon_c[2] = { wp_c->waypoint->lat, wp_c->waypoint->lon };
    double xy_a[2], xy_b[2], xy_c[2];
    gps_linearize_to_xy (gpslin, latlon_a, xy_a);
    gps_linearize_to_xy (gpslin, latlon_b, xy_b);
    gps_linearize_to_xy (gpslin, latlon_c, xy_c);
    vec2d_t ab = { xy_b[0] - xy_a[0], xy_b[1] - xy_a[1], };
    vec2d_t ac = { xy_c[0] - xy_a[0], xy_c[1] - xy_a[1], };
    return geom_vec_vec_angle_2d (&ab, &ac);
}

static int
_waypoint_edge_intersect_test (gps_linearize_t *gpslin, 
        RndfOverlayWaypoint *wp_a1, RndfOverlayWaypoint *wp_a2,
        RndfOverlayWaypoint *wp_b1, RndfOverlayWaypoint *wp_b2)
{
    double latlon_a1[2] = { wp_a1->waypoint->lat, wp_a1->waypoint->lon };
    double latlon_a2[2] = { wp_a2->waypoint->lat, wp_a2->waypoint->lon };
    double latlon_b1[2] = { wp_b1->waypoint->lat, wp_b1->waypoint->lon };
    double latlon_b2[2] = { wp_b2->waypoint->lat, wp_b2->waypoint->lon };
    double xy_a1[2], xy_a2[2], xy_b1[2], xy_b2[2];
    gps_linearize_to_xy (gpslin, latlon_a1, xy_a1);
    gps_linearize_to_xy (gpslin, latlon_a2, xy_a2);
    gps_linearize_to_xy (gpslin, latlon_b1, xy_b1);
    gps_linearize_to_xy (gpslin, latlon_b2, xy_b2);
    return geom_line_seg_line_seg_intersect_test_2d (POINT2D (xy_a1), 
            POINT2D (xy_a2), POINT2D (xy_b1), POINT2D (xy_b2));
}

static inline GPtrArray *
_get_waypoint_exits_and_entrances (RndfOverlayWaypoint *wp)
{
    GPtrArray *result = 
        g_ptr_array_sized_new (wp->num_exits + wp->num_entrances);
    for (int i=0; i<wp->num_entrances; i++) {
        RndfOverlayWaypoint *ewp = wp->entrances[i];
        g_ptr_array_add (result, ewp);
    }
    for (int i=0; i<wp->num_exits; i++) {
        RndfOverlayWaypoint *ewp = wp->exits[i];
        g_ptr_array_add (result, ewp);
    }
    return result;
}

static inline GPtrArray *
_get_waypoint_all_edges (RndfOverlayWaypoint *wp)
{
    GPtrArray *result = _get_waypoint_exits_and_entrances (wp);
    if (wp->next) g_ptr_array_add (result, wp->next);
    if (wp->prev) g_ptr_array_add (result, wp->prev);
    return result;
}

static void
compute_lane_boundary_expectations (RndfOverlay * r)
{
    for (GList *iter=r->intersections; iter; iter=iter->next) {
        RndfOverlayIntersection *isect = iter->data;

        RndfOverlayWaypoint *first_owp = isect->waypoints->data;
        double first_wp_ll[2] = { first_owp->waypoint->lat, 
            first_owp->waypoint->lon };
        gps_linearize_t gpslin;
        gps_linearize_init (&gpslin, first_wp_ll);

        for (GList *wpiter=isect->waypoints; wpiter; wpiter=wpiter->next) {
            RndfOverlayWaypoint *owp = wpiter->data;

            GPtrArray *edges = _get_waypoint_exits_and_entrances (owp);

            for (int i=0; i<edges->len; i++) {
                RndfOverlayWaypoint *ewp = g_ptr_array_index (edges, i);
                if (owp->next) {
                    double theta = _check_waypoint_angle (&gpslin, owp,
                            owp->next, ewp);
                    if (theta > 0 && theta < M_PI / 2) {
                        owp->expect_left_boundary_next = 0;
                    } else if (theta <= 0 && theta > -M_PI / 2) {
                        owp->expect_right_boundary_next = 0;
                    }
                }
                if (owp->prev) {
                    double theta = _check_waypoint_angle (&gpslin, owp,
                            owp->prev, ewp);
                    if (theta > 0 && theta < M_PI/2) {
//                        owp->prev->expect_right_boundary_next = 0;
                        owp->expect_right_boundary_prev = 0;
                    } else if (theta <= 0 && theta > -M_PI / 2) {
//                        owp->prev->expect_left_boundary_next = 0;
                        owp->expect_left_boundary_prev = 0;
                    }
                }
            }
            g_ptr_array_free (edges, TRUE);

            for (GList *siter=isect->waypoints; 
                    owp->next && siter && (owp->expect_left_boundary_next || 
                        owp->expect_right_boundary_next); 
                    siter=siter->next) {
                RndfOverlayWaypoint *swp = siter->data;
                if (swp == owp || swp == owp->next) continue;
                GPtrArray *swp_edges = _get_waypoint_all_edges (swp);
                for (int j=0; j<swp_edges->len; j++) {
                    RndfOverlayWaypoint *swp_e = 
                        g_ptr_array_index (swp_edges, j);
                    if (swp_e == owp || swp_e == owp->next) continue;
                    if (_waypoint_edge_intersect_test (&gpslin, owp, owp->next, 
                                swp, swp_e)) {
                        dbg ("   <%s - %s> isects <%s - %s>\n", 
                                owp->id_str, owp->next->id_str,
                                swp->id_str, swp_e->id_str);
                        owp->expect_left_boundary_next = 0;
                        owp->expect_right_boundary_next = 0;
                        break;
                    }
                }
                g_ptr_array_free (swp_edges, TRUE);
            }
        }
    }
}

static void
add_unit_direction_vector (double v[2], const double xy1[2],
        const double xy2[2], double scale)
{
    double xy[2];
    vector_sub_nd (xy2, xy1, 2, xy);
    vector_normalize_2d (xy);
    v[0] += scale * xy[0];
    v[1] += scale * xy[1];
}

static void
add_rndf_unit_direction_vector (gps_linearize_t * gpslin, double v[2],
        const double xy[2], RndfOverlayWaypoint * w, double scale)
{
    double ll[2] = { w->waypoint->lat, w->waypoint->lon };
    double wxy[2];
    gps_linearize_to_xy (gpslin, ll, wxy);
    add_unit_direction_vector (v, xy, wxy, scale);
}

static int
find_next_change_waypoint (gps_linearize_t * gpslin, double xy[2],
        double theta, RndfOverlayLane * lane, int i)
{
    if (i < 0 || !lane)
        return -1;

    while (i < lane->num_waypoints) {
        RndfOverlayWaypoint * w = lane->waypoints + i;
        double ll[2] = { w->waypoint->lat, w->waypoint->lon };
        double wxy[2];
        gps_linearize_to_xy (gpslin, ll, wxy);
        double wtheta = atan2 (wxy[1] - xy[1], wxy[0] - xy[0]);
        if (fabs (mod2pi (theta - wtheta)) < M_PI / 4)
            return i;
        i++;
    }
    return -1;
}

static void
compute_waypoint_change_pointers (RndfOverlay * r, RndfOverlayLane * lane,
        RndfOverlayLane * left, RndfOverlayLane * right)
{
    gps_linearize_t gpslin;
    int left_idx = left ? 0 : -1;
    int right_idx = right ? 0 : -1;
    if (lane->num_waypoints <= 1)
        return;
    for (int i = 0; i < lane->num_waypoints; i++) {
        RndfOverlayWaypoint * w = lane->waypoints + i;
        double ll[2] = { w->waypoint->lat, w->waypoint->lon };
        if (i == 0)
            gps_linearize_init (&gpslin, ll);

        double xy[2];
        gps_linearize_to_xy (&gpslin, ll, xy);

        double v[2] = { 0, 0 };
        if (w->next)
            add_rndf_unit_direction_vector (&gpslin, v, xy,
                    w->next, 1);
        if (w->prev)
            add_rndf_unit_direction_vector (&gpslin, v, xy,
                    w->prev, -1);

        double theta = atan2 (v[1], v[0]);
        left_idx = find_next_change_waypoint (&gpslin, xy, theta, left,
                left_idx);
        right_idx = find_next_change_waypoint (&gpslin, xy, theta, right,
                right_idx);
        if (left_idx >= 0)
            w->change_left = left->waypoints + left_idx;
        if (right_idx >= 0)
            w->change_right = right->waypoints + right_idx;
    }
}

static void
compute_lane_change_pointers (RndfOverlay * r, RndfOverlaySegment * s)
{
    for (int i = 0; i < s->num_lanes; i++) {
        RndfOverlayLane * lane = s->lanes + i;
        RndfOverlayLane * lane_left = i > 0 ? s->lanes + i - 1 : NULL;
        RndfOverlayLane * lane_right =
            i < s->num_lanes - 1 ? s->lanes + i + 1 : NULL;

        RndfOverlayLaneRelativePlacement * left_place = lane_left ?
            g_hash_table_lookup (lane->relative_positions, lane_left) : NULL;
        RndfOverlayLaneRelativePlacement * right_place = lane_right ?
            g_hash_table_lookup (lane->relative_positions, lane_right) : NULL;

        if ((left_place && left_place->rpos == RNDF_OVERLAY_RPOS_RIGHT) ||
                (right_place && right_place->rpos == RNDF_OVERLAY_RPOS_LEFT)) {
            RndfOverlayLane * tmp = lane_left;
            lane_left = lane_right;
            lane_right = tmp;
            RndfOverlayLaneRelativePlacement * tmp_place = left_place;
            left_place = right_place;
            right_place = tmp_place;
        }
        lane->adjacent_left = lane_left;
        lane->adjacent_right = lane_right;
        lane->adjacent_left_rdir = left_place?left_place->rdir:RNDF_OVERLAY_RDIR_UNKNOWN;
        lane->adjacent_right_rdir = right_place?right_place->rdir:RNDF_OVERLAY_RDIR_UNKNOWN;

        if (left_place && left_place->rdir != RNDF_OVERLAY_RDIR_SAME)
            lane_left = NULL;
        if (right_place && right_place->rdir != RNDF_OVERLAY_RDIR_SAME)
            lane_right = NULL;

        if (lane->lane->left_boundary != RNDF_BOUND_BROKEN_WHITE)
            lane_left = NULL;
        if (lane->lane->right_boundary != RNDF_BOUND_BROKEN_WHITE)
            lane_right = NULL;

        compute_waypoint_change_pointers (r, lane, lane_left, lane_right);
        lane->change_left = lane_left;
        lane->change_right = lane_right;
    }
}

RndfOverlay *
rndf_overlay_new_from_file (const char * file)
{
    RndfRouteNetwork * rndf = rndf_new_from_file (file);
    if (!rndf)
        return NULL;

    RndfOverlay * r = rndf_overlay_new (rndf);
    r->own_rndf_ptr = 1;
    return r;
}

RndfOverlay *
rndf_overlay_new (RndfRouteNetwork * rndf)
{
    RndfOverlay * r = calloc (1, sizeof (RndfOverlay));

    r->rndf = rndf;

    r->waypoint_hash = g_hash_table_new (hash_waypoint_id, equal_waypoint_id);
    r->lane_hash = g_hash_table_new (hash_lane_id, equal_lane_id);

    int i;
    r->num_segments = rndf->num_segments;
    r->segments = calloc (rndf->num_segments, sizeof (RndfOverlaySegment));
    for (i = 0; i < rndf->num_segments; i++)
        copy_segment (r, r->segments + i, rndf->segments + i);

    r->num_zones = rndf->num_zones;
    r->zones = calloc (rndf->num_zones, sizeof (RndfOverlayZone));
    for (i = 0; i < rndf->num_zones; i++)
        copy_zone (r, r->zones + i, rndf->zones + i);

    r->num_checkpoints = rndf->num_checkpoints;
    r->checkpoints = calloc (rndf->num_checkpoints,
            sizeof (RndfOverlayCheckpoint));
    for (i = 0; i < rndf->num_checkpoints; i++)
        copy_checkpoint (r, r->checkpoints + i, rndf->checkpoints + i);

    g_hash_table_foreach (r->waypoint_hash, add_exits, r);

    IntersectionState inter_state;
    memset (&inter_state, 0, sizeof (inter_state));
    inter_state.rndf = r;
    g_hash_table_foreach (r->waypoint_hash, add_segment_cache, &inter_state);
    g_hash_table_foreach (r->waypoint_hash, add_intersections, &inter_state);
    g_hash_table_foreach (r->waypoint_hash, add_yields, &inter_state);
    g_array_free (inter_state.segments_no_stop, TRUE);
    g_array_free (inter_state.segments_stopline, TRUE);

    compute_lane_boundary_expectations (r);

    for (i = 0; i < r->num_segments; i++)
        compute_lane_change_pointers (r, r->segments + i);

    // fill in lane hash
    for (int segidx = 0; segidx < r->num_segments; segidx++) {
        RndfOverlaySegment *seg = &r->segments[segidx];

        for (int laneidx = 0; laneidx < seg->num_lanes; laneidx++) {
            int32_t *id = malloc(2 * sizeof(int32_t));
            id[0] = seg->segment->id;
            id[1] = seg->lanes[laneidx].lane->id;
            g_hash_table_insert(r->lane_hash, id, &seg->lanes[laneidx]);
        }
    }

    return r;
}

void
rndf_overlay_free (RndfOverlay * r)
{
    for (GList * iter = r->intersections; iter; iter = iter->next) {
        RndfOverlayIntersection * inter = iter->data;
        g_list_free (inter->waypoints);
        free (inter);
    }
    g_list_free (r->intersections);

    g_hash_table_foreach (r->waypoint_hash, waypoint_destroy, r);
    g_hash_table_destroy (r->waypoint_hash);

    int i;
    for (i = 0; i < r->num_segments; i++) {
        RndfOverlaySegment * s = r->segments + i;
        int j;
        for (j = 0; j < s->num_lanes; j++) {
            free (s->lanes[j].waypoints);
            g_hash_table_destroy (s->lanes[j].relative_positions);
        }
        free (s->lanes);
    }
    free (r->segments);

    for (i = 0; i < r->num_zones; i++) {
        free (r->zones[i].spots);
        free (r->zones[i].peripoints);
    }
    free (r->zones);
    free (r->checkpoints);

    if (r->own_rndf_ptr)
        rndf_destroy (r->rndf);

    free (r);
}
