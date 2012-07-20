#include <math.h>
#include <assert.h>

#include <common/gridmap.h>
#include <common/math_util.h>

#include <dgc/gridmap_util.h>


/** Given a current gridmap, import the data and realign the gridmap
    so that pos[] is near the center.  If the provided gridmap is NULL
    or is not compatible with the tile, a new gridmap is created (and
    the old destroyed). The gridmap is returned. 
**/
gridmap_t *gridmap_util_import_tile(gridmap_t *gm, const lcmtypes_gridmap_tile_t *tile, const double pos[3])
{
    if (gm == NULL || gm->meters_per_pixel != tile->meters_per_pixel) {
        double sizex = gm ? gm->sizex : 200;
        double sizey = gm ? gm->sizey : 200;
        if (gm)
            gridmap_destroy(gm);
        gm = gridmap_create(0,0,sizex,sizey, tile->meters_per_pixel);
    }
    
    gridmap_recenter(gm, pos[0], pos[1], 5, 0);
    int xoff = (int) round((tile->x0 - gm->x0) / gm->meters_per_pixel);
    int yoff = (int) round((tile->y0 - gm->y0) / gm->meters_per_pixel);

//    if (xoff >=0 && yoff >=0 && xoff+tile->width <= gm->width && yoff+tile->height <= gm->height)
    if (gridmap_decode(gm, xoff, yoff, tile->width, tile->height, tile->data, tile->datalen))
        printf("gridmap decode error\n");

    return gm;
}


#define GRIDMAP_TILE_PIXELS 256

void gridmap_util_publish(const gridmap_t *gm, lcm_t *lc, const char *channel, int64_t *generation, int64_t utime)
{
    int total_size = 0;
    int num_tiles = 0;

    lcmtypes_gridmap_tile_t tile;
    memset(&tile, 0, sizeof(lcmtypes_gridmap_tile_t));
    int data_maxlen = 256 * 1024;
    uint8_t *buf = malloc(data_maxlen);
    if (!buf) {
        printf("gridmap_util_publish: out of memory!\n");
        return;
    }

    tile.data = buf;

    for (int x = 0; x < gm->width; x += GRIDMAP_TILE_PIXELS) {
        for (int y = 0; y < gm->height; y+= GRIDMAP_TILE_PIXELS) {

            tile.utime = utime;
            
            tile.width = imin(gm->width - x, GRIDMAP_TILE_PIXELS);
            tile.height = imin(gm->height - y, GRIDMAP_TILE_PIXELS);
            tile.x0 = gm->x0 + x * gm->meters_per_pixel;
            tile.y0 = gm->y0 + y * gm->meters_per_pixel;
            tile.meters_per_pixel = gm->meters_per_pixel;
            tile.generation = *generation;
            if (gridmap_encode(gm, x, y, tile.width, tile.height, tile.data, data_maxlen, &tile.datalen))
                printf("gridmap encode error\n");

            lcmtypes_gridmap_tile_t_publish(lc, channel, &tile);

            num_tiles ++;
            total_size += tile.datalen;
        }
    }
    
    *generation = (*generation)++;

    free(buf);
//    printf("tiles: %4d, total size: %15d\n", num_tiles, total_size);
}

gridmap_u16_t *
gridmap_util_import_tile_u16 (gridmap_u16_t *gm, 
        const lcmtypes_gridmap_tile_t *tile, const double pos[3])
{
    if (gm == NULL || gm->meters_per_pixel != tile->meters_per_pixel) {
        double sizex = gm ? gm->sizex : 200;
        double sizey = gm ? gm->sizey : 200;
        if (gm)
            gridmap_destroy_u16 (gm);
        gm = gridmap_create_u16 (0,0,sizex,sizey, tile->meters_per_pixel, 0);
    }
    
    gridmap_recenter_u16 (gm, pos[0], pos[1], 5, 0);
    int xoff = (int) round((tile->x0 - gm->x0) / gm->meters_per_pixel);
    int yoff = (int) round((tile->y0 - gm->y0) / gm->meters_per_pixel);

//    if (xoff >=0 && yoff >=0 && xoff+tile->width <= gm->width && yoff+tile->height <= gm->height)
    if (gridmap_decode_u16 (gm, xoff, yoff, tile->width, tile->height, 
                tile->data, tile->datalen))
        printf("gridmap decode error\n");

    return gm;
}

void 
gridmap_util_publish_u16 (const gridmap_u16_t *gm, lcm_t *lc, 
        const char *channel, int64_t *generation, int64_t utime)
{
    int total_size = 0;
    int num_tiles = 0;

    lcmtypes_gridmap_tile_t tile;
    memset(&tile, 0, sizeof(lcmtypes_gridmap_tile_t));
    int data_maxlen = 65536;
    uint8_t buf[data_maxlen];
    tile.data = buf;

#ifdef VALIDATE
    gridmap_u16_t *test = gridmap_create_u16 (gm->cx, gm->cy, 
            gm->sizex, gm->sizey, gm->meters_per_pixel, 0);
#endif

    for (int x = 0; x < gm->width; x += GRIDMAP_TILE_PIXELS) {
        for (int y = 0; y < gm->height; y+= GRIDMAP_TILE_PIXELS) {

            tile.utime = utime;
            
            tile.width = imin(gm->width - x, GRIDMAP_TILE_PIXELS);
            tile.height = imin(gm->height - y, GRIDMAP_TILE_PIXELS);
            tile.x0 = gm->x0 + x * gm->meters_per_pixel;
            tile.y0 = gm->y0 + y * gm->meters_per_pixel;
            tile.meters_per_pixel = gm->meters_per_pixel;
            tile.generation = *generation;
            if (gridmap_encode_u16 (gm, x, y, tile.width, tile.height, 
                        tile.data, data_maxlen, &tile.datalen))
                printf("gridmap encode error\n");

#ifdef VALIDATE
            uint8_t *p = tile.data;
            int i = 0, j = 0;
            for (uint8_t *p = tile.data; p < tile.data + tile.datalen; p += 6) {
                int runlen = (p[2] << 8) | p[3];
                int runcol = (p[4] << 8) | p[5];

                for (int k=0; k<runlen; k++) {
                    int gmc = gm->data[(y + i)*gm->width + (x+j)];
                    if (gmc != runcol) {
                        printf ("discrepancy: %d != %d (%d, %d) tile %d, %d w: %d h: %d\n",
                                gmc, runcol, x+j, y+i, x, y, tile.width, tile.height); }
                    j++;
                    if (j >= tile.width) {
                        j = 0;
                        i ++;
                    }
                }
            }
#endif

            lcmtypes_gridmap_tile_t_publish(lc, channel, &tile);

            num_tiles ++;
            total_size += tile.datalen;

#ifdef VALIDATE
            double c[3] = { gm->cx, gm->cy, 0 };
            test = gridmap_util_import_tile_u16 (test, &tile, c);
#endif
        }
    }

#ifdef VALIDATE
    printf ("validating...\n");
    int failed = 0;
    for (int i=0; i<gm->height; i++) {
        for (int j=0; j<gm->width; j++) {
            uint16_t gc = gm->data[i*gm->width+j];
            uint16_t tc = test->data[i*gm->width+j];
            if (gc != tc) {
                printf ("expected %d, got %d at (%d, %d)\n", gc, tc, j, i);
                failed = 1;
            }
        }
    }
    assert (!failed);
    gridmap_destroy_u16 (test);
#endif
    
    *generation = (*generation)++;

//    printf("tiles: %4d, total size: %15d\n", num_tiles, total_size);
}
