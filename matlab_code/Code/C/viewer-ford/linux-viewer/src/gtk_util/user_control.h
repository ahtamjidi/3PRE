#ifndef __gtku_user_control_h__
#define __gtku_user_control_h__

#include <glib-object.h>

#ifdef __cplusplus 
extern "C" {
#endif

/**
 * SECTION:user_control
 * @short_description:  User adjustable / automatically changing control value.
 * 
 * GtkuUserControl represents a control value that can both automatically change
 * and can be adjusted by the user.
 *
 * A control is strongly typed on construction, and
 * the following types of controls are currently supported:  Integer, Boolean,
 * Menu (enum), String.  The current value of a control can be queried using
 * the gtku_user_control_get_* methods.  There are two ways to possibly set the
 * value of a control - the polite way, and the forceful way.
 *
 * The forceful way of setting a control involves using the
 * gtku_user_control_force_set_* methods.  The value of the control will always
 * change, and the "value-changed" signal will be emitted. 
 *
 * The polite way of setting a control involves using the
 * #gtku_user_control_set_callback and gtku_user_control_try_set_* methods.  
 * The idea is that it is sometimes possible that the requested control value
 * is not acceptable, and should either not change at all, or should change to
 * a value other than what the user requested.  Each time a
 * gtku_user_control_try_set_* method is called, the callback is invoked to see
 * what the actual control value should be.  For example, the user may request
 * a specific shutter speed for a camera unit that is not allowed when the
 * camera is used in a specific mode.
 *
 * The GtkuUserControl class itself does not contain any rendering code to
 * provide a GUI interface.  However, each control may provide "hints" as to
 * how the control should be displayed.  For example, a string control may
 * provide the #GTKU_USER_CONTROL_FILENAME hint that suggests the control
 * actually represents a filename, and should be displayed with a file chooser
 * widget.
 */

/**
 * GtkuUserControlType:
 *
 * enumerates the different types of controls available
 */
typedef enum {
    // no valid control should every have this type
    GTKU_USER_CONTROL_TYPE_INVALID = 0,

    // Represents an integer control with a minimum value, maximum value, and
    // a step size.
    GTKU_USER_CONTROL_TYPE_INT = 1,

    // Represents a boolean control that can be either True or False
    GTKU_USER_CONTROL_TYPE_BOOLEAN = 2,

    // Represents an enumeration of choices from which one can be selected.
    GTKU_USER_CONTROL_TYPE_MENU = 3,

    // Represents a string-valued control
    GTKU_USER_CONTROL_TYPE_STRING = 4,

    // Represents a floating point control
    GTKU_USER_CONTROL_TYPE_FLOAT = 5,
} GtkuUserControlType;

typedef enum {
    GTKU_USER_CONTROL_FILENAME = 1,
    GTKU_USER_CONTROL_TOGGLE_BUTTON = (1 << 1),
    GTKU_USER_CONTROL_RADIO_BUTTONS = (1 << 2),
    GTKU_USER_CONTROL_TEXT_ENTRY = (1 << 3),
} GtkuUserControlUIHint;


typedef struct _GtkuUserControl GtkuUserControl;
typedef struct _GtkuUserControlClass GtkuUserControlClass;

#define GTKU_TYPE_USER_CONTROL  gtku_user_control_get_type()
#define GTKU_USER_CONTROL(obj)  (G_TYPE_CHECK_INSTANCE_CAST( (obj), \
        GTKU_TYPE_USER_CONTROL, GtkuUserControl))
#define GTKU_USER_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
            GTKU_TYPE_USER_CONTROL, GtkuUserControlClass ))
#define IS_GTKU_USER_CONTROL(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
            GTKU_TYPE_USER_CONTROL ))
#define IS_GTKU_USER_CONTROL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE( \
            (klass), GTKU_TYPE_USER_CONTROL))
#define GTKU_USER_CONTROL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), \
            GTKU_TYPE_USER_CONTROL, GtkuUserControlClass))

/**
 * GtkuUserControlCallback:
 * @ctl:      the GtkuUserControl in question
 * @proposed: the value passed in to gtku_user_control_try_set_val
 * @actual:   output parameter.  If the control value is to change, the
 *            function must set actual to the new value for the control.
 *            Often, it is sufficient to simply call g_value_copy( proposed,
 *            actual ).  However, it is possible that the control value may
 *            change, but not as requested.  In this case, actual can differ
 *            from proposed.
 * @user_data: the user_data parameter passed in to
 *             gtku_user_control_set_callback
 *
 * Returns: FALSE if the control value should not change.  TRUE if the control
 *          value should change to the output parameter actual.
 */
typedef gboolean (*GtkuUserControlCallback)(const GtkuUserControl *ctl, 
        const GValue *proposed, GValue *actual, void *user_data);

/**
 * GtkuUserControl:
 *
 * A control option on a GtkuUser that the user or a program can adjust.
 */
struct _GtkuUserControl {
    GObject parent;
    GtkuUserControlType type;
    int id;
    char * name;

	/*< private >*/
    int enabled;

    GtkuUserControlCallback try_set_function;
    void *user_data;

    char **menu_entries;
    int * menu_entries_enabled;
    int max_int;
    int min_int;
    int step_int;

    float max_float;
    float min_float;
    float step_float;
    int display_width;
    int display_prec;

    int ui_hints;

    GValue val;
    GValue initial_val;
};

struct _GtkuUserControlClass {
    GObjectClass parent_class;
};

GType gtku_user_control_get_type();

/**
 * gtku_user_control_set_callback:
 * @callback: the callback function to invoke when using the
 *            gtku_user_control_try_set_* methods.
 *
 * Sets the function which will be invoked when any of the
 * gtku_user_control_try_set_* methods are called.  If the callback returns
 * FALSE, then the control will not be set.  If the callback returns TRUE,
 * then the control will be adjusted.  See #GtkuUserControlCallback.  In
 * general, you shouldn't need to call this when subclassing from #GtkuUser, as
 * #GtkuUser does it automagically.
 */
void gtku_user_control_set_callback( GtkuUserControl *self,
        GtkuUserControlCallback callback, void *user_data );

/**
 * gtku_user_control_new_menu:
 * @id: a numerical identifier for the control.
 * @name: a nickname / human-understandable-name for the control.
 * @initial_index: the initial value for the control.
 * @enabled: TRUE of the control should be initially enabled
 * @entries: a NULL-terminated array of strings specifying the different
 *           options to present to the user.
 * @entries_enabled: an array of integers indicating whether each entry
 *           in @entries should be enabled.  1 indicates enabled, and 0
 *           disabled.  Specify NULL to implicitly enable all entries.
 *
 * Returns: a new menu control.
 */
GtkuUserControl * gtku_user_control_new_menu( int id, const char *name, 
        int initial_index, int enabled, const char **entries,
        const int * entries_enabled);

/**
 * gtku_user_control_new_int:
 * @id: a numerical identifier for the control.
 * @name: a nickname / human-understandable-name for the control.
 * @min: the minimum value the control can take on
 * @max: the maximum value the control can take on
 * @step: the step size with which the control is allowed to change.  
 * @initial_val: the initial value for the control.
 * @enabled: TRUE of the control should be initially enabled
 *
 * Returns: a new integer control
 */
GtkuUserControl * gtku_user_control_new_int( int id, const char *name, 
        int min, int max, int step, int initial_val, int enabled );

/**
 * gtku_user_control_new_float:
 * @id: a numerical identifier for the control.
 * @name: a nickname / human-understandable-name for the control.
 * @min: the minimum value the control can take on
 * @max: the maximum value the control can take on
 * @step: the step size with which the control is allowed to change.  
 * @initial_val: the initial value for the control.
 * @enabled: TRUE of the control should be initially enabled
 *
 * Returns: a new floating-point control
 */
GtkuUserControl * gtku_user_control_new_float( int id, const char *name, 
        float min, float max, float step, float initial_val, int enabled );

/**
 * gtku_user_control_new_boolean:
 * @id: a numerical identifier for the control.
 * @name: a nickname / human-understandable-name for the control.
 * @initial_val: the initial value for the control.
 * @enabled: TRUE of the control should be initially enabled
 * 
 * Returns: a new boolean control
 */
GtkuUserControl * gtku_user_control_new_boolean( int id, const char *name,
        int initial_val, int enabled );

/**
 * gtku_user_control_new_string:
 * @id: a numerical identifier for the control.
 * @name: a nickname / human-understandable-name for the control.
 * @initial_val: the initial value for the control.
 * @enabled: TRUE of the control should be initially enabled
 * 
 * Returns: a new string control
 */
GtkuUserControl * gtku_user_control_new_string( int id, const char *name,
        const char *initial_val, int enabled );

/**
 * gtku_user_control_unref:
 *
 * Decrements the reference count on the GtkuUserControl.  A GtkuUserControl
 * starts with a reference count of 1.  If this count ever reaches zero, the
 * control is destroyed.
 */
void gtku_user_control_unref( GtkuUserControl *self );

void gtku_user_control_modify_int (GtkuUserControl * self,
        int min, int max, int step, int enabled);
void gtku_user_control_modify_float (GtkuUserControl * self,
        float min, float max, float step, int enabled);

int gtku_user_control_try_set_val( GtkuUserControl *self, const GValue *val );
int gtku_user_control_try_set_int( GtkuUserControl *self, int val );
int gtku_user_control_try_set_float( GtkuUserControl *self, float val );
int gtku_user_control_try_set_menu( GtkuUserControl *self, int index );
int gtku_user_control_try_set_boolean( GtkuUserControl *self, int val );
int gtku_user_control_try_set_string( GtkuUserControl *self, const char *val );

int gtku_user_control_force_set_val(GtkuUserControl *self, const GValue *value);
int gtku_user_control_force_set_int(GtkuUserControl *self, int val );
int gtku_user_control_force_set_float(GtkuUserControl *self, float val );
int gtku_user_control_force_set_menu(GtkuUserControl *self, int val );
int gtku_user_control_force_set_boolean(GtkuUserControl *self, int val );
int gtku_user_control_force_set_string( GtkuUserControl *self, const char *val );

void gtku_user_control_get_val( const GtkuUserControl *self, GValue *value );
int gtku_user_control_get_int( const GtkuUserControl *self );
float gtku_user_control_get_float( const GtkuUserControl *self );
int gtku_user_control_get_menu( const GtkuUserControl *self );
int gtku_user_control_get_boolean( const GtkuUserControl *self );
const char* gtku_user_control_get_string( const GtkuUserControl *self );

int gtku_user_control_get_max_int( const GtkuUserControl *self );
int gtku_user_control_get_min_int( const GtkuUserControl *self );
int gtku_user_control_get_step_int( const GtkuUserControl *self );

float gtku_user_control_get_max_float( const GtkuUserControl *self );
float gtku_user_control_get_min_float( const GtkuUserControl *self );
float gtku_user_control_get_step_float( const GtkuUserControl *self );

void gtku_user_control_set_enabled( GtkuUserControl *self, int enabled );

int gtku_user_control_get_enabled( const GtkuUserControl *self );

#if 0
int gtku_user_control_set_display_scale (GtkuUserControl * self,
        float min, float max, int width, int prec);
#endif

/**
 * gtku_user_control_set_ui_hints:
 * @flags: a logical OR of #GtkuUserControlUIHint values
 *
 * See also: #GtkuUserControlUIHint
 */
void gtku_user_control_set_ui_hints( GtkuUserControl *self, int flags );

/**
 * gtku_user_control_get_ui_hints:
 *
 * See also: #GtkuUserControlUIHint
 *
 * Returns: a logical OR of #GtkuUserControlUIHint values
 */
int gtku_user_control_get_ui_hints( const GtkuUserControl *self );

#ifdef __cplusplus
}
#endif

#endif
