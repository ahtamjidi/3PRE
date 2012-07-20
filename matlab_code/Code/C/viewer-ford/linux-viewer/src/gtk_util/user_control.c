#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "user_control.h"

#define err(args...) fprintf(stderr, args)

enum {
    VALUE_CHANGED_SIGNAL,
    PARAMS_CHANGED_SIGNAL,
    LAST_SIGNAL
};

static guint gtku_user_control_signals[LAST_SIGNAL] = { 0 };

static void gtku_user_control_finalize( GObject *obj );
static void gtku_user_control_init( GtkuUserControl *self );
static void gtku_user_control_class_init( GtkuUserControlClass *klass );

G_DEFINE_TYPE (GtkuUserControl, gtku_user_control, G_TYPE_OBJECT);

static void
gtku_user_control_init( GtkuUserControl *self )
{
    self->type = 0;
    self->id = 0;
    self->name = NULL;
    self->enabled = 0;
    self->try_set_function = NULL;
    self->user_data = NULL;

    self->max_int = 0;
    self->min_int = 0;
    self->step_int = 0;

    self->menu_entries = NULL;
    self->menu_entries_enabled = NULL;

    memset( &self->val, 0, sizeof(self->val) );
    memset( &self->initial_val, 0, sizeof(self->initial_val) );
}

static void
gtku_user_control_class_init( GtkuUserControlClass *klass )
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = gtku_user_control_finalize;

    /**
     * GtkuUserControl::value-changed
     * @unit: the GtkuUser emitting the signal
     *
     * The value-changed signal is emitted when the value of the control is
     * changed.  This will always happen when any of gtku_user_control_force_set 
     * functions are called, and may happen when the gtku_user_control_try_set
     * functions are called
     */
    gtku_user_control_signals[VALUE_CHANGED_SIGNAL] = 
        g_signal_new("value-changed",
            G_TYPE_FROM_CLASS(klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
    /**
     * GtkuUserControl::parameters-changed
     * @unit: the GtkuUser emitting the signal
     *
     * The parameters-changed signal is emitted when properties of a
     * control are changed, such as min, max, step, or enabled by
     * gtku_user_control_set_enabled(), gtku_user_control_modify_int(),
     * or gtku_user_control_modify_float().
     */
    gtku_user_control_signals[PARAMS_CHANGED_SIGNAL] = 
        g_signal_new("parameters-changed",
            G_TYPE_FROM_CLASS(klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

static void
gtku_user_control_finalize( GObject *obj )
{
    GtkuUserControl *self = GTKU_USER_CONTROL( obj );

    free( self->name );
    if( self->menu_entries ) {
        int i;
        for( i=0; i <= self->max_int; i++ ) {
            free( self->menu_entries[i] );
        }
    }
    free(self->menu_entries);
    free(self->menu_entries_enabled);

    g_value_unset( &self->val );
    g_value_unset( &self->initial_val );

    G_OBJECT_CLASS (gtku_user_control_parent_class)->finalize(obj);
}

static GtkuUserControl *
gtku_user_control_new_basic( int id, const char *name, 
        int type, int enabled )
{
    GtkuUserControl *self = g_object_new(GTKU_TYPE_USER_CONTROL, NULL);
    self->id = id;
    self->type = type;
    self->name = strdup(name);
    self->enabled = enabled;
    return self;
}

GtkuUserControl * 
gtku_user_control_new_menu( int id,
        const char *name, int initial_index, int enabled,
        const char **entries, const int * entries_enabled )
{
    GtkuUserControl *self = gtku_user_control_new_basic( id, name, 
            GTKU_USER_CONTROL_TYPE_MENU, enabled );
    // how many entries?
    int nentries;
    for( nentries = 0; entries[nentries]; nentries++ );
    self->max_int = nentries - 1;
    self->min_int = 0;
    self->menu_entries = (char**) malloc((self->max_int+1)*sizeof(char*));
    int i;
    for( i=0; i<=self->max_int; i++ ) {
        self->menu_entries[i] = strdup( entries[i] );
    }
    if (entries_enabled) {
        self->menu_entries_enabled = (int*) malloc (nentries*sizeof (int));
        memcpy (self->menu_entries_enabled, entries_enabled,
                nentries * sizeof (int));
    }
    g_value_init( &self->val, G_TYPE_INT );
    g_value_set_int( &self->val, initial_index );
    g_value_init( &self->initial_val, G_TYPE_INT );
    g_value_set_int( &self->initial_val, initial_index );

    return self;
}

GtkuUserControl * 
gtku_user_control_new_int( int id, const char *name, 
        int min, int max, int step, int initial_val, int enabled )
{
    GtkuUserControl *self = gtku_user_control_new_basic( id, name, 
            GTKU_USER_CONTROL_TYPE_INT, enabled );
    self->min_int = min;
    self->max_int = max;
    self->step_int = step;
    g_value_init( &self->val, G_TYPE_INT );
    g_value_set_int( &self->val, initial_val );
    g_value_init( &self->initial_val, G_TYPE_INT );
    g_value_set_int( &self->initial_val, initial_val );
    return self;
}

void
gtku_user_control_modify_int (GtkuUserControl * self,
        int min, int max, int step, int enabled)
{
    self->min_int = min;
    self->max_int = max;
    self->step_int = step;
    self->enabled = enabled;
    g_signal_emit( G_OBJECT(self), 
            gtku_user_control_signals[PARAMS_CHANGED_SIGNAL], 0 );
}

static void
num_chars_float (float x, int sf, int * width, int * prec)
{
    int i = floor (log10 (x) + 1.0);
    int j = i - sf;
    if (i <= 0) {
        *width = -j + 2;
        *prec = -j;
    }
    else if (i > 0 && j < 0) {
        *width = i - j + 1;
        *prec = -j;
    }
    else {
        *width = i;
        *prec = 0;
    }
}

GtkuUserControl * 
gtku_user_control_new_float( int id, const char *name, 
        float min, float max, float step, float initial_val, int enabled )
{
    GtkuUserControl *self = gtku_user_control_new_basic( id, name, 
            GTKU_USER_CONTROL_TYPE_FLOAT, enabled );
    self->min_float = min;
    self->max_float = max;
    self->step_float = step;
    g_value_init( &self->val, G_TYPE_FLOAT );
    g_value_set_float( &self->val, initial_val );
    g_value_init( &self->initial_val, G_TYPE_FLOAT );
    g_value_set_float( &self->initial_val, initial_val );

    num_chars_float (self->max_float - self->min_float, 3,
            &self->display_width, &self->display_prec);
    return self;
}

void
gtku_user_control_modify_float (GtkuUserControl * self,
        float min, float max, float step, int enabled)
{
    self->min_float = min;
    self->max_float = max;
    self->step_float = step;
    self->enabled = enabled;
    g_signal_emit( G_OBJECT(self), 
            gtku_user_control_signals[PARAMS_CHANGED_SIGNAL], 0 );
}


GtkuUserControl * 
gtku_user_control_new_boolean( int id, const char *name,
        int initial_val, int enabled )
{
    GtkuUserControl *self = gtku_user_control_new_basic( id, name, 
            GTKU_USER_CONTROL_TYPE_BOOLEAN, enabled );
    self->min_int = 0;
    self->max_int = 1;
    self->step_int = 1;
    g_value_init( &self->val, G_TYPE_BOOLEAN );
    g_value_set_boolean( &self->val, initial_val );
    g_value_init( &self->initial_val, G_TYPE_BOOLEAN );
    g_value_set_boolean( &self->initial_val, initial_val );
    return self;
}

GtkuUserControl * 
gtku_user_control_new_string( int id, const char *name,
        const char *initial_val, int enabled )
{
    GtkuUserControl *self = gtku_user_control_new_basic( id, name,
            GTKU_USER_CONTROL_TYPE_STRING, enabled );
    g_value_init( &self->val, G_TYPE_STRING );
    g_value_set_string( &self->val, initial_val );
    g_value_init( &self->initial_val, G_TYPE_STRING );
    g_value_set_string( &self->initial_val, initial_val );
    return self;
}

void 
gtku_user_control_set_callback( GtkuUserControl *self,
        GtkuUserControlCallback callback, void *user_data )
{
    self->try_set_function = callback;
    self->user_data = user_data;
}

static int
check_type( GtkuUserControl *self, const GValue *value )
{
    GType expected_type = G_TYPE_INVALID;

    switch( self->type ) {
        case GTKU_USER_CONTROL_TYPE_MENU:
        case GTKU_USER_CONTROL_TYPE_INT:
            expected_type = G_TYPE_INT;
            break;
        case GTKU_USER_CONTROL_TYPE_FLOAT:
            expected_type = G_TYPE_FLOAT;
            break;
        case GTKU_USER_CONTROL_TYPE_BOOLEAN:
            expected_type = G_TYPE_BOOLEAN;
            break;
        case GTKU_USER_CONTROL_TYPE_STRING:
            expected_type = G_TYPE_STRING;
            break;
        default:
            err("UnitControl: unrecognized type %s\n", 
                    g_type_name(G_VALUE_TYPE(value)) );
            return 0;
    }

    if( expected_type == G_TYPE_INVALID ||
        expected_type != G_VALUE_TYPE(value) ) {
        err("UnitControl: [%s] expected %s, got %s\n",
                self->name, 
                g_type_name(expected_type),
                g_type_name(G_VALUE_TYPE(value)) );
        return 0;
    }
    return 1;
}

// ============ force set ============
int
gtku_user_control_force_set_val(GtkuUserControl *self, const GValue *value)
{
    if( ! check_type( self, value ) ) return -1;
    g_value_copy( value, &self->val );
    g_signal_emit( G_OBJECT(self), 
            gtku_user_control_signals[VALUE_CHANGED_SIGNAL], 0 );
    return 0;
}

int 
gtku_user_control_force_set_int(GtkuUserControl *self, int val )
{
    GValue gv = { 0, };
    g_value_init( &gv, G_TYPE_INT );
    g_value_set_int( &gv, val );
    int result = gtku_user_control_force_set_val( self, &gv );
    g_value_unset(&gv);
    return result;
}

int 
gtku_user_control_force_set_float(GtkuUserControl *self, float val )
{
    GValue gv = { 0, };
    g_value_init( &gv, G_TYPE_FLOAT );
    g_value_set_float( &gv, val );
    int result = gtku_user_control_force_set_val( self, &gv );
    g_value_unset(&gv);
    return result;
}

int 
gtku_user_control_force_set_menu(GtkuUserControl *self, int val )
{
    return gtku_user_control_force_set_int( self, val );
}

int 
gtku_user_control_force_set_boolean(GtkuUserControl *self, int val )
{
    GValue gv = { 0, };
    g_value_init( &gv, G_TYPE_BOOLEAN );
    g_value_set_boolean( &gv, val );
    int result = gtku_user_control_force_set_val( self, &gv );
    g_value_unset(&gv);
    return result;
}

int 
gtku_user_control_force_set_string(GtkuUserControl *self, const char *val)
{
    GValue gv = { 0, };
    g_value_init( &gv, G_TYPE_STRING );
    g_value_set_string( &gv, val );
    int result = gtku_user_control_force_set_val( self, &gv );
    g_value_unset( &gv );
    return result;
}

// ============ try set ============
int
gtku_user_control_try_set_val( GtkuUserControl *self, const GValue *value )
{
    if( ! check_type( self, value ) ) return -1;

    // check if something (i.e. a GtkuUser) has asked for veto power over
    // setting the control value
    if( self->try_set_function ) {
        GValue av = { 0, };
        g_value_init( &av, G_VALUE_TYPE(value) );
        if( self->try_set_function( self, value, &av, self->user_data ) ) {
            g_value_copy( &av, &self->val );
            g_value_unset( &av );
        } else {
            g_value_unset( &av );
            return -1;
        }
    } else {
        g_value_copy( value, &self->val );
    }

    g_signal_emit( G_OBJECT(self), 
            gtku_user_control_signals[VALUE_CHANGED_SIGNAL], 0 );
    
    return 0;
}

int 
gtku_user_control_try_set_int( GtkuUserControl *self, int val )
{
    GValue gv = { 0, };
    g_value_init( &gv, G_TYPE_INT );
    g_value_set_int( &gv, val );
    int result = gtku_user_control_try_set_val( self, &gv );
    g_value_unset( &gv );
    return result;
}

int 
gtku_user_control_try_set_float( GtkuUserControl *self, float val )
{
    GValue gv = { 0, };
    g_value_init( &gv, G_TYPE_FLOAT );
    g_value_set_float( &gv, val );
    int result = gtku_user_control_try_set_val( self, &gv );
    g_value_unset( &gv );
    return result;
}

int 
gtku_user_control_try_set_menu( GtkuUserControl *self, int index )
{
    return gtku_user_control_try_set_int( self, index );
}

int 
gtku_user_control_try_set_boolean( GtkuUserControl *self, int val )
{
    GValue gv = { 0, };
    g_value_init( &gv, G_TYPE_BOOLEAN );
    g_value_set_boolean( &gv, val );
    int result = gtku_user_control_try_set_val( self, &gv );
    g_value_unset( &gv );
    return result;
}

int 
gtku_user_control_try_set_string(GtkuUserControl *self, const char *val)
{
    GValue gv = { 0, };
    g_value_init( &gv, G_TYPE_STRING );
    g_value_set_string( &gv, val );
    int result = gtku_user_control_try_set_val( self, &gv );
    g_value_unset( &gv );
    return result;
}

// ============== get ==============
void
gtku_user_control_get_val( const GtkuUserControl *self, GValue *value )
{
    g_value_init( value, G_VALUE_TYPE(&self->val) );
    g_value_copy( &self->val, value );
}
int 
gtku_user_control_get_int( const GtkuUserControl *self )
{
    return g_value_get_int( &self->val );
}
float
gtku_user_control_get_float( const GtkuUserControl *self )
{
    return g_value_get_float( &self->val );
}
int 
gtku_user_control_get_menu( const GtkuUserControl *self )
{
    return g_value_get_int( &self->val );
}
int 
gtku_user_control_get_boolean( const GtkuUserControl *self )
{
    return g_value_get_boolean( &self->val );
}
const char *
gtku_user_control_get_string( const GtkuUserControl *self )
{
    return g_value_get_string( &self->val );
}

// =========
int 
gtku_user_control_get_max_int( const GtkuUserControl *self )
{ return self->max_int; }
int 
gtku_user_control_get_min_int( const GtkuUserControl *self )
{ return self->min_int; }
int 
gtku_user_control_get_step_int( const GtkuUserControl *self )
{ return self->step_int; }

float 
gtku_user_control_get_max_float( const GtkuUserControl *self )
{ return self->max_float; }
float
gtku_user_control_get_min_float( const GtkuUserControl *self )
{ return self->min_float; }
float
gtku_user_control_get_step_float( const GtkuUserControl *self )
{ return self->step_float; }

void 
gtku_user_control_set_enabled( GtkuUserControl *self, int enabled )
{
    if( enabled != self->enabled ) {
        self->enabled = enabled;
        g_signal_emit( G_OBJECT(self), 
                gtku_user_control_signals[PARAMS_CHANGED_SIGNAL], 0 );
    } 
}

int 
gtku_user_control_get_enabled( const GtkuUserControl *self )
{
    return self->enabled;
}

void 
gtku_user_control_unref( GtkuUserControl *self )
{
    g_object_unref( self );
}

void 
gtku_user_control_set_ui_hints( GtkuUserControl *self, int flags )
{
    self->ui_hints = flags;
}

int 
gtku_user_control_get_ui_hints( const GtkuUserControl *self )
{
    return self->ui_hints;
}
