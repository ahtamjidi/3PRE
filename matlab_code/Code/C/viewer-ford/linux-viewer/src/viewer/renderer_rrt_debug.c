#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <lcm/lcm.h>
#include <gtk_util/gtk_util.h>
#include <common/small_linalg.h>
#include <common/glib_util.h>
#include <common/config.h>
#include <common/fasttrig.h>

#include <dgc/globals.h>
#include <dgc/config_util.h>

#include <lcmtypes/lcmtypes_rrt_debug_t.h>

#include <libviewer/viewer.h>

#define PARAM_MIN_PLOT "min ID"
#define PARAM_MAX_PLOT "max ID"
#define PARAM_MIN0     "min offset"
#define PARAM_MAX0     "max offset"
#define MAX_ID_TO_PLOT ((double)1e4)

#define PARAM_NAME_RENDER_CTRL "Ctrl Input"
#define PARAM_NAME_RENDER_STATE "States"

typedef struct _RendererRrtDebug {
    Renderer renderer;

    Config *config;
    lcm_t *lc;
    lcmtypes_rrt_debug_t *data;
    int64_t      min_node_id;
    GtkuParamWidget *pw;
    int plot_ctrl;
    int plot_state;

    Viewer *viewer;

    CTrans * ctrans;
} RendererRrtDebug;

static void my_free( Renderer *renderer )
{
    RendererRrtDebug *self = (RendererRrtDebug*) renderer->user;

    if (self->lc)     globals_release_lcm (self->lc);
    if (self->ctrans) globals_release_ctrans (self->ctrans);
    if (self->data)   lcmtypes_rrt_debug_t_destroy (self->data);
        
    free( self );
}

static void my_draw( Viewer *viewer, Renderer *renderer )
{
    RendererRrtDebug *self = (RendererRrtDebug*) renderer->user;

    if (!self->data) return;

    double min0 = gtku_param_widget_get_double(self->pw, PARAM_MIN0);
    double max0 = gtku_param_widget_get_double(self->pw, PARAM_MAX0);
    int id_min = gtku_param_widget_get_int(self->pw, PARAM_MIN_PLOT);
    int id_max = gtku_param_widget_get_int(self->pw, PARAM_MAX_PLOT);
    if (max0 == MAX_ID_TO_PLOT) { // Plot everything
        max0 = INT_MAX;
    }
    int64_t min_id_plot = self->min_node_id + min0 + id_min;
    int64_t max_id_plot = self->min_node_id + max0 + id_max;

    double p_local[3], z;
    ctrans_local_pos (self->ctrans, p_local);
    z = p_local[2] - 0.06;
    
    // Controller input
    if (self->plot_ctrl) {
        glLineWidth (0.1);
        // Forward and Reverse
        for (int forward = 0; forward < 2; forward++) {
            if (forward == 1)
                glColor3f (0.439, 0.439, 0.439);    // silver for forward
            else
                glColor3f (0.6, 0.4, 0.0);     // gold for reverse
            
            for( int i = 0; i < self->data->num_nodes_plot; i++ ) {
                lcmtypes_rrt_debug_node_t *node = &self->data->node[i];
                
                 // skip out of range, other direction
                if ((node->npt_input <= 1) || (node->is_forward != forward) ||
                    (node->id < min_id_plot) || (node->id > max_id_plot)) {
                    continue;  
                }
               
                glBegin (GL_LINE_STRIP);
                glVertex3f (node->input[0].x, node->input[0].y, z);
                glVertex3f (node->input[1].x, node->input[1].y, z);
                glEnd ();
            }        
        }
    }
    
    // Predicted states
    if (self->plot_state) {
        glLineWidth (0.1);
        double best_total_cost = DBL_MAX;
        for( int i = 0; i < self->data->num_nodes_plot; i++ ) {
            double UB_cost = self->data->node[i].UB_cost;
            if (UB_cost > 0)
                best_total_cost = MIN (best_total_cost, 
                            UB_cost + self->data->node[i].cumulative_cost);
        }
        glEnable (GL_BLEND);
        for( int i = 0; i < self->data->num_nodes_plot; i++ ) {
            lcmtypes_rrt_debug_node_t *node = &self->data->node[i];
            
             // skip out of range
            if ((node->npt_state <= 1) ||
                (node->id < min_id_plot) || (node->id > max_id_plot)) {
                continue;  
            }
            float alpha;
            if (node->UB_cost < 0) {    // Not reaching the target
                alpha = 0.3;
                if (node->is_forward) 
                    glColor4f (0.75, 0.60, 0.50, alpha);  // Khaki
                else              
                    glColor4f (0.13, 0.67, 0.66, alpha);  // Sea green
            }
            else {
                double ratio = best_total_cost / 
                             (node->UB_cost + node->cumulative_cost);
                alpha = MIN (MAX (0.2, ratio*ratio*ratio*ratio), 0.9);
                if (node->is_forward) 
                    glColor4f (0.5, 0.5, 1.0, alpha);  // Purple
                else              
                    glColor4f (0.0, 0.4, 0.0, alpha);  // Dark green
                // Change color for the best ones    
                if (node->UB_cost + node->cumulative_cost < best_total_cost + 1) {
                    glColor4f(1, 1, 0, 0.9);
                }                    
            }
            if (node->is_safe == 0) {    // Unsafe node
                glColor4f (0.9, 0.0, 0.0, alpha);   // Red
            }
            
            glBegin (GL_LINE_STRIP);
            for (int j = 0; j < node->npt_state; j++) {
                glVertex3f (node->state[j].x, node->state[j].y, z);
            }
            glEnd ();        
        }        
        glDisable (GL_BLEND);
    }
}

static int64_t
get_min_node_id (lcmtypes_rrt_debug_t *rrt_dbg)
{
    int64_t min_id = INT64_MAX;
    if (rrt_dbg == NULL) return min_id;
    
    for (int i = 0; i < rrt_dbg->num_nodes_plot; i++) {
        min_id = MIN (min_id, rrt_dbg->node[i].id);
    }
    
    return min_id;
}

static void on_rrt_debug (const lcm_recv_buf_t *rbuf, const char *channel, 
        const lcmtypes_rrt_debug_t *msg, void *user_data )
{
    RendererRrtDebug *self = (RendererRrtDebug*) user_data;

    if (!ctrans_have_pose (self->ctrans))
        return;

    if (self->data)
        lcmtypes_rrt_debug_t_destroy (self->data);
    self->data = lcmtypes_rrt_debug_t_copy(msg);

    self->min_node_id = get_min_node_id (self->data);

    viewer_request_redraw(self->viewer);
}

static void 
on_param_widget_changed (GtkuParamWidget *pw, const char *name, 
        RendererRrtDebug *self)
{
    if (! strcmp (name, PARAM_NAME_RENDER_CTRL)) {
        self->plot_ctrl 
            = gtku_param_widget_get_bool (self->pw, PARAM_NAME_RENDER_CTRL);
    }
    else if (! strcmp (name, PARAM_NAME_RENDER_STATE)) {
        self->plot_state
            = gtku_param_widget_get_bool (self->pw, PARAM_NAME_RENDER_STATE);        
    }
    viewer_request_redraw (self->viewer);
}

void setup_renderer_rrt_debug(Viewer *viewer, int priority) 
{
    RendererRrtDebug *self = 
        (RendererRrtDebug*) calloc(1, sizeof(RendererRrtDebug));    

    Renderer *renderer = &self->renderer;

    renderer->draw = my_draw;
    renderer->destroy = my_free;
    renderer->name = "RRT Debug";
    renderer->widget = gtk_vbox_new(FALSE, 0);
    renderer->enabled = 1;
    renderer->user = self;

    self->lc = globals_get_lcm();
    self->ctrans = globals_get_ctrans();
    self->viewer = viewer;
    self->data = NULL;
    self->min_node_id = 0;
    self->plot_ctrl = 0;
    self->plot_state = 1;

    self->pw = GTKU_PARAM_WIDGET(gtku_param_widget_new());
 
    gtk_box_pack_start(GTK_BOX(renderer->widget), GTK_WIDGET(self->pw), TRUE, TRUE, 0);

    gtku_param_widget_add_double (self->pw, PARAM_MIN0, 
            GTKU_PARAM_WIDGET_SLIDER, 0, 1e4, 1e2, 0);
    gtku_param_widget_add_int (self->pw, PARAM_MIN_PLOT, 
            GTKU_PARAM_WIDGET_SLIDER, 0, 5e2, 1, 0);
    gtku_param_widget_add_double (self->pw, PARAM_MAX0, 
            GTKU_PARAM_WIDGET_SLIDER, 0, MAX_ID_TO_PLOT, 1e2, MAX_ID_TO_PLOT);
    gtku_param_widget_add_int (self->pw, PARAM_MAX_PLOT, 
            GTKU_PARAM_WIDGET_SLIDER, 0, 5e2, 1, 5e2);
    gtku_param_widget_add_booleans (self->pw, 0,
            PARAM_NAME_RENDER_CTRL, self->plot_ctrl, 
            PARAM_NAME_RENDER_STATE, self->plot_state, NULL);
            
    gtk_widget_show_all(renderer->widget);
   
    g_signal_connect (G_OBJECT (self->pw), "changed", 
            G_CALLBACK (on_param_widget_changed), self);
            
    lcmtypes_rrt_debug_t_subscribe(self->lc, "RRT_DEBUG", on_rrt_debug, self);

    viewer_add_renderer(viewer, renderer, priority);
}
