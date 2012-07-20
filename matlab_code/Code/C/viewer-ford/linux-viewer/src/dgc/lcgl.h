#ifndef _LCGL_H
#define _LCGL_H

#include <lcm/lcm.h>
#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LCGL_GL_BEGIN    4
#define LCGL_GL_END      5
#define LCGL_GL_VERTEX3F 6
#define LCGL_GL_VERTEX3D 7
#define LCGL_GL_COLOR3F  8
#define LCGL_GL_COLOR4F  9
#define LCGL_GL_POINTSIZE 10
#define LCGL_GL_ENABLE   11
#define LCGL_GL_DISABLE  12
#define LCGL_BOX         13
#define LCGL_CIRCLE      14
#define LCGL_GL_LINE_WIDTH  15
#define LCGL_NOP         16
#define LCGL_GL_VERTEX2D 17
#define LCGL_GL_VERTEX2F 18
#define LCGL_TEXT        19
#define LCGL_DISK        20
#define LCGL_GL_TRANSLATED 21
#define LCGL_GL_ROTATED 22
#define LCGL_GL_LOAD_IDENTITY 23
#define LCGL_GL_PUSH_MATRIX 24
#define LCGL_GL_POP_MATRIX  25
#define LCGL_RECT           26
#define LCGL_TEXT_LONG      27

union lcgl_bytefloat
{
    uint8_t  bytes[4];
    float    f;
};

union lcgl_bytedouble
{
    uint8_t  bytes[8];
    double   d;
};

typedef struct lcgl lcgl_t;

lcgl_t *lcgl_init(lcm_t *lcm, const char *name);

void lcgl_destroy (lcgl_t *lcgl);

void lcgl_begin(lcgl_t *lcgl, GLenum mode);
void lcgl_end(lcgl_t *lcgl);

void lcgl_vertex2d(lcgl_t *lcgl, double v0, double v1);
void lcgl_vertex2f(lcgl_t *lcgl, float v0, float v1);
void lcgl_vertex3d(lcgl_t *lcgl, double v0, double v1, double v2);
void lcgl_color3f(lcgl_t *lcgl, float v0, float v1, float v);
void lcgl_color4f(lcgl_t *lcgl, float v0, float v1, float v2, float v3);
void lcgl_point_size(lcgl_t *lcgl, float v);
void lcgl_line_width(lcgl_t *lcgl, float line_width);

void lcgl_switch_buffer(lcgl_t *lcgl);
void lcgl_enable(lcgl_t *lcgl, GLenum v);
void lcgl_disable(lcgl_t *lcgl, GLenum v);

void lcgl_translated(lcgl_t *lcgl, double v0, double v1, double v2);
void lcgl_rotated(lcgl_t *lcgl, double angle, double x, double y, double z);
void lcgl_push_matrix (lcgl_t * lcgl);
void lcgl_pop_matrix (lcgl_t * lcgl);
void lcgl_load_identity(lcgl_t *lcgl);

void lcgl_box_3dv_3fv(lcgl_t *lcgl, double xyz[3], float dim[3]);
void lcgl_circle(lcgl_t *lcgl, double xyz[3], double radius);
void lcgl_disk(lcgl_t *lcgl, double xyz[3], double r_in, double r_out);
void lcgl_text(lcgl_t *lcgl, const double xyz[3], const char *text);
void lcgl_text_ex(lcgl_t *lcgl, const double xyz[3], const char *text, uint32_t font, uint32_t flags);

void lcgl_rect(lcgl_t *lcgl, double xyz[3], double size[2], double theta_rad, int filled);

// these macros provide better "work-alike" interface to GL.  They
// expect that an lcgl* is defined in the current scope.

#define lcglBegin(v) lcgl_begin(lcgl, v)
#define lcglEnd() lcgl_end(lcgl)

#define lcglVertex2d(v0, v1) lcgl_vertex2d(lcgl, v0, v1)
#define lcglVertex2dv(v) lcgl_vertex2d(lcgl, v[0], v[1])

#define lcglVertex2f(v0, v1) lcgl_vertex2f(lcgl, v0, v1)
#define lcglVertex2fv(v) lcgl_vertex2f(lcgl, v[0], v[1])

#define lcglVertex3d(v0, v1, v2) lcgl_vertex3d(lcgl, v0, v1, v2)
#define lcglVertex3dv(v) lcgl_vertex3d(lcgl, v[0], v[1], v[2])

#define lcglColor3f(v0, v1, v2) lcgl_color3f(lcgl, v0, v1, v2)
#define lcglColor3fv(v) lcgl_color3f(lcgl, v[0], v[1], v[2])

#define lcglColor4f(v0, v1, v2, v3) lcgl_color4f(lcgl, v0, v1, v2, v3)
#define lcglColor4fv(v) lcgl_color4f(lcgl, v[0], v[1], v[2], v[3])

#define lcglPointSize(v) lcgl_point_size(lcgl, v)
#define lcglEnable(v) lcgl_enable(lcgl, v)
#define lcglDisable(v) lcgl_disable(lcgl, v)

#define lcglBox(xyz, dim) lcgl_box_3dv_3fv(lcgl, xyz, dim)
#define lcglCircle(xyz, radius) lcgl_circle(lcgl, xyz, radius)
#define lcglDisk(xyz, r_in, r_out) lcgl_disk(lcgl, xyz, r_in, r_out)
#define lcglLineWidth(size) lcgl_line_width(lcgl, size);

#define lcglTranslated(v0, v1, v2) lcgl_translated(lcgl, v0, v1, v2)
#define lcglTranslatef lcglTranslated

#define lcglRotated(angle, x, y, z) lcgl_rotated(lcgl, angle, x, y, z)
#define lcglRotatef lcglRotated

#define lcglLoadIdentity() lcgl_load_identity(lcgl)

#define lcglPushMatrix() lcgl_push_matrix(lcgl)
#define lcglPopMatrix() lcgl_pop_matrix(lcgl)

////////////////////////////////////////////////////////////////////////

typedef struct lcgl_vertex_buffer lcgl_vertex_buffer_t;
typedef void (*lcgl_vertex_buffer_full_callback_t)(lcgl_vertex_buffer_t *vertbuf, void *user);

struct lcgl_vertex
{
    double xyz[3];
};

struct lcgl_vertex_buffer
{
    int size;
    int capacity;
    struct lcgl_vertex *vertices;
    
    lcgl_vertex_buffer_full_callback_t full_callback;
    void *user;
  };

/* vertex buffers allow you to queue up many plot operations for
   single lines and points without having to put a glBegin/End around
   each one. When the buffer is full, the callback is called; you can
   then emit a glBegin/glEnd block.

   IMPORTANT: The callback function must send the data AND must adjust
   the size field (specifically, it must free at least one element.)
   This is not done automatically, because operations like GL_LINE or
   GL_QUADS will want to copy the last N vertices back into the
   beginning of the buffer.

   You'll generally want to call _flush before lcgl_switch_buffer, in
   order to flush out any unsent data.
*/
lcgl_vertex_buffer_t *lcgl_vertex_buffer_create(int capacity, 
                                                lcgl_vertex_buffer_full_callback_t full_callback, 
                                                void *user);

void lcgl_vertex_buffer_destroy(lcgl_vertex_buffer_t *vertbuf);

/** add a vertex to the buffer, and call the callback if this fills the buffer. **/
void lcgl_vertex_buffer_add(lcgl_vertex_buffer_t *vertbuf, double v[3]);

/** force a call to the callback if size > 0 **/
void lcgl_vertex_buffer_flush(lcgl_vertex_buffer_t *vertbuf);

/** Sends all of the vertices; does not clear them. **/
void lcgl_vertex_buffer_send(lcgl_vertex_buffer_t *vertbuf, lcgl_t *lcgl);

#ifdef __cplusplus
}
#endif

#endif
