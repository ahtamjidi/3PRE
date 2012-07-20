#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

#include "lcgl.h"
#include "lcmtypes/lcmtypes_lcgl_data_t.h"
#include "common/timestamp.h"
#include <glutil/glutil.h>

#define INITIAL_ALLOC (1024*1024)

struct lcgl
{
    lcm_t *lcm;
    char *name;
    char *channel_name;

    int32_t scene;
    int32_t sequence;

    uint8_t *data;
    int     datalen;
    int     data_alloc;
};

static inline void ensure_space(lcgl_t *lcgl, int needed)
{
    if (lcgl->datalen + needed < lcgl->data_alloc)
        return;

    // grow our buffer.
    int new_alloc = lcgl->data_alloc * 2;
    lcgl->data = realloc(lcgl->data, new_alloc);
    lcgl->data_alloc = new_alloc;
}

static inline void lcgl_encode_u8(lcgl_t *lcgl, uint8_t v)
{
    ensure_space(lcgl, 1);

    lcgl->data[lcgl->datalen++] = v & 0xff;
}

static inline void lcgl_encode_u32(lcgl_t *lcgl, uint32_t v)
{
    ensure_space(lcgl, 4);

    lcgl->data[lcgl->datalen++] = (v>>24) & 0xff;
    lcgl->data[lcgl->datalen++] = (v>>16) & 0xff;
    lcgl->data[lcgl->datalen++] = (v>>8)  & 0xff;
    lcgl->data[lcgl->datalen++] = (v>>0)  & 0xff;
}

static inline void lcgl_encode_float(lcgl_t *lcgl, float f)
{
    union lcgl_bytefloat u;

    ensure_space(lcgl, 4);

    u.f = f;

    for (int i = 0; i < 4; i++)
        lcgl->data[lcgl->datalen++] = u.bytes[i] & 0xff;
}

static inline void lcgl_encode_double(lcgl_t *lcgl, double d)
{
    union lcgl_bytedouble u;

    ensure_space(lcgl, 8);

    u.d = d;

    for (int i = 0; i < 8; i++)
        lcgl->data[lcgl->datalen++] = u.bytes[i] & 0xff;
}

void lcgl_nop(lcgl_t *lcgl)
{
    lcgl_encode_u8(lcgl, LCGL_NOP);
}

void lcgl_switch_buffer(lcgl_t *lcgl)
{
    lcmtypes_lcgl_data_t ld;
    memset(&ld, 0, sizeof(ld));

    ld.name     = lcgl->name;
    ld.scene    = lcgl->scene;
    ld.sequence = lcgl->sequence;
    ld.datalen  = lcgl->datalen;
    ld.data     = lcgl->data;

    lcmtypes_lcgl_data_t_publish(lcgl->lcm, lcgl->channel_name, &ld);

    lcgl->sequence = 0;
    lcgl->datalen = 0;

    lcgl->scene++;
}

lcgl_t *lcgl_init(lcm_t *lcm, const char *name)
{
    lcgl_t *lcgl = (lcgl_t*) calloc(1, sizeof(lcgl_t));

    lcgl->lcm = lcm;
    lcgl->name = strdup(name);
    lcgl->scene = timestamp_now();
    lcgl->channel_name = malloc(128);

    lcgl->data = malloc(INITIAL_ALLOC);
    lcgl->data_alloc = INITIAL_ALLOC;

    // XXX sanitize LCGL channel name?
    snprintf(lcgl->channel_name, 128, "LCGL_%s", lcgl->name);

    return lcgl;
}

void lcgl_destroy (lcgl_t *lcgl)
{
    free (lcgl->data);
    memset (lcgl->name, 0, strlen (lcgl->name));
    free (lcgl->name);
    free (lcgl->channel_name);
    memset (lcgl, 0, sizeof (lcgl_t));

    free (lcgl);
}

void lcgl_begin(lcgl_t *lcgl, GLenum mode)
{
    lcgl_encode_u8(lcgl, LCGL_GL_BEGIN);
    lcgl_encode_u32(lcgl, mode);
}

void lcgl_end(lcgl_t *lcgl)
{
    lcgl_encode_u8(lcgl, LCGL_GL_END);
}

void lcgl_vertex2d(lcgl_t *lcgl, double v0, double v1)
{
    lcgl_encode_u8(lcgl, LCGL_GL_VERTEX2D);
    assert (isfinite (v0) && isfinite (v1));

    lcgl_encode_double(lcgl, v0);
    lcgl_encode_double(lcgl, v1);
}

void lcgl_vertex2f(lcgl_t *lcgl, float v0, float v1)
{
    lcgl_encode_u8(lcgl, LCGL_GL_VERTEX2F);
    assert (isfinite (v0) && isfinite (v1));

    lcgl_encode_float(lcgl, v0);
    lcgl_encode_float(lcgl, v1);
}

void lcgl_vertex3d(lcgl_t *lcgl, double v0, double v1, double v2)
{
    lcgl_encode_u8(lcgl, LCGL_GL_VERTEX3D);
    assert (isfinite (v0) && isfinite (v1) && isfinite (v2));

    lcgl_encode_double(lcgl, v0);
    lcgl_encode_double(lcgl, v1);
    lcgl_encode_double(lcgl, v2);
}

void lcgl_translated(lcgl_t *lcgl, double v0, double v1, double v2)
{
    lcgl_encode_u8(lcgl, LCGL_GL_TRANSLATED);
    assert (isfinite (v0) && isfinite (v1) && isfinite (v2));

    lcgl_encode_double(lcgl, v0);
    lcgl_encode_double(lcgl, v1);
    lcgl_encode_double(lcgl, v2);
}

void lcgl_rotated(lcgl_t *lcgl, double angle, double x, double y, double z)
{
    lcgl_encode_u8(lcgl, LCGL_GL_ROTATED);

    lcgl_encode_double(lcgl, angle);
    lcgl_encode_double(lcgl, x);
    lcgl_encode_double(lcgl, y);
    lcgl_encode_double(lcgl, z);
}

void lcgl_push_matrix (lcgl_t * lcgl)
{
    lcgl_encode_u8 (lcgl, LCGL_GL_PUSH_MATRIX);
}

void lcgl_pop_matrix (lcgl_t * lcgl)
{
    lcgl_encode_u8 (lcgl, LCGL_GL_POP_MATRIX);
}

void lcgl_load_identity(lcgl_t *lcgl)
{
    lcgl_encode_u8(lcgl, LCGL_GL_LOAD_IDENTITY);
}

void lcgl_color3f(lcgl_t *lcgl, float v0, float v1, float v2)
{
    lcgl_encode_u8(lcgl, LCGL_GL_COLOR3F);
    assert (isfinite (v0) && isfinite (v1) && isfinite (v2));
    lcgl_encode_float(lcgl, v0);
    lcgl_encode_float(lcgl, v1);
    lcgl_encode_float(lcgl, v2);
}

void lcgl_color4f(lcgl_t *lcgl, float v0, float v1, float v2, float v3)
{
    lcgl_encode_u8(lcgl, LCGL_GL_COLOR4F);
    lcgl_encode_float(lcgl, v0);
    lcgl_encode_float(lcgl, v1);
    lcgl_encode_float(lcgl, v2);
    lcgl_encode_float(lcgl, v3);
}

void lcgl_point_size(lcgl_t *lcgl, float v)
{
    lcgl_encode_u8(lcgl, LCGL_GL_POINTSIZE);
    lcgl_encode_float(lcgl, v);
}

void lcgl_enable(lcgl_t *lcgl, GLenum v)
{
    lcgl_encode_u8(lcgl, LCGL_GL_ENABLE);
    lcgl_encode_u32(lcgl, v);
}

void lcgl_disable(lcgl_t *lcgl, GLenum v)
{
    lcgl_encode_u8(lcgl, LCGL_GL_DISABLE);
    lcgl_encode_u32(lcgl, v);
}

void lcgl_box_3dv_3fv(lcgl_t *lcgl, double xyz[3], float dim[3])
{
    lcgl_encode_u8(lcgl, LCGL_BOX);
    lcgl_encode_double(lcgl, xyz[0]);
    lcgl_encode_double(lcgl, xyz[1]);
    lcgl_encode_double(lcgl, xyz[2]);
    lcgl_encode_float(lcgl, dim[0]);
    lcgl_encode_float(lcgl, dim[1]);
    lcgl_encode_float(lcgl, dim[2]);
}

void lcgl_circle(lcgl_t *lcgl, double xyz[3], double radius)
{
    lcgl_encode_u8(lcgl, LCGL_CIRCLE);
    lcgl_encode_double(lcgl, xyz[0]);
    lcgl_encode_double(lcgl, xyz[1]);
    lcgl_encode_double(lcgl, xyz[2]);
    lcgl_encode_float(lcgl, radius);
}

void lcgl_disk(lcgl_t *lcgl, double xyz[3], double r_in, double r_out)
{
    lcgl_encode_u8(lcgl, LCGL_DISK);
    lcgl_encode_double(lcgl, xyz[0]);
    lcgl_encode_double(lcgl, xyz[1]);
    lcgl_encode_double(lcgl, xyz[2]);
    lcgl_encode_float(lcgl, r_in);
    lcgl_encode_float(lcgl, r_out);
}

void lcgl_line_width(lcgl_t *lcgl, float line_width)
{
    lcgl_encode_u8(lcgl, LCGL_GL_LINE_WIDTH);
    lcgl_encode_float(lcgl, line_width);
}

void lcgl_text_ex(lcgl_t *lcgl, const double xyz[3], const char *text, uint32_t font, uint32_t flags)
{
    lcgl_encode_u8(lcgl, LCGL_TEXT_LONG);
    lcgl_encode_u32(lcgl, font);
    lcgl_encode_u32(lcgl, flags);

    lcgl_encode_double(lcgl, xyz[0]);
    lcgl_encode_double(lcgl, xyz[1]);
    lcgl_encode_double(lcgl, xyz[2]);

    int len = strlen(text);

    lcgl_encode_u32(lcgl, len);
    for (int i = 0; i < len; i++)
        lcgl_encode_u8(lcgl, text[i]);
}

void lcgl_text(lcgl_t *lcgl, const double xyz[3], const char *text)
{
    lcgl_text_ex(lcgl, xyz, text, 0, 
                 GLUTIL_DRAW_TEXT_DROP_SHADOW | 
                 GLUTIL_DRAW_TEXT_JUSTIFY_CENTER |
                 GLUTIL_DRAW_TEXT_ANCHOR_HCENTER |
                 GLUTIL_DRAW_TEXT_ANCHOR_VCENTER);
}

//////// vertex buffer 
lcgl_vertex_buffer_t *lcgl_vertex_buffer_create(int capacity, lcgl_vertex_buffer_full_callback_t full_callback, void *user)
{
    lcgl_vertex_buffer_t *vertbuf = (lcgl_vertex_buffer_t*) calloc(1, sizeof(lcgl_vertex_buffer_t));
    vertbuf->size = 0;
    vertbuf->capacity = capacity;
    vertbuf->vertices = (struct lcgl_vertex*) calloc(capacity, sizeof(struct lcgl_vertex));
    vertbuf->full_callback = full_callback;
    vertbuf->user = user;

    return vertbuf;
}

void lcgl_vertex_buffer_destroy(lcgl_vertex_buffer_t *vertbuf)
{
    free(vertbuf->vertices);
    free(vertbuf);
}

void lcgl_vertex_buffer_flush(lcgl_vertex_buffer_t *vertbuf)
{
    if (vertbuf->size)
        vertbuf->full_callback(vertbuf, vertbuf->user);
}

void lcgl_vertex_buffer_add(lcgl_vertex_buffer_t *vertbuf, double v[3])
{
    assert(vertbuf->size < vertbuf->capacity);

    memcpy(vertbuf->vertices[vertbuf->size].xyz, v, sizeof(double)*3);
    vertbuf->size++;
    if (vertbuf->size == vertbuf->capacity)
        lcgl_vertex_buffer_flush(vertbuf);
}

void lcgl_vertex_buffer_send(lcgl_vertex_buffer_t *vertbuf, lcgl_t *lcgl)
{
    for (int i = 0; i < vertbuf->size; i++) {
/*        printf("%f %f %f\n", vertbuf->vertices[i].xyz[0],
               vertbuf->vertices[i].xyz[1],
               vertbuf->vertices[i].xyz[2]);*/
        lcglVertex3dv(vertbuf->vertices[i].xyz);
    }
}

void lcgl_rect(lcgl_t *lcgl, double xyz[3], double size[2], double theta, int filled)
{
    lcgl_encode_u8(lcgl, LCGL_RECT);

    lcgl_encode_double(lcgl, xyz[0]);
    lcgl_encode_double(lcgl, xyz[1]);
    lcgl_encode_double(lcgl, xyz[2]);

    lcgl_encode_double(lcgl, size[0]);
    lcgl_encode_double(lcgl, size[1]);

    lcgl_encode_double(lcgl, theta);

    lcgl_encode_u8(lcgl, filled);
}
