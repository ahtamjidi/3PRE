#ifndef __GTKU_GL_IMAGE_AREA_H__
#define __GTKU_GL_IMAGE_AREA_H__

#include <GL/gl.h>

#include "gtkgldrawingarea.h"

G_BEGIN_DECLS

#define GTKU_TYPE_GL_IMAGE_AREA            (gtku_gl_image_area_get_type ())
#define GTKU_GL_IMAGE_AREA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTKU_TYPE_GL_IMAGE_AREA, GtkuGLImageArea))
#define GTKU_GL_IMAGE_AREA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTKU_TYPE_GL_IMAGE_AREA, GtkuGLImageAreaClass))
#define GTK_IS_GL_IMAGE_AREA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTKU_TYPE_GL_IMAGE_AREA))
#define GTK_IS_GL_IMAGE_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTKU_TYPE_GL_IMAGE_AREA))
#define GTKU_GL_IMAGE_AREA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTKU_TYPE_GL_IMAGE_AREA, GtkuGLImageAreaClass))

typedef struct _GtkuGLImageArea        GtkuGLImageArea;
typedef struct _GtkuGLImageAreaClass   GtkuGLImageAreaClass;

struct _GtkuGLImageArea {
    GtkuGLDrawingArea  parent;

    /*< private >*/
    GLenum target;
    GLint int_format;
    GLint format;
    GLuint texname;
    int width;
    int height;

    GLuint texc_width;
    GLuint texc_height;

    GLuint pbo;
    int use_pbo;
    int max_data_size;
};

struct _GtkuGLImageAreaClass {
    GtkDrawingAreaClass parent_class;
};

GType       gtku_gl_image_area_get_type (void);
GtkWidget * gtku_gl_image_area_new ();
int         gtku_gl_image_area_set_image_format (GtkuGLImageArea *self,
        int width, int height, GLenum format);
int         gtku_gl_image_area_upload_image (GtkuGLImageArea * self,
        const void *data, int row_stride);

G_END_DECLS

#endif

