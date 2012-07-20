#ifndef __gtku_param_widget_h__
#define __gtku_param_widget_h__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTKU_TYPE_PARAM_WIDGET  gtku_param_widget_get_type()
#define GTKU_PARAM_WIDGET(obj)  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        GTKU_TYPE_PARAM_WIDGET, GtkuParamWidget))
#define GTKU_PARAM_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
            GTKU_TYPE_PARAM_WIDGET, GtkuParamWidgetClass))
#define IS_GTKU_PARAM_WIDGET(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
            GTKU_TYPE_PARAM_WIDGET))
#define IS_GTKU_PARAM_WIDGET_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE (\
            (klass), GTKU_TYPE_PARAM_WIDGET))

typedef struct _GtkuParamWidget GtkuParamWidget;
typedef struct _GtkuParamWidgetClass GtkuParamWidgetClass;

typedef enum 
{
    GTKU_PARAM_WIDGET_DEFAULTS = 0,

    // ui hints for integers
    GTKU_PARAM_WIDGET_SLIDER = 1,
    GTKU_PARAM_WIDGET_SPINBOX,

    // ui hints for enums
    GTKU_PARAM_WIDGET_MENU,
//    GTKU_PARAM_WIDGET_RADIO,

    // ui hints for booleans
    GTKU_PARAM_WIDGET_CHECKBOX,
    GTKU_PARAM_WIDGET_TOGGLE_BUTTON,
} GtkuParamWidgetUIHint;

struct _GtkuParamWidget
{
    GtkVBox vbox;

    /* private */
    GHashTable *params;
    GHashTable *widget_to_param;

    GList *widgets;

    char *estr;
};

struct _GtkuParamWidgetClass
{
    GtkVBoxClass parent_class;
};

GType        gtku_param_widget_get_type(void);
GtkWidget *  gtku_param_widget_new(void);

int gtku_param_widget_add_enum (GtkuParamWidget *pw,
        const char *name, GtkuParamWidgetUIHint ui_hints, int initial_value,
        const char *string1, int value1, ...) 
        __attribute__ ((sentinel));

int gtku_param_widget_add_enumv (GtkuParamWidget *pw,
        const char *name, GtkuParamWidgetUIHint ui_hints, int initial_value,
        int noptions, const char **names, const int *values);

int gtku_param_widget_add_int (GtkuParamWidget *pw,
        const char *name, GtkuParamWidgetUIHint ui_hints, 
        int min, int max, int increment, int initial_value);

int gtku_param_widget_add_double (GtkuParamWidget *pw,
        const char *name, GtkuParamWidgetUIHint ui_hints,
        double min, double max, double increment, double initial_value);

int gtku_param_widget_add_booleans (GtkuParamWidget *pw, 
        GtkuParamWidgetUIHint ui_hints, 
        const char *name1, int initially_checked1,
        ...) 
        __attribute__ ((sentinel));

int gtku_param_widget_get_int (GtkuParamWidget *pw, const char *name);

double gtku_param_widget_get_double (GtkuParamWidget *pw, const char *name);

int gtku_param_widget_get_bool (GtkuParamWidget *pw, const char *name);

int gtku_param_widget_get_enum (GtkuParamWidget *pw, const char *name);

const char* gtku_param_widget_get_enum_str (GtkuParamWidget *pw, 
        const char *name);

void gtku_param_widget_set_int (GtkuParamWidget *pw, const char *name, 
        int val);

void gtku_param_widget_set_double (GtkuParamWidget *pw, const char *name,
        double val);

void gtku_param_widget_set_bool (GtkuParamWidget *pw, const char *name,
        int val);

void gtku_param_widget_set_enum (GtkuParamWidget *pw, const char *name,
        int val);

void gtku_param_widget_load_from_key_file (GtkuParamWidget *pw, 
        GKeyFile *keyfile, const char *group_name);

void gtku_param_widget_save_to_key_file (GtkuParamWidget *pw,
        GKeyFile *keyfile, const char *group_name);

G_END_DECLS

#endif  /* __gtku_param_widget_h__ */
