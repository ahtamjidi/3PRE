#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <gdk/gdkkeysyms.h>

#include <common/small_linalg.h>
#include <common/rotations.h>
#include <common/camtrans.h>
#include <common/jpeg.h>
#include <common/pixels.h>
#include <lcmtypes/lcmtypes_image_t.h>
#include <dgc/globals.h>
#include <dgc/config_util.h>

#include <libviewer/viewer.h>

#define EYE_MIN_DIST 0.1
#define EYE_MAX_DIST 10000
#define EYE_ZOOM_INC (EYE_MAX_DIST - EYE_MIN_DIST) / 100

#define PROJECTION_PERSPECTIVE  1
#define PROJECTION_ORTHOGRAPHIC 2

typedef struct _CameraViewHandler CameraViewHandler;
struct _CameraViewHandler
{
    ViewHandler  vhandler;

    Viewer *viewer;

    lcm_t *lc;

    double viewport_width;
    double viewport_height;

    Config *config;
    CamTrans *camtrans;
    CTrans *ctrans;

    lcmtypes_image_t *last_thumb;
    uint8_t *uncompressed_buffer;
    int uncompressed_buffer_size;
    GLuint texname;
    int is_uploaded;

    GLuint undistort_vbo;

    // XXX
    double fov_degrees;

    double last_mouse_x;
    double last_mouse_y;

    int projection_type;

    double lookat[3];
    double eye[3];
    double up[3];

    int have_last;
    double lastpos[3], lastquat[4];
};

static void on_image (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_image_t *msg, void *user_data);

static void 
on_image (const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_image_t *msg, void *user_data)
{
    CameraViewHandler *self = (CameraViewHandler*) user_data;
    if (self->last_thumb) lcmtypes_image_t_destroy (self->last_thumb);
    self->last_thumb = lcmtypes_image_t_copy (msg);
    self->is_uploaded = 0;
}

static double
get_cam_vertical_fov (CamTrans *camtrans)
{
    double cop_x = camtrans_get_principal_x (camtrans);
    double cop_y = camtrans_get_principal_y (camtrans);
    double cam_height = camtrans_get_image_height (camtrans);

    double upper[3], lower[3], middle[3];
    camtrans_pixel_to_ray (camtrans, cop_x, 0, upper);
    camtrans_pixel_to_ray (camtrans, cop_x, cop_y, middle);
    camtrans_pixel_to_ray (camtrans, cop_x, cam_height, lower);

    // since the center of projection may not be in the middle of the image,
    // the angle from the COP to the top pixel may not be the same as the angle
    // from the COP to the bottom pixel.  Set the FOV to be twice the
    // smaller of these two angles, so that the entire vertical FOV is within
    // the image extents
    double theta1 = vector_angle_3d (upper, middle);
    double theta2 = vector_angle_3d (lower, middle);
    double fov = MIN (theta1, theta2) * 2;
    return fov;
}

static int
upload_image (CameraViewHandler * self)
{
    if (self->is_uploaded)
        return 0;

    int thumb_width = self->last_thumb->width;
    int thumb_height = self->last_thumb->height;

    GLenum int_format;
    GLenum gl_format;
    uint8_t *tex_src = NULL;

    switch (self->last_thumb->pixelformat) {
        case 0:
        case PIXEL_FORMAT_GRAY:
        case PIXEL_FORMAT_BAYER_BGGR:
        case PIXEL_FORMAT_BAYER_RGGB:
        case PIXEL_FORMAT_BAYER_GRBG:
        case PIXEL_FORMAT_BAYER_GBRG:
            int_format = GL_LUMINANCE8;
            gl_format = GL_LUMINANCE;
            tex_src = self->last_thumb->image;
            break;
        case PIXEL_FORMAT_MJPEG:
            {
                lcmtypes_image_t * msg = self->last_thumb;
                int req_size = msg->width * 3 * msg->height;
                if (req_size > self->uncompressed_buffer_size) {
                    self->uncompressed_buffer = 
                        (uint8_t*) realloc (self->uncompressed_buffer, 
                                req_size);
                    self->uncompressed_buffer_size = req_size;
                }
                jpeg_decompress_to_8u_rgb (msg->image, msg->size, 
                        self->uncompressed_buffer, 
                        msg->width, msg->height, msg->width*3);
            }
            int_format = GL_RGBA8;
            gl_format = GL_RGB;
            tex_src = self->uncompressed_buffer;
            break;
        default:
            return -1;
    }

    GLenum textarget = GL_TEXTURE_RECTANGLE_ARB;

    glPixelStorei (GL_UNPACK_ROW_LENGTH, self->last_thumb->width);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D (textarget, 0, int_format, thumb_width, thumb_height,
            0, gl_format, GL_UNSIGNED_BYTE, tex_src);
    glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);

    self->is_uploaded = 1;
    return 0;
}

static void 
update_gl_matrices   (Viewer *viewer, ViewHandler *vhandler)
{
    CameraViewHandler *self = (CameraViewHandler*) vhandler->user;

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    self->viewport_width = viewport[2];
    self->viewport_height = viewport[3];

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

    double cam_width = camtrans_get_image_width (self->camtrans);
    double cam_height = camtrans_get_image_height (self->camtrans);

    double fov_y = get_cam_vertical_fov (self->camtrans);

    gluPerspective (fov_y * 180 / M_PI, 
            self->viewport_width / self->viewport_height, 0.1,
            EYE_MAX_DIST * 2);

    CTrans *ctrans = self->ctrans;

    double fwd[3];

    double pos_calib[3], orientation_calib[4];
    double fwd_calib[3] = { 0, 0, 1 };
    double up_calib[3] = { 0, -1, 0 };
    camtrans_get_position (self->camtrans, pos_calib);
    camtrans_get_orientation (self->camtrans, orientation_calib);
    rot_quat_rotate (orientation_calib, fwd_calib);
    rot_quat_rotate (orientation_calib, up_calib);

    double calib_to_local[16], calib_to_local_rot[16];
    if (self->last_thumb) {
        lcmtypes_pose_t pose;
        ctrans_local_pose_at (ctrans, &pose, self->last_thumb->utime);
        ctrans_calibration_to_local_matrix_with_pose (ctrans, calib_to_local, 
                &pose);
    } else {
        ctrans_calibration_to_local_matrix (ctrans, calib_to_local);
    }
    matrix_rigid_body_transform_get_rotation_matrix_4x4d (calib_to_local,
            calib_to_local_rot);
    
    matrix_vector_multiply_4x4_3d (calib_to_local,     pos_calib,  self->eye);
    matrix_vector_multiply_4x4_3d (calib_to_local_rot, fwd_calib,  fwd);
    matrix_vector_multiply_4x4_3d (calib_to_local_rot, up_calib,   self->up);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

    self->lookat[0] = self->eye[0] + fwd[0];
    self->lookat[1] = self->eye[1] + fwd[1];
    self->lookat[2] = self->eye[2] + fwd[2];
    gluLookAt (self->eye[0], self->eye[1], self->eye[2],
            self->eye[0] + fwd[0], self->eye[1] + fwd[1], self->eye[2] + fwd[2],
            self->up[0], self->up[1], self->up[2]);

    if (self->last_thumb) {
        int thumb_width = self->last_thumb->width;
        int thumb_height = self->last_thumb->height;
        GLenum textarget = GL_TEXTURE_RECTANGLE_ARB;
        glPushAttrib (GL_ENABLE_BIT);
        glEnable (textarget);
        glBindTexture (textarget, self->texname);

        if (upload_image (self) < 0) {
            glBindTexture (textarget, 0);
            glPopAttrib ();
            return;
        }

        glBegin (GL_QUADS);
        glColor3f (1.0, 1.0, 1.0);

        double scale_x = thumb_width / cam_width;
        double scale_y = thumb_height / cam_height;

        int xstep = cam_width/24;
        int ystep = cam_height/24;
        for (int y=0; y<cam_height; y+=ystep) {
            for (int x=0; x<cam_width; x+=xstep) {
                int xn = x+xstep;
                int yn = y+ystep;
                double r1_calib[3], r2_calib[3], r3_calib[3], r4_calib[4];

                camtrans_pixel_to_ray (self->camtrans, x, y, r1_calib);
                camtrans_pixel_to_ray (self->camtrans, xn, y, r2_calib);
                camtrans_pixel_to_ray (self->camtrans, xn, yn, r3_calib);
                camtrans_pixel_to_ray (self->camtrans, x, yn, r4_calib);

                vector_normalize_3d (r1_calib);
                vector_normalize_3d (r2_calib);
                vector_normalize_3d (r3_calib);
                vector_normalize_3d (r4_calib);

                double r1[3], r2[3], r3[3], r4[3];
                matrix_vector_multiply_4x4_3d (calib_to_local_rot, r1_calib, 
                        r1);
                matrix_vector_multiply_4x4_3d (calib_to_local_rot, r2_calib, 
                        r2);
                matrix_vector_multiply_4x4_3d (calib_to_local_rot, r3_calib, 
                        r3);
                matrix_vector_multiply_4x4_3d (calib_to_local_rot, r4_calib, 
                        r4);

                double p1[3], p2[3], p3[3], p4[3];
                vector_add_3d (self->eye, r1, p1);
                vector_add_3d (self->eye, r2, p2);
                vector_add_3d (self->eye, r3, p3);
                vector_add_3d (self->eye, r4, p4);

                glTexCoord2f (x * scale_x, y * scale_y);
                glVertex3d (p1[0], p1[1], p1[2]);
                glTexCoord2f (xn * scale_x, y * scale_y);
                glVertex3d (p2[0], p2[1], p2[2]);
                glTexCoord2f (xn * scale_x, yn * scale_y);
                glVertex3d (p3[0], p3[1], p3[2]);
                glTexCoord2f (x * scale_x, yn * scale_y);
                glVertex3d (p4[0], p4[1], p4[2]);
            }
        }
        glEnd ();

        glBindTexture (textarget, 0);
        glPopAttrib ();
    }

}

static void 
get_eye_look (ViewHandler *vhandler, double eye[3], double look[3], 
        double up[3])
{
    CameraViewHandler *self = (CameraViewHandler*) vhandler->user;

    memcpy(eye, self->eye, 3 * sizeof(double));
    memcpy(look, self->lookat, 3 * sizeof(double));
    memcpy(up, self->up, 3 * sizeof(double));
}
   
static void 
destroy (ViewHandler *super)
{
    CameraViewHandler *self = (CameraViewHandler*) super;

    if (self->config) {
        globals_release_config (self->config);
        self->config = NULL;
    }
    if (self->lc) {
        globals_release_lcm (self->lc);
        self->lc = NULL;
    }
    if (self->camtrans) {
        camtrans_destroy (self->camtrans);
    }
    if (self->texname > 0) {
        glDeleteTextures (1, &self->texname);
    }
    if (self->uncompressed_buffer) {
        free (self->uncompressed_buffer);
        self->uncompressed_buffer = NULL;
    }
    if (self->last_thumb) {
        lcmtypes_image_t_destroy (self->last_thumb);
    }

    free (self);
}

CameraViewHandler *camera_view_handler_new(Viewer *viewer, 
        const char *cam_name)
{
    CameraViewHandler *self  = 
        (CameraViewHandler*) calloc(1, sizeof(CameraViewHandler));

    self->fov_degrees = 60;
    self->projection_type = PROJECTION_PERSPECTIVE;

    self->lookat[0] = 1; self->lookat[1] = 0; self->lookat[2] = 0;
    self->eye[0] = 0;    self->eye[1] = 0;    self->eye[2] = 0;
    self->up[0] = 1;     self->up[1] = 0;     self->up[2] = 0;

    self->vhandler.update_gl_matrices = update_gl_matrices;
    self->vhandler.get_eye_look = get_eye_look;
    self->vhandler.set_look_at = NULL;
    self->vhandler.update_follow_target = NULL;
    self->vhandler.set_camera_perspective = NULL;
    self->vhandler.set_camera_orthographic = NULL;
    self->vhandler.user = self;

    self->viewer = viewer;
    
    self->lc = globals_get_lcm ();
    self->config = globals_get_config ();
    self->ctrans = globals_get_ctrans ();

    self->camtrans = config_util_get_new_camtrans (self->config, cam_name);
    if (!self->camtrans) {
        destroy ((ViewHandler*) self);
        return NULL;
    }

    char thumbnail_channel[256];
    if (0 != config_util_get_camera_thumbnail_channel (self->config, cam_name,
                thumbnail_channel, sizeof (thumbnail_channel))) {
        destroy ((ViewHandler*) self);
        return NULL;
    }

    self->last_thumb = NULL;
    self->uncompressed_buffer = NULL;
    self->uncompressed_buffer_size = 0;

    lcmtypes_image_t_subscribe (self->lc, thumbnail_channel, on_image, self);

    glGenTextures (1, &self->texname);

    return self;
}
