#ifndef __glutil_splot2d_h__
#define __glutil_splot2d_h__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GLUTIL_SPLOT2D_HIDDEN,
    GLUTIL_SPLOT2D_TOP_LEFT,
    GLUTIL_SPLOT2D_TOP_RIGHT,
    GLUTIL_SPLOT2D_BOTTOM_LEFT,
    GLUTIL_SPLOT2D_BOTTOM_RIGHT
} GLUtilSPlot2dLegendLocation;

typedef struct _GLUtilSPlot2d GLUtilSPlot2d;

GLUtilSPlot2d * glutil_splot2d_new ();

void glutil_splot2d_free (GLUtilSPlot2d *self);

int glutil_splot2d_set_title (GLUtilSPlot2d *self, const char *title);

int glutil_splot2d_set_text_color (GLUtilSPlot2d *self, 
        double r, double g, double b, double a);

void glutil_splot2d_set_show_title (GLUtilSPlot2d *self, int val);

//void glutil_splot2d_set_show_ylim (GLUtilSPlot2d *self, int val);

int glutil_splot2d_set_bgcolor (GLUtilSPlot2d *self, 
        double r, double g, double b, double alpha);

int glutil_splot2d_set_border_color (GLUtilSPlot2d *self, 
        double r, double g, double b, double alpha);

int glutil_splot2d_set_show_legend (GLUtilSPlot2d *self,
        GLUtilSPlot2dLegendLocation where);

int glutil_splot2d_set_xlim (GLUtilSPlot2d *self, double xmin, double xmax);

int glutil_splot2d_set_ylim (GLUtilSPlot2d *self, double ymin, double ymax);

int glutil_splot2d_add_plot (GLUtilSPlot2d *self, const char *name,
        int max_points);

int glutil_splot2d_remove_plot (GLUtilSPlot2d *self, const char *name);

int glutil_splot2d_set_color (GLUtilSPlot2d *self, const char *name, 
        double r, double g, double b, double alpha);

int glutil_splot2d_add_point (GLUtilSPlot2d *self, const char *name,
        double x, double y);

/**
 * renders the plot in a square from [ 0, 0 ] to [ 1, 1 ]
 */
//void glutil_splot2d_gl_render (GLUtilSPlot2d *self);

void glutil_splot2d_gl_render_at_window_pos (GLUtilSPlot2d *self,
        int topleft_x, int topleft_y, int width, int height);

#ifdef __cplusplus
}
#endif

#endif
