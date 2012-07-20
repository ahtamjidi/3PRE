#include <GL/gl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include <dgc/lcgl.h>
#include <common/fasttrig.h>
#include <glutil/glutil.h>
#include "lcmtypes/lcmtypes_lcgl_data_t.h"

union bytefloat
{
    uint8_t  bytes[4];
    float    f;
};

union bytedouble
{
    uint8_t  bytes[8];
    double   d;
};

typedef struct lcgl_decoder lcgl_decoder_t;
struct lcgl_decoder
{
    uint8_t *data;
    int      datalen;
    int      datapos;
};


static inline uint8_t lcgl_decode_u8(lcgl_decoder_t *ldec)
{
    return ldec->data[ldec->datapos++];
}

static inline uint32_t lcgl_decode_u32(lcgl_decoder_t *ldec)
{
    uint32_t v = 0;

    v += ldec->data[ldec->datapos++]<<24;
    v += ldec->data[ldec->datapos++]<<16;
    v += ldec->data[ldec->datapos++]<<8;
    v += ldec->data[ldec->datapos++]<<0;

    return v;
}

static inline float lcgl_decode_float(lcgl_decoder_t *ldec)
{
    union bytefloat u;

    for (int i = 0; i < 4; i++)
        u.bytes[i] = ldec->data[ldec->datapos++];

    return u.f;
}

static inline double lcgl_decode_double(lcgl_decoder_t *ldec)
{
    union bytedouble u;

    for (int i = 0; i < 8; i++)
        u.bytes[i] = ldec->data[ldec->datapos++];

    return u.d;
}


static void gl_box(double xyz[3], double dim[3])
{
    glBegin(GL_QUADS);

    glNormal3f(-1, 0, 0);
    glVertex3f(xyz[0]-dim[0]/2, xyz[1]-dim[1]/2, xyz[2]-dim[2]/2);
    glVertex3f(xyz[0]-dim[0]/2, xyz[1]-dim[1]/2, xyz[2]+dim[2]/2);
    glVertex3f(xyz[0]-dim[0]/2, xyz[1]+dim[1]/2, xyz[2]+dim[2]/2);
    glVertex3f(xyz[0]-dim[0]/2, xyz[1]+dim[1]/2, xyz[2]-dim[2]/2);

    glNormal3f(1, 0, 0);
    glVertex3f(xyz[0]+dim[0]/2, xyz[1]-dim[1]/2, xyz[2]-dim[2]/2);
    glVertex3f(xyz[0]+dim[0]/2, xyz[1]-dim[1]/2, xyz[2]+dim[2]/2);
    glVertex3f(xyz[0]+dim[0]/2, xyz[1]+dim[1]/2, xyz[2]+dim[2]/2);
    glVertex3f(xyz[0]+dim[0]/2, xyz[1]+dim[1]/2, xyz[2]-dim[2]/2);

    glNormal3f(0,0,-1);
    glVertex3f(xyz[0]-dim[0]/2, xyz[1]-dim[1]/2, xyz[2]-dim[2]/2);
    glVertex3f(xyz[0]-dim[0]/2, xyz[1]+dim[1]/2, xyz[2]-dim[2]/2);
    glVertex3f(xyz[0]+dim[0]/2, xyz[1]+dim[1]/2, xyz[2]-dim[2]/2);
    glVertex3f(xyz[0]+dim[0]/2, xyz[1]-dim[1]/2, xyz[2]-dim[2]/2);

    glNormal3f(0,0,1);
    glVertex3f(xyz[0]-dim[0]/2, xyz[1]-dim[1]/2, xyz[2]+dim[2]/2);
    glVertex3f(xyz[0]-dim[0]/2, xyz[1]+dim[1]/2, xyz[2]+dim[2]/2);
    glVertex3f(xyz[0]+dim[0]/2, xyz[1]+dim[1]/2, xyz[2]+dim[2]/2);
    glVertex3f(xyz[0]+dim[0]/2, xyz[1]-dim[1]/2, xyz[2]+dim[2]/2);

    glNormal3f(0,-1,0);
    glVertex3f(xyz[0]-dim[0]/2, xyz[1]-dim[1]/2, xyz[2]-dim[2]/2);
    glVertex3f(xyz[0]-dim[0]/2, xyz[1]-dim[1]/2, xyz[2]+dim[2]/2);
    glVertex3f(xyz[0]+dim[0]/2, xyz[1]-dim[1]/2, xyz[2]+dim[2]/2);
    glVertex3f(xyz[0]+dim[0]/2, xyz[1]-dim[1]/2, xyz[2]-dim[2]/2);

    glNormal3f(0,1,0);
    glVertex3f(xyz[0]-dim[0]/2, xyz[1]+dim[1]/2, xyz[2]-dim[2]/2);
    glVertex3f(xyz[0]-dim[0]/2, xyz[1]+dim[1]/2, xyz[2]+dim[2]/2);
    glVertex3f(xyz[0]+dim[0]/2, xyz[1]+dim[1]/2, xyz[2]+dim[2]/2);
    glVertex3f(xyz[0]+dim[0]/2, xyz[1]+dim[1]/2, xyz[2]-dim[2]/2);

    glEnd();
}

void lcgl_decode(uint8_t *data, int datalen)
{
    lcgl_decoder_t ldec;
    ldec.data = data;
    ldec.datalen = datalen;
    ldec.datapos = 0;

    while (ldec.datapos < ldec.datalen) {

        uint8_t opcode = lcgl_decode_u8(&ldec);
        switch (opcode) {
            
        case LCGL_GL_BEGIN:
        {
            uint32_t v = lcgl_decode_u32(&ldec);
            glBegin(v);
            break;
        }

        case LCGL_GL_END:
            glEnd();
            break;

        case LCGL_GL_VERTEX2D:
        {
            double v[2];
            for (int i = 0; i < 2; i++)
                v[i] = lcgl_decode_double(&ldec);
            
            glVertex2dv(v);
            break;
        }
        
        case LCGL_GL_VERTEX2F:
        {
            float v[2];
            for (int i = 0; i < 2; i++)
                v[i] = lcgl_decode_float(&ldec);
            
            glVertex2fv(v);
            break;
        }
        
        case LCGL_GL_VERTEX3F:
        {
            float v[3];
            for (int i = 0; i < 3; i++)
                v[i] = lcgl_decode_float(&ldec);

            glVertex3fv(v);
            break;
        }

        case LCGL_GL_VERTEX3D:
        {
            double v[3];
            for (int i = 0; i < 3; i++)
                v[i] = lcgl_decode_double(&ldec);

            glVertex3dv(v);
            break;
        }

        case LCGL_GL_TRANSLATED:
        {
            double v[3];
            for (int i = 0; i < 3; i++)
                v[i] = lcgl_decode_double(&ldec);

            glTranslated(v[0], v[1], v[2]);
            break;
        }

        case LCGL_GL_ROTATED:
        {
            double theta = lcgl_decode_double(&ldec);

            double v[3];
            for (int i = 0; i < 3; i++)
                v[i] = lcgl_decode_double(&ldec);

            glRotated(theta, v[0], v[1], v[2]);
            break;
        }

        case LCGL_GL_LOAD_IDENTITY:
        {
            glLoadIdentity();
            break;
        }

        case LCGL_GL_PUSH_MATRIX:
        {
            glPushMatrix ();
            break;
        }

        case LCGL_GL_POP_MATRIX:
        {
            glPopMatrix ();
            break;
        }

        case LCGL_GL_COLOR3F:
        {
            float v[3];
            for (int i = 0; i < 3; i++)
                v[i] = lcgl_decode_float(&ldec);

            glColor3fv(v);
            break;
        }

        case LCGL_GL_COLOR4F:
        {
            float v[4];
            for (int i = 0; i < 4; i++)
                v[i] = lcgl_decode_float(&ldec);

            glColor4fv(v);
            break;
        }

        case LCGL_GL_POINTSIZE:
            glPointSize(lcgl_decode_float(&ldec));
            break;
            
        case LCGL_GL_LINE_WIDTH:
            glLineWidth(lcgl_decode_float(&ldec));
            break;

        case LCGL_GL_ENABLE:
            glEnable(lcgl_decode_u32(&ldec));
            break;
            
        case LCGL_GL_DISABLE:
            glDisable(lcgl_decode_u32(&ldec));
            break;
            
        case LCGL_NOP:
            break;

        case LCGL_BOX:
        {
            double xyz[3];
            for (int i = 0; i < 3; i++)
                xyz[i] = lcgl_decode_double(&ldec);
            double dim[3];
            for (int i = 0; i < 3; i++)
                dim[i] = lcgl_decode_float(&ldec);

            gl_box(xyz, dim);
            break;
        }

        case LCGL_RECT:
        {
            double xyz[3], size[2], theta;

            for (int i = 0; i < 3; i++)
                xyz[i] = lcgl_decode_double(&ldec);

            for (int i = 0; i < 2; i++)
                size[i] = lcgl_decode_double(&ldec);

            theta = lcgl_decode_double(&ldec);

            int filled = lcgl_decode_u8(&ldec);

            glPushMatrix();
            glTranslated(xyz[0], xyz[1], xyz[2]);
            glRotated(M_PI / 180.0 * theta, 0, 0, 1);
                
            if (filled) 
                glBegin(GL_QUADS);
            else
                glBegin(GL_LINE_LOOP);

            glVertex3d(-size[0]/2, -size[1]/2, xyz[2]);
            glVertex3d(-size[0]/2,  size[1]/2, xyz[2]);
            glVertex3d( size[0]/2,  size[1]/2, xyz[2]);
            glVertex3d( size[0]/2, -size[1]/2, xyz[2]);
            glEnd();

            glPopMatrix();
            break;
        }

        case LCGL_CIRCLE:
        {
            double xyz[3];

            for (int i = 0; i < 3; i++)
                xyz[i] = lcgl_decode_double(&ldec);
            float radius = lcgl_decode_float(&ldec);

            glBegin(GL_LINE_STRIP);
            int segments = 40;

            for (int i = 0; i <= segments; i++) {
                double s,c;
                fasttrig_sincos(2*M_PI/segments * i, &s, &c);
                glVertex3d(xyz[0] + c*radius, xyz[1] + s*radius, xyz[2]);
            }

            glEnd();
            break;
        }

        case LCGL_DISK:
        {
            double xyz[3];

            for (int i = 0; i < 3; i++)
                xyz[i] = lcgl_decode_double(&ldec);
            float r_in = lcgl_decode_float(&ldec);
            float r_out = lcgl_decode_float(&ldec);

            GLUquadricObj *q = gluNewQuadric();
            glPushMatrix();
            glTranslatef(xyz[0], xyz[1], xyz[2]);
            gluDisk(q, r_in, r_out, 15, 1);
            glPopMatrix();
            gluDeleteQuadric(q);
            break;
        }
        
        case LCGL_TEXT:
        {
            int font = lcgl_decode_u8(&ldec);
            int flags = lcgl_decode_u8(&ldec);

            (void) font; (void) flags;
            double xyz[3];
            for (int i = 0; i < 3; i++)
                xyz[i] = lcgl_decode_double(&ldec);

            int len = lcgl_decode_u32(&ldec);
            char buf[len+1];
            for (int i = 0; i < len; i++) 
                buf[i] = lcgl_decode_u8(&ldec);
            buf[len] = 0;

            glutil_draw_text(xyz, NULL, buf, 0);
            break;
        }

        case LCGL_TEXT_LONG:
        {
            uint32_t font = lcgl_decode_u32(&ldec);
            uint32_t flags = lcgl_decode_u32(&ldec);

            (void) font;

            double xyz[3];
            for (int i = 0; i < 3; i++)
                xyz[i] = lcgl_decode_double(&ldec);

            int len = lcgl_decode_u32(&ldec);
            char buf[len+1];
            for (int i = 0; i < len; i++) 
                buf[i] = lcgl_decode_u8(&ldec);
            buf[len] = 0;

            glutil_draw_text(xyz, NULL, buf, flags);
            break;
        }

        default:
            printf("lcgl unknown opcode %d\n", opcode);
            break;
        }
    }
}

