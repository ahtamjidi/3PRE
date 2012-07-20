#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <common/jpeg.h>
#include <common/small_linalg.h>
#include <common/math_util.h>
#include <common/geometry.h>
#include <common/pixels.h>
#include <common/glib_util.h>
#include <glutil/glutil.h>
#include <lcmtypes/lcmtypes_image_t.h>
#include <dgc/config_util.h>
#include <dgc/globals.h>
#include <glutil/glutil.h>
#include <gtk_util/gtk_util.h>

#include "libviewer/viewer.h"

#define RENDERER_NAME "Camera"

#define PARAM_RENDER_IN "Show"

typedef struct _RendererCamThumb RendererCamThumb;

struct _RendererCamThumb {
    Renderer renderer;

    CTrans *ctrans;

    GtkuParamWidget    *pw;

    GHashTable *cam_handlers;
    Config *config;

    lcm_t *lc;
    Viewer *viewer;
};

typedef struct _cam_renderer {
    char *channel;
    GtkuGLDrawingArea *gl_area;
    lcmtypes_image_t *last_image;
    GLUtilTexture *texture;
    RendererCamThumb *renderer;
    GtkuParamWidget *pw;
    GtkWidget *expander;
    //GtkAspectFrame *aspect_frame;
    uint8_t *uncompresed_buffer;
    int uncompressed_buffer_size;
    int width, height;
    int is_uploaded;

    int msg_received;
    int render_place;
    int expanded;
} cam_renderer_t;

enum {
    RENDER_IN_WIDGET,
    RENDER_IN_TOP_RIGHT,
    RENDER_IN_TOP_CENTER,
    RENDER_IN_TOP_LEFT,
    RENDER_IN_BOTTOM_RIGHT,
    RENDER_IN_BOTTOM_CENTER,
    RENDER_IN_BOTTOM_LEFT
};

static void
cam_renderer_destroy (cam_renderer_t *cr)
{
    if (cr->last_image) {
        lcmtypes_image_t_destroy (cr->last_image);
    }
// TODO
//    if (cr->texture) {
//        glutil_texture_free (cr->texture);
//    }
    if (cr->uncompresed_buffer) {
        free (cr->uncompresed_buffer);
        cr->uncompresed_buffer = NULL;
    }
    free (cr->channel);
    // specifically do not delete the gl_area
    free (cr);
}

static void on_image (const lcm_recv_buf_t *rbuf, const char *channel, 
        const lcmtypes_image_t *msg, void *user_data);
static void cam_renderer_draw (cam_renderer_t *cr);

static void
cam_thumb_draw (Viewer *viewer, Renderer *renderer)
{
    RendererCamThumb *self = (RendererCamThumb*) renderer->user;
    GLint viewport[4];

    glGetIntegerv (GL_VIEWPORT, viewport);

    // transform into window coordinates, where <0, 0> is the top left corner
    // of the window and <viewport[2], viewport[3]> is the bottom right corner
    // of the window
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, viewport[2], 0, viewport[3]);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0, viewport[3], 0);
    glScalef(1, -1, 1);

    double vp_width = viewport[2] - viewport[0];
    double vp_height = viewport[3] - viewport[1];

    GPtrArray *crlist = gu_hash_table_get_vals (self->cam_handlers);
    for (int criter = 0; criter < g_ptr_array_size(crlist); criter++) {
        cam_renderer_t *cr = g_ptr_array_index(crlist, criter);

        if (!cr->last_image) continue;
        double aspect = cr->last_image->width / 
            (double) cr->last_image->height;

        double thumb_width, thumb_height;
        if ((vp_width / 3) / aspect > vp_height / 3) {
            thumb_height = vp_height / 3;
            thumb_width = thumb_height * aspect;
        } else {
            thumb_width = vp_width / 3;
            thumb_height = thumb_width / aspect;
        }

        int rmode = gtku_param_widget_get_enum (cr->pw, PARAM_RENDER_IN);
        if (rmode == RENDER_IN_WIDGET) continue;

        point2d_t p1 = { viewport[0], viewport[1] }; 

        switch (rmode) {
            case RENDER_IN_BOTTOM_RIGHT:
                p1.x = vp_width - thumb_width;
                p1.y = vp_height - thumb_height;
                break;
            case RENDER_IN_BOTTOM_CENTER:
                p1.x = vp_width / 3;
                p1.y = vp_height - thumb_height;
                break;
            case RENDER_IN_BOTTOM_LEFT:
                p1.x = 0;
                p1.y = vp_height - thumb_height;
                break;
            case RENDER_IN_TOP_LEFT:
                p1.x = 0;
                p1.y = 0;
                break;
            case RENDER_IN_TOP_CENTER:
                p1.x = vp_width / 3;
                p1.y = 0;
                break;
            case RENDER_IN_TOP_RIGHT:
                p1.x = vp_width - thumb_width;
                p1.y = 0;
                break;
            default:
                break;
        }

        glPushMatrix ();
        glTranslatef (p1.x, p1.y, 1);
        glScalef (thumb_width, thumb_height, 1);
        cam_renderer_draw (cr);
        glPopMatrix ();
    }
    g_ptr_array_free (crlist, TRUE);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static void
on_load_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererCamThumb *self = user_data;

    GError *gerr = NULL;
    char **keys = g_key_file_get_keys (keyfile, RENDERER_NAME, NULL, &gerr);
    if (gerr) {
        g_error_free (gerr);
        return;
    }
    for (int i=0; keys[i]; i++) {
        char *key = keys[i];
        cam_renderer_t *cr = g_hash_table_lookup (self->cam_handlers, key);
        if (!cr) {
            cr = (cam_renderer_t*) calloc (1, sizeof (cam_renderer_t));
            cr->channel = strdup (key);
            cr->renderer = self;
            g_hash_table_replace (self->cam_handlers, cr->channel, cr);
        }
        char *val = g_key_file_get_string (keyfile, RENDERER_NAME, key, NULL);
        cr->render_place = 0;
        cr->expanded = 0;
        if (val) {
            sscanf (val, "%d %d", &cr->render_place, &cr->expanded);
        }
    }
    g_strfreev (keys);
}

static void
on_save_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererCamThumb *self = user_data;
    GPtrArray *keys = gu_hash_table_get_keys(self->cam_handlers);

    for (int k = 0; k < g_ptr_array_size(keys); k++) {
        char *key = g_ptr_array_index(keys, k);
        cam_renderer_t *cr = g_hash_table_lookup(self->cam_handlers, key);

        char str[80];
        sprintf (str, "%d %d", cr->render_place, cr->expanded);
        g_key_file_set_string (keyfile, RENDERER_NAME, key, str);
    }
}

static void
cam_thumb_free (Renderer *renderer) 
{
    RendererCamThumb *self = (RendererCamThumb*) renderer;

    g_hash_table_destroy (self->cam_handlers);
    globals_release_config (self->config);
    globals_release_ctrans (self->ctrans);
    globals_release_lcm (self->lc);
    free (renderer);
}

static Renderer *_new (Viewer *viewer)
{
    RendererCamThumb *self = 
        (RendererCamThumb*) calloc (1, sizeof (RendererCamThumb));
    self->viewer = viewer;
    self->renderer.draw = cam_thumb_draw;
    self->renderer.destroy = cam_thumb_free;
    self->renderer.name = "Camera";
    self->renderer.user = self;
    self->renderer.enabled = 1;

    self->ctrans = globals_get_ctrans();
    self->lc = globals_get_lcm (NULL);

    self->renderer.widget = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (self->renderer.widget);

    self->cam_handlers = g_hash_table_new_full (g_str_hash, g_str_equal,
            NULL, (GDestroyNotify) cam_renderer_destroy);
    self->config = globals_get_config ();

    char **cam_names = config_get_subkeys (self->config, "cameras");
    for (int i=0; cam_names[i]; i++) {
        char channel[80];
        if (0 == config_util_get_camera_thumbnail_channel (self->config,
                    cam_names[i], channel, sizeof (channel))) {
            lcmtypes_image_t_subscribe (self->lc, channel, on_image, self);
        }
    }
    g_strfreev (cam_names);

    g_signal_connect (G_OBJECT (viewer), "load-preferences", 
            G_CALLBACK (on_load_preferences), self);
    g_signal_connect (G_OBJECT (viewer), "save-preferences",
            G_CALLBACK (on_save_preferences), self);

    return &self->renderer;
}

void setup_renderer_cam_thumb (Viewer *viewer, int render_priority)
{
    viewer_add_renderer(viewer, _new(viewer), render_priority);
}

static void
cam_renderer_draw (cam_renderer_t *cr)
{
    if (! cr->renderer->renderer.enabled) return;
    if (! cr->last_image) return;

    // create the texture object if necessary
    if (! cr->texture) {
        cr->texture = glutil_texture_new (cr->last_image->width, 
                cr->last_image->height, 
                cr->last_image->width * 3 * cr->last_image->height);
    }

    // upload the texture to the graphics card if necessary
    if (!cr->is_uploaded) {
        int stride = 0;
        GLenum gl_format;
        uint8_t *tex_src = NULL;

        if (cr->last_image->pixelformat == 0 || 
                cr->last_image->pixelformat == PIXEL_FORMAT_GRAY ||
                cr->last_image->pixelformat == PIXEL_FORMAT_BAYER_BGGR ||
                cr->last_image->pixelformat == PIXEL_FORMAT_BAYER_RGGB ||
                cr->last_image->pixelformat == PIXEL_FORMAT_BAYER_GRBG ||
                cr->last_image->pixelformat == PIXEL_FORMAT_BAYER_GBRG) {

            stride = cr->last_image->width;
            gl_format = GL_LUMINANCE;
            tex_src = cr->last_image->image;
        } else if (cr->last_image->pixelformat == PIXEL_FORMAT_MJPEG) {
            lcmtypes_image_t * msg = cr->last_image;

            // might need to JPEG decompress...
            stride = cr->last_image->width * 3;
            int buf_size = msg->height * stride;
            if (cr->uncompressed_buffer_size < buf_size) {
                cr->uncompresed_buffer = 
                    realloc (cr->uncompresed_buffer, buf_size);
                cr->uncompressed_buffer_size = buf_size;
            }
            jpeg_decompress_to_8u_rgb (msg->image, msg->size, 
                    cr->uncompresed_buffer, msg->width, msg->height, stride);

            gl_format = GL_RGB;
            tex_src = cr->uncompresed_buffer;
        } else {
            return;
        }
        glutil_texture_upload (cr->texture, gl_format, GL_UNSIGNED_BYTE,
                stride, tex_src);
        cr->is_uploaded = 1;
    }

    // draw the image
    glColor3f (1, 1, 1);
    glutil_texture_draw (cr->texture);

}

static gboolean
on_gl_area_expose (GtkWidget * widget, GdkEventExpose * event, void* user_data)
{
    cam_renderer_t *cr = (cam_renderer_t*) user_data;

    gtku_gl_drawing_area_set_context (cr->gl_area);

    glClearColor (0.0, 0.0, 0.0, 1.0);
    glClear (GL_COLOR_BUFFER_BIT);
    
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

    glOrtho (0, 1, 1, 0, -1, 1);
    glMatrixMode (GL_MODELVIEW);

    cam_renderer_draw (cr);

    gtku_gl_drawing_area_swap_buffers (cr->gl_area);
    return TRUE;
}

static void
on_gl_area_size (GtkWidget * widget, GtkAllocation * alloc, cam_renderer_t * cr)
{
    gtk_widget_set_size_request (widget, -1,
            alloc->width * cr->height / cr->width);
}

static void
on_expander_expanded (GtkExpander *expander, void *user_data)
{
    cam_renderer_t *cr = user_data;
    cr->expanded = gtk_expander_get_expanded (expander);
}

static void
on_cam_renderer_param_widget_changed (GtkuParamWidget *pw, const char *param,
        void *user_data)
{
    cam_renderer_t *cr = (cam_renderer_t*) user_data;

    // delete the old texture object if it exists.  make sure that we've
    // selected the correct OpenGL context
    if (cr->texture) {
        if (cr->render_place == RENDER_IN_WIDGET) {
            gtku_gl_drawing_area_set_context (cr->gl_area);
        } else {
            gtku_gl_drawing_area_set_context (cr->renderer->viewer->gl_area);
        }

        glutil_texture_free (cr->texture);
        cr->texture = NULL;
    }

    cr->render_place = gtku_param_widget_get_enum (pw, PARAM_RENDER_IN);
    if (cr->render_place == RENDER_IN_WIDGET) {
        gtk_widget_show (GTK_WIDGET (cr->gl_area));
    } else {
        gtk_widget_hide (GTK_WIDGET (cr->gl_area));
    }

    cr->is_uploaded = 0;
    viewer_request_redraw (cr->renderer->viewer);
}

static void 
on_image (const lcm_recv_buf_t *rbuf, const char *channel, 
        const lcmtypes_image_t *msg, void *user_data)
{
    RendererCamThumb *self = (RendererCamThumb*) user_data;

    if (! self->renderer.enabled) return;

    cam_renderer_t *cr = g_hash_table_lookup (self->cam_handlers, channel);
    if (!cr) {
        cr = (cam_renderer_t*) calloc (1, sizeof (cam_renderer_t));
        cr->renderer = self;
        cr->render_place = 0;
        cr->channel = strdup (channel);
        g_hash_table_replace (self->cam_handlers, cr->channel, cr);
    }

    if (! cr->msg_received) {
        cr->gl_area = GTKU_GL_DRAWING_AREA (gtku_gl_drawing_area_new (FALSE));

        cr->pw = GTKU_PARAM_WIDGET (gtku_param_widget_new ());
        gtku_param_widget_add_enum (cr->pw, PARAM_RENDER_IN, 
                0,
                cr->render_place,
                "Here", RENDER_IN_WIDGET,
                "Top Left", RENDER_IN_TOP_LEFT,
                "Top Cent.", RENDER_IN_TOP_CENTER,
                "Top Right", RENDER_IN_TOP_RIGHT,
                "Bot. Left", RENDER_IN_BOTTOM_LEFT,
                "Bot. Cent.", RENDER_IN_BOTTOM_CENTER,
                "Bot. Right", RENDER_IN_BOTTOM_RIGHT,
                NULL);

        cr->expander = gtk_expander_new (channel);
        gtk_box_pack_start (GTK_BOX (self->renderer.widget),
                cr->expander, TRUE, TRUE, 0);
        GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
        gtk_container_add (GTK_CONTAINER (cr->expander), vbox);

        gtk_box_pack_start (GTK_BOX (vbox), 
               GTK_WIDGET (cr->pw), TRUE, TRUE, 0);
        gtk_box_pack_start (GTK_BOX (vbox), 
               GTK_WIDGET (cr->gl_area), TRUE, TRUE, 0);

        g_signal_connect (G_OBJECT (cr->gl_area), "size-allocate",
                G_CALLBACK (on_gl_area_size), cr);
        cr->width = msg->width;
        cr->height = msg->height;

        gtk_widget_show_all (GTK_WIDGET (cr->expander));
        gtk_expander_set_expanded (GTK_EXPANDER (cr->expander), cr->expanded);

        if (cr->render_place == RENDER_IN_WIDGET) {
            gtk_widget_show (GTK_WIDGET (cr->gl_area));
        } else {
            gtk_widget_hide (GTK_WIDGET (cr->gl_area));
        }

        g_signal_connect (G_OBJECT (cr->pw), "changed",
                G_CALLBACK (on_cam_renderer_param_widget_changed), cr);
        g_signal_connect (G_OBJECT (cr->gl_area), "expose-event", 
                G_CALLBACK (on_gl_area_expose), cr);

        g_signal_connect (G_OBJECT (cr->expander), "notify::expanded",
                G_CALLBACK (on_expander_expanded), cr);

        cr->texture = NULL;
        cr->last_image = NULL;
        cr->renderer = self;
        cr->uncompressed_buffer_size = msg->width * msg->height * 3;
        cr->uncompresed_buffer = 
            (uint8_t*) malloc (cr->uncompressed_buffer_size);

        cr->msg_received = 1;
    }

    if (cr->last_image) {
        lcmtypes_image_t_destroy (cr->last_image);
    }
    cr->last_image = lcmtypes_image_t_copy (msg);
    cr->is_uploaded = 0;

    switch (gtku_param_widget_get_enum (cr->pw, PARAM_RENDER_IN)) {
        case RENDER_IN_WIDGET:
            if (gtk_expander_get_expanded (GTK_EXPANDER (cr->expander)))
                gtku_gl_drawing_area_invalidate (cr->gl_area);
        default:
            viewer_request_redraw (self->viewer);
            break;
    }
}
