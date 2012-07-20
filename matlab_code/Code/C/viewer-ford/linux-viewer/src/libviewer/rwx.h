#ifndef __rwx_h__
#define __rwx_h__

// Renderware file format parser

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

typedef struct _rwx_vertex
{
    double pos[3];
    int id;
} rwx_vertex_t;

typedef struct _rwx_triangle
{
    int vertices[3];
} rwx_triangle_t;

typedef struct _rwx_clump
{
    double color[3];
//    double surface[3];
    double diffuse;
    double specular;
    double opacity;
    double ambient;
    char *name;
    rwx_vertex_t *vertices;
    rwx_triangle_t *triangles;
    int nvertices;
    int ntriangles;
} rwx_clump_t;

typedef struct _rwx_model
{
    GList *clumps;
    int nclumps;
} rwx_model_t;


rwx_model_t * rwx_model_create( const char *fname );

void rwx_model_destroy( rwx_model_t *model );

void rwx_model_gl_draw( rwx_model_t *model );

#ifdef __cplusplus
}
#endif

#endif
