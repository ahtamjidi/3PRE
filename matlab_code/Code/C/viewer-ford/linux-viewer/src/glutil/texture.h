#ifndef GLUTIL_TEXTURE_H
#define GLUTIL_TEXTURE_H

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GLUtilTexture GLUtilTexture;

GLUtilTexture * glutil_texture_new (int width, int height, int max_data_size);

void glutil_texture_free (GLUtilTexture * t);

int glutil_texture_upload (GLUtilTexture * t, GLenum format,
        GLenum type, int stride, void * data);

/**
 * glutil_texture_draw:
 *
 * renders the texture in a unit square from (0, 0) to (1, 1).  Texture
 * coordinate mapping:
 *
 * texture          opengl
 * 0, 0          -> 0, 0
 * width, 0      -> 1, 0
 * 0, height     -> 0, 1
 * width, height -> 1, 1
 *
 * all opengl Z coordinates are 0.
 */
void
glutil_texture_draw (GLUtilTexture * t);

void
glutil_texture_draw_coords (GLUtilTexture * t, 
        double x0, double y0, 
        double x1, double y1, 
        double x2, double y2, 
        double x3, double y3);

/**
 * gl_texture_set_interp:
 * @nearest_or_linear: typically GL_LINEAR or GL_NEAREST.  default is GL_LINEAR
 *
 * sets the interpolation mode when the texture is not drawn at a 1:1 scale.
 */
void glutil_texture_set_interp (GLUtilTexture * t, GLint nearest_or_linear);

void glutil_texture_set_internal_format (GLUtilTexture *t, GLenum fmt);

int glutil_texture_get_width (GLUtilTexture *t);

int glutil_texture_get_height (GLUtilTexture *t);

#ifdef __cplusplus
}
#endif

#endif
