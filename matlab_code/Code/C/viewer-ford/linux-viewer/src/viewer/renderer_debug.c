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
#include <common/math_util.h>
#include <common/gridmap.h>
#include <glutil/glutil.h>
#include <dgc/globals.h>
#include <dgc/config_util.h>
#include <dgc/gridmap_util.h>

#include <lcmtypes/lcmtypes_rect_list_t.h>
#include <lcmtypes/lcmtypes_gridmap_tile_t.h>
#include <lcmtypes/lcmtypes_obstacle_list_t.h>

#include <libviewer/viewer.h>
#include <common/jpeg.h>
#include <glutil/texture.h>
#include <dirent.h>

#include "tile_set.h"

#define RENDERER_NAME "Debug"

#define MAX_TILE_AGE 1

#define SAT_TILE_WIDTH 256
#define SAT_TILE_HEIGHT 256
#define MAX_SATELLITE_CAR_DIST 500 // max distance between a satellite tile and the car, in meters

#define PARAM_RECTS "Rects"
#define PARAM_LANE_COST "Lane cost"
#define PARAM_OBSTACLE_COST "Obstacle cost"
#define PARAM_SATELLITE_IMAGE "Satellite image"
//#define PARAM_OBSTACLE_COST_COLORED "Cost color-coding"
#define PARAM_HAZARD_MAP "Hazard map"
#define PARAM_MASK_MAP "Mask map"
#define PARAM_CLINE_MAP "Centerline map"
#define PARAM_NAME_OBST_OPACITY "Obst opacity"
#define PARAM_NAME_SAT_OPACITY "Sat opacity"
#define PARAM_NAME_SATELLITE_XOFF "Satellite x offset"
#define PARAM_NAME_SATELLITE_YOFF "Satellite y offset"

typedef struct _satellite_tile { double x0, y0, x1, y1; GLUtilTexture    *tex; } satellite_tile_t;

typedef struct _RendererDebug {
    Renderer         renderer;

    Config           *config;
    lcm_t             *lc;
    GHashTable       *channels_hashtable;
    GPtrArray        *channels;
    Viewer           *viewer;
    GtkuParamWidget  *pw;
    GHashTable       *tileset_hashtable;
    CTrans           *ctrans;
    unsigned char    *sat_data;   // satellite image data
    int              sat_texture_valid;
    GArray           *satellite_tiles;
    lcmtypes_rect_list_t      *rects;
} RendererDebug;

static void
_latlon_local_pos (RendererDebug *self, double lat, double lon, double p[3])
{
    double gps[3] = { lat, lon, 0 };
    ctrans_gps_to_local (self->ctrans, gps, p, NULL);
}

/* load satellite images into a set of textures */
void load_satellite_images( RendererDebug *self, const char *dirname )
{
    // open the directory
    DIR *dirp = opendir( dirname );
    if ( dirp == NULL ) {
        printf("Error reading directory %s\n", dirname);
        return;
    }

    struct dirent  *dp;

    // for each image in the directory...
    while ((dp = readdir(dirp)) != NULL) {
        
        int id1,id2,id3,id4,id5,id6,id7,id8;
        
        // read lat and lon from filename
        if ( sscanf(dp->d_name, "%d.%d.%d.%d.%d.%d.%d.%d.jpg", &id1, &id2, &id3, &id4, \
                    &id5, &id6, &id7, &id8) != 8 ) 
            continue;

        // convert lat-lon to x-y coordinates
        double topleft_lat = (double)id1 + (double)id2 / 1000000;
        double topleft_lon = - (double)id3 - (double)id4 / 1000000;
        double bottomright_lat = (double)id5 + (double)id6 / 1000000;
        double bottomright_lon = - (double)id7 - (double)id8 / 1000000;

        // create a new satellite tile structure
        satellite_tile_t tt;
        
        tt.x0 = topleft_lat;
        tt.y0 = topleft_lon;
        tt.x1 = bottomright_lat;
        tt.y1 = bottomright_lon;
        tt.tex = glutil_texture_new (SAT_TILE_WIDTH, SAT_TILE_HEIGHT, 3 *SAT_TILE_WIDTH*SAT_TILE_HEIGHT);

        // load the file into the texture
        char fullname[512];
        sprintf(fullname, "%s/%s", dirname, dp->d_name);

        FILE *fp = fopen(fullname,"r");
        if ( !fp )
            continue;

        printf("Loading %s\n", fullname);

        // obtain file size
        fseek (fp , 0 , SEEK_END);
        int fp_size = ftell (fp);
        rewind (fp);
        
        // read the file
        uint8_t * src = (uint8_t*)malloc(fp_size);
        fread( src, 1, fp_size, fp);
        fclose(fp);

        int dest_size = 3*SAT_TILE_WIDTH*SAT_TILE_HEIGHT*sizeof(uint8_t);
        uint8_t *data = (uint8_t*)malloc(dest_size);
        
        // uncompress the jpeg image into data
        int jres = jpeg_decompress_to_8u_rgb(src, fp_size, data, 
                                             SAT_TILE_WIDTH,
                                             SAT_TILE_HEIGHT, 3*SAT_TILE_WIDTH);
        assert(jres==0);
    
        glutil_texture_upload( tt.tex, GL_RGB, GL_UNSIGNED_BYTE, 
                               3*SAT_TILE_WIDTH, data);
        free(src);
        free(data);

        // append the satellite tile to the global array
        g_array_append_val( self->satellite_tiles, tt );
    }

    closedir( dirp );

    self->sat_texture_valid = 1;
}

static void my_free( Renderer *renderer )
{
    RendererDebug *self = (RendererDebug*) renderer->user;

    g_hash_table_destroy ( self->tileset_hashtable );

    // unload the satellite tiles
    int i;
    for (i=0;i<self->satellite_tiles->len;i++) {
        satellite_tile_t *tt = &g_array_index( self->satellite_tiles, satellite_tile_t, i );
        glutil_texture_free( tt->tex );
    }
    
    g_array_free( self->satellite_tiles, 1 );
    
    free( self );
}

static void
on_obstacles (const lcm_recv_buf_t *rbuf, const char * channel, const lcmtypes_obstacle_list_t * msg, void * user)
{
    RendererDebug *self = (RendererDebug*) user;

    if (self->rects)
        lcmtypes_rect_list_t_destroy(self->rects);

    self->rects = lcmtypes_rect_list_t_copy(&msg->rects);
}

static void on_gridmap_tile(const lcm_recv_buf_t *rbuf, const char *channel, const lcmtypes_gridmap_tile_t *tile, void *user)
{
    RendererDebug *self = (RendererDebug*) user;

    double pos[3] = { 0, 0, 0 };
    ctrans_local_pos (self->ctrans, pos);
    
    if (!strcmp(channel, "OBSTACLE_MAP") && 
        gtku_param_widget_get_bool(self->pw, PARAM_OBSTACLE_COST)) {
        tile_set_process_new_tile(g_hash_table_lookup(self->tileset_hashtable, channel), tile);
    }

    if (!strcmp(channel, "HAZARD_MAP") && gtku_param_widget_get_bool(self->pw, PARAM_HAZARD_MAP)) {
        tile_set_process_new_tile(g_hash_table_lookup(self->tileset_hashtable, channel), tile);
    }

    if (!strcmp(channel, "MASK_MAP") && gtku_param_widget_get_bool(self->pw, PARAM_MASK_MAP)) {
        tile_set_process_new_tile(g_hash_table_lookup(self->tileset_hashtable, channel), tile);
    }
    
    viewer_request_redraw(self->viewer);
}

static void draw_rects(RendererDebug *self)
{
    if (!self->rects)
        return;

    lcmtypes_rect_t *rects = self->rects->rects;
    for (int i = 0; i < self->rects->num_rects; i++) {
        double x0 = self->rects->xy[0] + rects[i].dxy[0];
        double y0 = self->rects->xy[1] + rects[i].dxy[1];
        double sx = rects[i].size[0] / 2;
        double sy = rects[i].size[1] / 2;

        double MIN_RECT_SIZE = 0.1;
        sx = fmax(sx, MIN_RECT_SIZE);
        sy = fmax(sy, MIN_RECT_SIZE);

        glPushMatrix();

        glTranslated(x0, y0, 0);
        glRotatef(to_degrees(rects[i].theta), 0, 0, 1);
        glColor3f(.3, .3, .3);
        glBegin(GL_QUADS);
        glVertex2f( - sx, - sy);
        glVertex2f( - sx, + sy);
        glVertex2f( + sx, + sy);
        glVertex2f( + sx, - sy);
        glEnd();

        glPopMatrix();
    }
}

static void my_draw( Viewer *viewer, Renderer *renderer )
{
    RendererDebug *self = (RendererDebug*) renderer->user;
    
    double carpos[3] = { 0, 0, 0 };
    ctrans_local_pos (self->ctrans, carpos);

        glEnable (GL_BLEND);
        glEnable (GL_RESCALE_NORMAL);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glShadeModel (GL_SMOOTH);

    // draw the satellite images
    if (gtku_param_widget_get_bool(self->pw, PARAM_SATELLITE_IMAGE) && 
            self->sat_texture_valid) {
        glColor4d (1, 1, 1, 
            gtku_param_widget_get_double (self->pw, PARAM_NAME_SAT_OPACITY));

        double xoffset = gtku_param_widget_get_double(self->pw, 
                PARAM_NAME_SATELLITE_XOFF);
        double yoffset = gtku_param_widget_get_double(self->pw, 
                PARAM_NAME_SATELLITE_YOFF);

        int i;
        for (i=0;i<self->satellite_tiles->len;i++) {
            satellite_tile_t *tt = 
                &g_array_index( self->satellite_tiles, satellite_tile_t, i );

            double topleft[3];
            double topright[3];
            double bottomleft[3];
            double bottomright[3];

            _latlon_local_pos( self, tt->x0, tt->y0, topleft );
            _latlon_local_pos( self, tt->x1, tt->y1, bottomright );
            _latlon_local_pos( self, tt->x0, tt->y1, bottomleft );
            _latlon_local_pos( self, tt->x1, tt->y0, topright );

            double cx = (topleft[0]+bottomright[0])/2;
            double cy = (topleft[1]+bottomleft[1])/2;
            if (fabs(cx-carpos[0])+fabs(cy-carpos[1]) < MAX_SATELLITE_CAR_DIST) 
                glutil_texture_draw_coords (tt->tex, 
                        topleft[0]+xoffset, topleft[1]+yoffset, 
                        topright[0]+xoffset, topright[1]+yoffset,
                        bottomright[0]+xoffset, bottomright[1]+yoffset, 
                        bottomleft[0]+xoffset, bottomleft[1]+yoffset);
        }
    }

    glPushMatrix ();
    glTranslatef (0, 0, carpos[2]);
    
/*    if (gtku_param_widget_get_bool(self->pw, PARAM_MASK_MAP)) {
        double color[4] = {.3, .3, 1, .5};
        glColor4dv(color);
        tile_set_draw(g_hash_table_lookup(self->tileset_hashtable, "MASK_MAP"));
    }
*/

    if (1) { 
        double alpha = gtku_param_widget_get_double (self->pw, PARAM_NAME_OBST_OPACITY);
        double color[4] = {1,1,1,alpha};
        glColor4dv(color);
        
        if (gtku_param_widget_get_bool(self->pw, PARAM_OBSTACLE_COST))
            tile_set_draw(g_hash_table_lookup(self->tileset_hashtable, "OBSTACLE_MAP"));
    }
    
    if (gtku_param_widget_get_bool(self->pw, PARAM_HAZARD_MAP)) {
        glColor4d(1,1,1,1);
        tile_set_draw(g_hash_table_lookup(self->tileset_hashtable, "HAZARD_MAP"));
    }

    if (gtku_param_widget_get_bool(self->pw, PARAM_RECTS))
        draw_rects(self);
    
    glPopMatrix ();
/*
    double xyz[] = {.5, .5, 0};
    glutil_draw_text(xyz, NULL, "This\nIs\nA\nVery long Test", 
                     GLUTIL_DRAW_TEXT_NORMALIZED_SCREEN_COORDINATES |
                     GLUTIL_DRAW_TEXT_JUSTIFY_CENTER |
                     GLUTIL_DRAW_TEXT_ANCHOR_BOTTOM |
                     GLUTIL_DRAW_TEXT_ANCHOR_RIGHT |
                     GLUTIL_DRAW_TEXT_DROP_SHADOW);

    double xyz2[] = {.5, .75, 0};
    glutil_draw_text(xyz2, NULL, "This",
                     GLUTIL_DRAW_TEXT_NORMALIZED_SCREEN_COORDINATES |
                     GLUTIL_DRAW_TEXT_JUSTIFY_CENTER |
                     GLUTIL_DRAW_TEXT_ANCHOR_BOTTOM |
                     GLUTIL_DRAW_TEXT_ANCHOR_RIGHT |
                     GLUTIL_DRAW_TEXT_DROP_SHADOW);
*/

    

}

static void on_param_widget_changed (GtkuParamWidget *pw, const char *name, void *user)
{
    RendererDebug *self = (RendererDebug*) user;
    (void) self;

    viewer_request_redraw(self->viewer);
}

void on_load_sat_activate(GtkMenuItem *mi, void *user)
{
    RendererDebug *self = (RendererDebug*) user;

           
    if (!self->sat_texture_valid) {
        
        // prompt user for a directory containing the satellite images
       
        GtkWidget *dialog;
                 
        dialog = gtk_file_chooser_dialog_new ("Select directory for satellite images",
                 NULL,
                 GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                 NULL);

        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
            char *dirname;
            dirname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
            if ( dirname )
                load_satellite_images( self, dirname );
            g_free (dirname);
        }
        
        gtk_widget_destroy (dialog);
        
    }  else {
        
        printf("Freeing %d textures\n", self->satellite_tiles->len);
        
        if ( self->satellite_tiles->len == 0 )
            return;
        
        // unload the satellite tiles
        int i;
        for (i=0;i<self->satellite_tiles->len;i++) {
            satellite_tile_t *tt = &g_array_index( self->satellite_tiles, satellite_tile_t, i );
            glutil_texture_free( tt->tex );
        }
        
        g_array_free( self->satellite_tiles, 1 );
        
        self->sat_texture_valid = 0;
    }
}

static void
on_load_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererDebug *self = user_data;
    gtku_param_widget_load_from_key_file (self->pw, keyfile, RENDERER_NAME);

    // always deactivate satellite underlay
    gtku_param_widget_set_bool( self->pw, PARAM_SATELLITE_IMAGE, 0 );
}

static void
on_save_preferences (Viewer *viewer, GKeyFile *keyfile, void *user_data)
{
    RendererDebug *self = user_data;
    gtku_param_widget_save_to_key_file (self->pw, keyfile, RENDERER_NAME);
}

void setup_renderer_debug(Viewer *viewer, int render_priority) 
{
    RendererDebug *self = 
        (RendererDebug*) calloc(1, sizeof(RendererDebug));

    Renderer *renderer = &self->renderer;

    renderer->draw = my_draw;
    renderer->destroy = my_free;
    renderer->name = RENDERER_NAME;
    self->pw = GTKU_PARAM_WIDGET(gtku_param_widget_new());
    renderer->widget = GTK_WIDGET(self->pw);
    renderer->enabled = 1;
    renderer->user = self;

    self->lc = globals_get_lcm();
    self->ctrans = globals_get_ctrans();
    self->config = globals_get_config();
    self->channels_hashtable = g_hash_table_new(g_str_hash, g_str_equal);
    self->channels = g_ptr_array_new();
    self->viewer = viewer;
    self->tileset_hashtable = g_hash_table_new(g_str_hash, g_str_equal);
    self->satellite_tiles = g_array_new(FALSE, FALSE, sizeof(satellite_tile_t));
    self->sat_texture_valid = 0;

    gtku_param_widget_add_booleans (self->pw, 0, PARAM_RECTS, 1, NULL);
    gtku_param_widget_add_booleans (self->pw, 0, PARAM_OBSTACLE_COST, 1, NULL);
    gtku_param_widget_add_booleans (self->pw, 0, PARAM_SATELLITE_IMAGE, 0, NULL);
    gtku_param_widget_add_double (self->pw, 
                                  PARAM_NAME_SATELLITE_XOFF,
                                  GTKU_PARAM_WIDGET_SLIDER, -10, 10, .1, 0);
    gtku_param_widget_add_double (self->pw, 
                                  PARAM_NAME_SATELLITE_YOFF,
                                  GTKU_PARAM_WIDGET_SLIDER, -10, 10, .1, 0);

    gtku_param_widget_add_double (self->pw, 
                                  PARAM_NAME_OBST_OPACITY, 
                                  GTKU_PARAM_WIDGET_SLIDER, 0, 1, 0.05, 0.3);
    gtku_param_widget_add_double (self->pw, 
                                  PARAM_NAME_SAT_OPACITY, 
                                  GTKU_PARAM_WIDGET_SLIDER, 0, 1, 0.05, 1);
    gtku_param_widget_add_booleans (self->pw, 0, PARAM_HAZARD_MAP, 0, NULL);
    gtku_param_widget_add_booleans (self->pw, 0, PARAM_MASK_MAP, 1, NULL);

    // Load
    GtkWidget *load_sat = gtk_menu_item_new_with_mnemonic ("_Load/Unload satellite images...");
    gtk_menu_append (GTK_MENU(viewer->file_menu), load_sat);
    gtk_widget_show (load_sat);
    g_signal_connect (G_OBJECT (load_sat), "activate",
                      G_CALLBACK (on_load_sat_activate), self);

    g_signal_connect (G_OBJECT (self->pw), "changed", 
                      G_CALLBACK (on_param_widget_changed), self);
    
    ///////////////////////////////////////////////////////
    // obstacle hazard map
    if (1) {
        int sz = 256;
        float *r = (float*) malloc(sz * sizeof(float));
        float *g = (float*) malloc(sz * sizeof(float));
        float *b = (float*) malloc(sz * sizeof(float));
        float *a = (float*) malloc(sz * sizeof(float));

        for (int i = 0; i < 256; i++) {
            double f = ((float) i) / 255;
            double scale = 1.0;
            r[i] = f * scale;
            g[i] = f * scale;
            b[i] = f * scale;
            a[i] = 1;
            
            if (i & 1) {
                r[i] = 0;
                g[i] = 0;
            }
        }
        
        r[255] = 1.0;
        g[255] = .3;
        b[255] = .3;
        r[254] = 1.0;
        g[254] = .3;
        b[254] = .3;

        g_hash_table_insert(self->tileset_hashtable, "OBSTACLE_MAP", 
                            tile_set_create_colormap(r, g, b, a, sz));
    }

    ///////////////////////////////////////////////////////
    // hazard color map
    if (1) {
        int sz = 256;
        float *r = (float*) malloc(sz * sizeof(float));
        float *g = (float*) malloc(sz * sizeof(float));
        float *b = (float*) malloc(sz * sizeof(float));
        float *a = (float*) malloc(sz * sizeof(float));

        for (int i = 0; i < 256; i++) {
            double f = ((float) i) / 255;
            double scale = 0.8;
            r[i] = f * scale;
            g[i] = 0;
            b[i] = 0;
            a[i] = .5;
        } 

        g[1] = .5;
        b[1] = .5;

        g_hash_table_insert(self->tileset_hashtable, "HAZARD_MAP", 
                            tile_set_create_colormap(r, g, b, a, sz));
    }

    if (g_file_test ("sat", G_FILE_TEST_IS_DIR)) 
        load_satellite_images (self, "sat");
    
    lcmtypes_obstacle_list_t_subscribe (self->lc, "OBSTACLES", on_obstacles, self);
    lcmtypes_gridmap_tile_t_subscribe(self->lc, "OBSTACLE_MAP", on_gridmap_tile, self);
    lcmtypes_gridmap_tile_t_subscribe(self->lc, "LANE_MAP", on_gridmap_tile, self);
    lcmtypes_gridmap_tile_t_subscribe(self->lc, "HAZARD_MAP", on_gridmap_tile, self);
    lcmtypes_gridmap_tile_t_subscribe(self->lc, "MASK_MAP", on_gridmap_tile, self);

    viewer_add_renderer(viewer, renderer, render_priority);

    g_signal_connect (G_OBJECT (viewer), "load-preferences", 
            G_CALLBACK (on_load_preferences), self);
    g_signal_connect (G_OBJECT (viewer), "save-preferences",
            G_CALLBACK (on_save_preferences), self);
}
