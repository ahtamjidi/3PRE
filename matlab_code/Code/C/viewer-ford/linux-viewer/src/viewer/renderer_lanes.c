#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <common/small_linalg.h>
#include <common/math_util.h>
#include <common/geometry.h>
#include <common/glib_util.h>
#include <common/hspline.h>
#include <common/gridmap.h>
#include <glutil/glutil.h>
#include <lcmtypes/lcmtypes_lane_list_t.h>
#include <lcmtypes/lcmtypes_lane_detection_t.h>
#include <lcmtypes/lcmtypes_lane_boundary_flag_t.h>
#include <lcmtypes/lcmtypes_lane_clines_t.h>
#include <lcmtypes/lcmtypes_gridmap_tile_t.h>

#include <dgc/globals.h>

#include <gtk_util/gtk_util.h>

#include <glutil/glutil.h>

#include "libviewer/viewer.h"
#include "dgcviewer.h"

#define MAX_NUM_DETECTIONS 500

#define PARAM_SHOW_DETECTIONS "Detections"
#define PARAM_SHOW_CLINES "Clines (lane_clines)"
#define PARAM_SHOW_CENTERLINES "Centerlines (tracked)"
#define PARAM_SHOW_BOUNDARIES "Boundaries (tracked)"
#define PARAM_SHOW_DETECT_MAP "Detection Map"

#define RENDERER_NAME "Lanes"

#define IN_BETWEEN(a, b, c) ((b <= a && a <= c))

typedef struct {
    GLUtilTexture *tex;
    double x0, x1;
    double y0, y1;
    double width;
    double height;
    int width_pix;
    int height_pix;
    uint8_t *data;
    int64_t utime;
    int64_t generation;
} tile_texture_t;

static tile_texture_t * tile_texture_new (const lcmtypes_gridmap_tile_t *tile) {
    tile_texture_t *tt = g_slice_new (tile_texture_t);
    tt->tex = glutil_texture_new (tile->width, tile->height, 
            tile->width * tile->height);
    tt->x0 = tile->x0;
    tt->y0 = tile->y0;
    tt->width = tile->width * tile->meters_per_pixel;
    tt->height = tile->height * tile->meters_per_pixel;
    tt->x1 = tt->x0 + tt->width;
    tt->y1 = tt->y0 + tt->height;
    tt->data = g_slice_alloc (tile->width * tile->height);
    tt->width_pix = tile->width;
    tt->height_pix = tile->height;
    tt->utime = tile->utime;
    tt->generation = tile->generation;
    gridmap_decode_base (tt->data, tile->width, tile->height, tile->data,
            tile->datalen);
    glutil_texture_set_internal_format (tt->tex, GL_ALPHA);
    glutil_texture_set_interp (tt->tex, GL_NEAREST);
    glutil_texture_upload (tt->tex, GL_ALPHA, GL_UNSIGNED_BYTE, 
            tile->width, tt->data);
    return tt;
}

static void tile_texture_destroy (tile_texture_t *tt) 
{
    glutil_texture_free (tt->tex);
    g_slice_free1 (tt->width_pix * tt->height_pix, tt->data);
    g_slice_free (tile_texture_t, tt);
}

typedef struct _RendererLanes RendererLanes;

struct _RendererLanes {
    Renderer renderer;

    CTrans *ctrans;

    GtkuParamWidget    *pw;

    lcm_t *lc;
    lcmtypes_lane_list_t_subscription_t *lc_handler_ll;
    lcmtypes_lane_detection_t_subscription_t *lc_handler_ld;
    lcmtypes_gridmap_tile_t_subscription_t *lc_handler_gm;
    lcmtypes_lane_clines_t_subscription_t *lc_handler_cl;

    lcmtypes_lane_list_t *lanes;
    lcmtypes_lane_clines_t *clines;

    // contains: lcmtypes_lane_detection_t
    GQueue *detections;

    GUSet *tiles;
    int64_t last_generation;

    Viewer *viewer;
};

static inline void
matrix_eigen_symm_2x2 (const double m[4], double v[4], double e[2], 
        double *theta)
{
    assert (m[1] == m[2]);
    double phi = 0.5 * atan2 (-2 * m[2], m[3] - m[0]);
    v[0] = -sin(phi);
    v[1] = cos(phi);
    v[2] = -v[1];
    v[3] = v[0];

    if (fabs (v[0]) < 1e-10) {
        e[0] = (m[2]*v[0] + m[3]*v[1]) / v[1];
    } else {
        e[0] = (m[0]*v[0] + m[1]*v[1]) / v[0];
    }
    if (fabs (v[2]) < 1e-10) {
        e[1] = (m[2]*v[2] + m[3]*v[3]) / v[3];
    } else {
        e[1] = (m[0]*v[2] + m[1]*v[3]) / v[2];
    }
    *theta = phi;
}

static void
draw_lane_boundary (RendererLanes *self, const lcmtypes_lane_boundary_t *boundary)
{
    assert (boundary->npoints > 1);

    glColor3f (0.7, 0.7, 0.7);
    glBegin (GL_LINES);
    for (int i=0; i<boundary->npoints - 1; i++) {
        if (boundary->edge_flags[i] & LCMTYPES_LANE_BOUNDARY_FLAG_T_VALID) {
            glVertex2d (boundary->points[i][0], boundary->points[i][1]);
            glVertex2d (boundary->points[i+1][0], boundary->points[i+1][1]);
        }
    }
    glEnd ();
}

#if 0
static void
draw_centerline_uncertainty (RendererLanes *self, const lcmtypes_lane_t *lane,
        double sigma_scale) 
{
    glBegin (GL_LINE_STRIP);
    for (int i=0; i<lane->ncenterpoints; i++) {
        point2d_t p = { lane->centerline[i][0], lane->centerline[i][1] };
        vec2d_t tan;
        if (0 == i) {
            tan.x = lane->centerline[i+1][0] - p.x;
            tan.y = lane->centerline[i+1][1] - p.y;
        } else {
            tan.x = p.x - lane->centerline[i-1][0];
            tan.y = p.y - lane->centerline[i-1][1];
        }
        geom_vec_normalize_2d (&tan);
        vec2d_t v = { -tan.y, tan.x };

        glVertex2d (p.x + v.x * lane->transverse_sigma[i] * sigma_scale, 
                p.y + v.y * lane->transverse_sigma[i] * sigma_scale);
    }
    glEnd ();
}
#endif

#if 0
static void
draw_centerline_cov (RendererLanes *self, const lcmtypes_lane_t *lane)
{
    for (int i=0; i<lane->ncenterpoints; i++) {
        double eigv[4];
        double eig[2];
        double theta;
        matrix_eigen_symm_2x2 (lane->centerline_cov[i], eigv, eig, &theta);
        double cx = lane->centerline[i][0];
        double cy = lane->centerline[i][1];

        glColor3f (0.3, 0.3, 0.3);
        glPushMatrix ();
        glTranslatef (cx, cy, 0);
        glutil_draw_ellipse (eig[0], eig[1], theta, 20);
        glPopMatrix ();
    }
}
#endif

static void
draw_lane (RendererLanes *self, const lcmtypes_lane_t *lane) 
{
    if (gtku_param_widget_get_bool (self->pw, PARAM_SHOW_BOUNDARIES)) {
        draw_lane_boundary (self, &lane->left_boundary);
        draw_lane_boundary (self, &lane->right_boundary);
    }
    if (gtku_param_widget_get_bool (self->pw, PARAM_SHOW_CENTERLINES)) {
        glColor3f (0.3, 0.3, 0.3);
        glBegin (GL_LINE_STRIP);
        for (int i=0; i<lane->ncenterpoints; i++) {
            glVertex2d (lane->centerline[i][0], lane->centerline[i][1]);
        }
        glEnd ();
        glPointSize (3);
        glBegin (GL_POINTS);
        for (int i=0; i<lane->ncenterpoints; i++) {
            glVertex2d (lane->centerline[i][0], lane->centerline[i][1]);
        }
        glEnd ();
        for (int i=0; i<lane->ncenterpoints; i++) {
            double v = lane->centerline_confidence[i];
            glColor3f (v, v, v);
            glPushMatrix ();
            glTranslatef (lane->centerline[i][0], lane->centerline[i][1], 0);
            glutil_draw_circle (0.2);
            glPopMatrix ();
        }
    }
}

static void
draw_lane_detection (RendererLanes *self, const lcmtypes_lane_detection_t *d)
{
    for (int i=0; i<d->num_splines; i++) {
        double b = d->scores[i];
        glColor3f (b, b, b);
//        int nsamples = 15;
//        double x[nsamples], y[nsamples];
        lcmtypes_spline_t *spline = &d->splines[i];
//        hspline_sample_points ((hspline_point_t*) spline->points,
//                spline->num_points, x, y, nsamples);
        glBegin (GL_LINE_STRIP);
        for (int j=0; j<spline->num_points; j++) {
            glVertex2f (spline->points[j][0], spline->points[j][1]);
        }
        glEnd ();
        glPointSize (4.0);
        glBegin (GL_POINTS);
        for (int j=0; j<spline->num_points; j++) {
            glVertex2f (spline->points[j][0], spline->points[j][1]);
        }
        glEnd ();
//        for (int j=0; j<nsamples; j++) {
//            glVertex2f (x[j], y[j]);
//        }
    }
}

static void
draw_clines (RendererLanes *self)
{
    if (!self->clines) return;

#if 0
    double colors[][3] = {
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 1 },
        { 1, 1, 0 },
        { 0, 1, 1 },
        { 1, 0, 1 },
        { 0.5, 0, 0 },
        { 0, 0.5, 0 },
        { 0, 0, 0.5 },
        { 0.5, 0.5, 0 },
        { 0, 0.5, 0.5 },
        { 0.5, 0, 0.5 }
    };
    int ncolors = sizeof (colors) / sizeof (double) / 3;
#endif
    for (int i=0; i<self->clines->nclines; i++) {
//        int color_ind = i % ncolors;
        glColor3f (0, 1, 1);

        lcmtypes_pointlist2d_t *pline = &self->clines->clines[i];
        
        glBegin (GL_LINE_STRIP);
        for (int j=0; j<pline->npoints; j++) {
            glVertex2d (pline->points[j].x, pline->points[j].y);
        }
        glEnd ();

        glPointSize (5.0);
        glBegin (GL_POINTS);
        for (int j=0; j<pline->npoints; j++) {
            glVertex2d (pline->points[j].x, pline->points[j].y);
        }
        glEnd ();
    }
}

static void
lanes_draw (Viewer *viewer, Renderer *renderer)
{
    RendererLanes *self = (RendererLanes*) renderer->user;

    glPushAttrib (GL_ENABLE_BIT);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    double carpos[3];
    ctrans_local_pos (self->ctrans, carpos);
    glPushMatrix ();
    glTranslatef (0, 0, carpos[2] - 0.05);

    // draw detection map
    if (gtku_param_widget_get_bool (self->pw, PARAM_SHOW_DETECT_MAP)) {
        GPtrArray *all_tiles = gu_set_get_elements (self->tiles);
        for (int i=0; i<all_tiles->len; i++) {
            tile_texture_t *tt = g_ptr_array_index (all_tiles, i);
            glPushMatrix ();
            glTranslated (tt->x0, tt->y0, 0);
            glScaled (tt->width, tt->height, 0);
            glColor4f (0.3, 0.3, 1.0, 1);
            glutil_texture_draw (tt->tex);
            glPopMatrix ();
        }
    }

    // draw lanes
    if (self->lanes) {
        for (int i=0; i<self->lanes->nlanes; i++) {
            draw_lane (self, &self->lanes->lanes[i]);
        }
    }

    // draw raw detections
    if (gtku_param_widget_get_bool (self->pw, PARAM_SHOW_DETECTIONS)) {
        for (GList *diter=g_queue_peek_head_link (self->detections);
                diter; diter=diter->next) {
            lcmtypes_lane_detection_t *d = (lcmtypes_lane_detection_t*) diter->data;
            draw_lane_detection (self, d);
        }
    }
    
    // draw clines
    if (gtku_param_widget_get_bool (self->pw, PARAM_SHOW_CLINES) && 
            self->clines) {
        draw_clines (self);
    }

    glPopMatrix ();

    glPopAttrib ();
}

static void
_trim_detections (RendererLanes *self)
{
    while (g_queue_get_length (self->detections) > MAX_NUM_DETECTIONS) {
        lcmtypes_lane_detection_t *d = 
            (lcmtypes_lane_detection_t*) g_queue_pop_head (self->detections);
        lcmtypes_lane_detection_t_destroy (d);
    }

    if (g_queue_is_empty (self->detections)) return;

    lcmtypes_lane_detection_t *last_det = 
        (lcmtypes_lane_detection_t*) g_queue_peek_tail (self->detections);

    int64_t max_age_usec = 1000000;
    int64_t min_keep_utime = last_det->utime - max_age_usec;

    while (!g_queue_is_empty (self->detections)) {
        lcmtypes_lane_detection_t *d = 
            (lcmtypes_lane_detection_t*) g_queue_peek_head (self->detections);
        if (d->utime < min_keep_utime ||
            d->utime > last_det->utime) {
            lcmtypes_lane_detection_t_destroy (d);
            g_queue_pop_head (self->detections);
        } else {
            break;
        }
    }
}

static void
on_lane_list (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_lane_list_t *lanes, 
        void *user_data)
{
    RendererLanes *self = user_data;
    if (self->lanes) lcmtypes_lane_list_t_destroy (self->lanes);
    self->lanes = lcmtypes_lane_list_t_copy (lanes);
    viewer_request_redraw (self->viewer);
}

static void
on_lane_detection (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_lane_detection_t *det,
        void *user_data)
{
    RendererLanes *self = user_data;
    g_queue_push_tail (self->detections, lcmtypes_lane_detection_t_copy (det));
    _trim_detections (self);
}

static void
on_lane_clines (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_lane_clines_t *clines,
        void *user_data)
{
    RendererLanes *self = user_data;
    if (self->clines) lcmtypes_lane_clines_t_destroy (self->clines);
    self->clines = lcmtypes_lane_clines_t_copy (clines);
}

static inline int
_tiles_overlap (tile_texture_t *t1, tile_texture_t *t2) {
    double c1x = (t1->x0+t1->x1)/2;
    double c1y = (t1->y0+t1->y1)/2;
    double c2x = (t2->x0+t2->x1)/2;
    double c2y = (t2->y0+t2->y1)/2;
    double w2 = t2->width;
    double h2 = t2->height;
    
    double thresh = 4.0;

    // overlap if center is inside
    if ( t2->x0 + thresh < c1x && c1x < t2->x1 -thresh &&
         t2->y0 + thresh < c1y && c1y < t2->y1 -thresh)
        return 1;
    
    // overlap if centers are too close
    if ( fabs(c1x-c2x)<w2/2-thresh && fabs(c1y-c2y) < h2/2-thresh )
        return 1;

    return 0;
}

static void
on_detect_map_tile (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_gridmap_tile_t *tile,
        void *user_data)
{
    RendererLanes *self = user_data;
    if (tile->generation < self->last_generation) return;
    if (!gtku_param_widget_get_bool (self->pw, PARAM_SHOW_DETECT_MAP)) return;

//    if (tile->generation > self->last_generation) {
////        gu_set_remove_all (self->tiles);
//        self->last_generation = tile->generation;
//    }

    tile_texture_t *new_tt = tile_texture_new (tile);

    GPtrArray *old_tiles = gu_set_get_elements (self->tiles);
    for (int i=0; i<old_tiles->len; i++) {
        tile_texture_t *old_tt = g_ptr_array_index (old_tiles, i);
        if ((fabs (old_tt->utime - new_tt->utime) * 1e-6 > 1) ||
            (abs (old_tt->generation - new_tt->generation) > 1) ||
            (_tiles_overlap (old_tt, new_tt))) {
            gu_set_remove (self->tiles, old_tt);
            continue;
        }
    }

    gu_set_add (self->tiles, new_tt);
}

static void
on_param_widget_changed (GtkuParamWidget *pw, const char *name, void *user)
{
    RendererLanes *self = (RendererLanes*) user;
   viewer_request_redraw ( self->viewer);
}

static void
on_load_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererLanes *self = user_data;
    gtku_param_widget_load_from_key_file (self->pw, keyfile, RENDERER_NAME);
}

static void
on_save_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererLanes *self = user_data;
    gtku_param_widget_save_to_key_file (self->pw, keyfile, RENDERER_NAME);
}

static void
lanes_free (Renderer *renderer) 
{
    RendererLanes *self = (RendererLanes*) renderer;
    lcmtypes_lane_list_t_destroy (self->lanes);

    lcmtypes_lane_list_t_unsubscribe (self->lc, self->lc_handler_ll);
    lcmtypes_lane_detection_t_unsubscribe (self->lc, self->lc_handler_ld);
    lcmtypes_gridmap_tile_t_unsubscribe (self->lc, self->lc_handler_gm);

    gu_set_destroy (self->tiles);

    gu_queue_free_with_func (self->detections, 
            (GDestroyNotify) lcmtypes_lane_detection_t_destroy);
    globals_release_ctrans (self->ctrans);
    globals_release_lcm (self->lc);
    free (renderer);
}

Renderer *renderer_lanes_new (Viewer *viewer)
{
    RendererLanes *self = (RendererLanes*) calloc (1, sizeof (RendererLanes));
    self->viewer = viewer;
    self->renderer.draw = lanes_draw;
    self->renderer.destroy = lanes_free;
    self->renderer.name = RENDERER_NAME;
    self->renderer.user = self;
    self->renderer.enabled = 1;

    self->renderer.widget = gtk_alignment_new (0, 0.5, 1.0, 0);
    self->ctrans = globals_get_ctrans();

    self->lanes = NULL;
    self->clines = NULL;
    self->lc = globals_get_lcm ();
    self->lc_handler_ll = lcmtypes_lane_list_t_subscribe (self->lc, "LANES", 
            on_lane_list, self);
    self->lc_handler_ld = lcmtypes_lane_detection_t_subscribe (self->lc, 
            "CAMLINES.*", on_lane_detection, self);
    self->lc_handler_gm = lcmtypes_gridmap_tile_t_subscribe (self->lc, 
            "LANE_DETECT_MAP", on_detect_map_tile, self);
    self->lc_handler_cl = lcmtypes_lane_clines_t_subscribe (self->lc,
            "LANE_CLINES", on_lane_clines, self);

    self->detections = g_queue_new ();

    self->tiles = gu_set_new_full (g_direct_hash, g_direct_equal, 
            (GDestroyNotify) tile_texture_destroy);
    self->last_generation = 0;

    self->pw = GTKU_PARAM_WIDGET (gtku_param_widget_new ());
    gtk_container_add (GTK_CONTAINER (self->renderer.widget), 
            GTK_WIDGET (self->pw));
    gtku_param_widget_add_booleans (self->pw, 0, PARAM_SHOW_BOUNDARIES, 1, 
            NULL);
    gtku_param_widget_add_booleans (self->pw, 0, PARAM_SHOW_CLINES, 0, NULL);
    gtku_param_widget_add_booleans (self->pw, 0, PARAM_SHOW_CENTERLINES, 0, 
            NULL);
    gtku_param_widget_add_booleans (self->pw, 0, PARAM_SHOW_DETECTIONS, 0, 
            NULL);
    gtku_param_widget_add_booleans (self->pw, 0, PARAM_SHOW_DETECT_MAP, 0, 
            NULL);
    gtk_widget_show (GTK_WIDGET (self->pw));

    g_signal_connect (G_OBJECT (self->pw), "changed",
            G_CALLBACK (on_param_widget_changed), self);

    g_signal_connect (G_OBJECT (viewer), "load-preferences", 
            G_CALLBACK (on_load_preferences), self);
    g_signal_connect (G_OBJECT (viewer), "save-preferences",
            G_CALLBACK (on_save_preferences), self);

    return &self->renderer;
}

void setup_renderer_lanes(Viewer *viewer, int render_priority)
{
    viewer_add_renderer(viewer, renderer_lanes_new(viewer), render_priority);
}
