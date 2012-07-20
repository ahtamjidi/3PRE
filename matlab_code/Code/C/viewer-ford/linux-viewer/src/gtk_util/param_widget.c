#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gtk/gtksignal.h>

#include "param_widget.h"

//#define dbg(args...) fprintf(stderr, args)
#define dbg(args...)
#define err(args...) fprintf(stderr, args)

typedef struct _param_data {
    char *name;
    GtkWidget *widget;
} param_data_t;

enum {
    CHANGED_SIGNAL,
    LAST_SIGNAL
};

static void gtku_param_widget_class_init (GtkuParamWidgetClass *klass);
static void gtku_param_widget_init (GtkuParamWidget *pw);
static void gtku_param_widget_finalize (GObject *obj);

static guint gtku_param_widget_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (GtkuParamWidget, gtku_param_widget, GTK_TYPE_VBOX);

static void
gtku_param_widget_class_init (GtkuParamWidgetClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = gtku_param_widget_finalize;

    gtku_param_widget_signals[CHANGED_SIGNAL] = g_signal_new("changed",
            G_TYPE_FROM_CLASS(klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            0,
            NULL,
            NULL,
            gtk_marshal_VOID__STRING,
            G_TYPE_NONE, 1, G_TYPE_STRING);
}

static void
gtku_param_widget_init (GtkuParamWidget *pw)
{
    dbg ("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);

    pw->widgets = NULL;
    pw->params = g_hash_table_new (g_str_hash, g_str_equal);
    pw->widget_to_param = g_hash_table_new (NULL, NULL);
    pw->estr = NULL;
}

static void
gtku_param_widget_finalize (GObject *obj)
{
    GtkuParamWidget *pw = GTKU_PARAM_WIDGET (obj);
    dbg ("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);

    if (pw->estr) {
        free (pw->estr);
        pw->estr = NULL;
    }

    G_OBJECT_CLASS (gtku_param_widget_parent_class)->finalize(obj);
}

GtkWidget *
gtku_param_widget_new (void)
{
    return GTK_WIDGET (g_object_new (GTKU_TYPE_PARAM_WIDGET, NULL));
}

static int
have_parameter_key (GtkuParamWidget *pw, const char *name)
{
    return g_hash_table_lookup (pw->params, name) != NULL;
}

static void
generic_widget_changed (GtkWidget *w, gpointer user_data)
{
    GtkuParamWidget *pw = GTKU_PARAM_WIDGET(user_data);
    param_data_t *pd = 
        (param_data_t*) g_hash_table_lookup (pw->widget_to_param, w);
//    dbg("[%s] changed\n", pd->name);
    g_signal_emit (G_OBJECT (pw), 
            gtku_param_widget_signals[CHANGED_SIGNAL], 0, pd->name);
}

static void
add_row (GtkuParamWidget *pw, const char *name,
        GtkWidget *w, const char *signal_name)
{
    GtkWidget *hb = gtk_hbox_new (FALSE, 0);
    GtkWidget *lbl = gtk_label_new (name);
    gtk_box_pack_start (GTK_BOX (hb), lbl, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hb), w, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (&pw->vbox), hb, TRUE, TRUE, 0);

    pw->widgets = g_list_append (pw->widgets, hb);
    pw->widgets = g_list_append (pw->widgets, lbl);
    pw->widgets = g_list_append (pw->widgets, w);

    g_hash_table_insert (pw->params, (gpointer)name, w);

    // TODO bookkeeping.  This leaks memory.
    param_data_t *pd = (param_data_t*) calloc (1, sizeof(param_data_t));
    pd->name = strdup (name);
    pd->widget = w;

    g_hash_table_insert (pw->widget_to_param, w, pd);

    if (signal_name && strlen (signal_name)) {
        g_signal_connect (G_OBJECT(w), signal_name,
                G_CALLBACK (generic_widget_changed), pw);
    }

    gtk_widget_show_all (hb);
}

int
gtku_param_widget_add_int(GtkuParamWidget *pw, const char *name, 
        GtkuParamWidgetUIHint ui_hints,
        int min, int max, int increment, int initial_value)
{
    if (have_parameter_key (pw, name)) return -1;
    if (min >= max || initial_value < min || initial_value > max) {
        err("WARNING: param_widget_add_int - invalid args\n");
        return -1;
    }

    GtkWidget *w = NULL;
    switch(ui_hints) {
        case GTKU_PARAM_WIDGET_SPINBOX:
            w = gtk_spin_button_new_with_range (min, max, increment);
            gtk_spin_button_set_value (GTK_SPIN_BUTTON(w), initial_value);
            break;
        case GTKU_PARAM_WIDGET_SLIDER:
        case GTKU_PARAM_WIDGET_DEFAULTS:
            w = gtk_hscale_new_with_range (min, max, increment);
            gtk_range_set_value (GTK_RANGE(w), initial_value);
            break;
        default:
            err("ERROR: param_widget_add_int - bad ui_hints\n");
            return -1;
    } 
    g_object_set_data (G_OBJECT(w), "data-type", "int");
    add_row (pw, name, w, "value-changed");
    return 0;
}

int gtku_param_widget_add_double (GtkuParamWidget *pw,
        const char *name, GtkuParamWidgetUIHint ui_hints, 
        double min, double max, double increment, double initial_value)
{
    if (have_parameter_key (pw, name)) return -1;
    if (min >= max || initial_value < min || initial_value > max) {
        err("WARNING: param_widget_add_int - invalid args\n");
        return -1;
    }

    GtkWidget *w = NULL;
    switch(ui_hints) {
        case GTKU_PARAM_WIDGET_SPINBOX:
            w = gtk_spin_button_new_with_range (min, max, increment);
            gtk_spin_button_set_value (GTK_SPIN_BUTTON(w), initial_value);
            break;
        case GTKU_PARAM_WIDGET_SLIDER:
        case GTKU_PARAM_WIDGET_DEFAULTS:
            w = gtk_hscale_new_with_range (min, max, increment);
            gtk_range_set_value (GTK_RANGE(w), initial_value);
            break;
        default:
            err("ERROR: param_widget_add_int - bad ui_hints\n");
            return -1;
    } 
    add_row (pw, name, w, "value-changed");
    g_object_set_data (G_OBJECT(w), "data-type", "double");
    return 0;
}

static int
add_checkboxes_helper (GtkuParamWidget *pw, GtkBox *box, const char *name, 
        int checked)
{
    if (have_parameter_key (pw, name)) return -1;
    GtkWidget *cb = gtk_check_button_new_with_label (name);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb), checked);

    gtk_box_pack_start (GTK_BOX (box), cb, FALSE, FALSE, 0);

    pw->widgets = g_list_append (pw->widgets, cb);

    g_hash_table_insert (pw->params, (gpointer)name, cb);
    g_object_set_data (G_OBJECT(cb), "data-type", "boolean");

    // TODO bookkeeping.  This leaks memory.
    param_data_t *pd = (param_data_t*) calloc (1, sizeof(param_data_t));
    pd->name = strdup (name);
    pd->widget = cb;

    g_hash_table_insert (pw->widget_to_param, cb, pd);

    g_signal_connect (G_OBJECT(cb), "toggled",
            G_CALLBACK (generic_widget_changed), pw);
    return 0;
}

static int
add_checkboxes (GtkuParamWidget *pw, 
        const char *name1, int initally_checked1,
        va_list ap)
{
    GtkBox *hb = GTK_BOX (gtk_hbox_new (FALSE, 0));

    int status;
    status = add_checkboxes_helper (pw, hb, name1, initally_checked1);
    if (0 != status) return status;

    int i;
    for (i=0; i<10000; i++) {
        const char *name = va_arg (ap, const char *);
        if (! name || strlen (name) == 0) break;

        int checked = va_arg (ap, int);

        status = add_checkboxes_helper (pw, hb, name, checked);
        if (0 != status) {
            return status;
        }
    }

    gtk_widget_show_all (GTK_WIDGET (hb));

    gtk_box_pack_start (GTK_BOX(&pw->vbox), GTK_WIDGET(hb), TRUE, TRUE, 0);
    return 0;
}

static int
add_togglebuttons_helper (GtkuParamWidget *pw, GtkBox *box, const char *name, 
        int checked)
{
    if (have_parameter_key (pw, name)) return -1;
    GtkWidget *tb = gtk_toggle_button_new_with_label (name);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tb), checked);

    gtk_box_pack_start (GTK_BOX (box), tb, FALSE, FALSE, 0);

    pw->widgets = g_list_append (pw->widgets, tb);

    g_hash_table_insert (pw->params, (gpointer)name, tb);
    g_object_set_data (G_OBJECT(tb), "data-type", "boolean");

    // TODO bookkeeping.  This leaks memory.
    param_data_t *pd = (param_data_t*) calloc (1, sizeof(param_data_t));
    pd->name = strdup (name);
    pd->widget = tb;

    g_hash_table_insert (pw->widget_to_param, tb, pd);

    g_signal_connect (G_OBJECT(tb), "toggled",
            G_CALLBACK (generic_widget_changed), pw);
    return 0;
}

static int
add_togglebuttons (GtkuParamWidget *pw, 
        const char *name1, int initally_checked1,
        va_list ap)
{
    GtkBox *hb = GTK_BOX (gtk_hbox_new (FALSE, 0));

    int status;
    status = add_togglebuttons_helper (pw, hb, name1, initally_checked1);
    if (0 != status) return status;

    int i;
    for (i=0; i<10000; i++) {
        const char *name = va_arg (ap, const char *);
        if (! name || strlen (name) == 0) break;

        int checked = va_arg (ap, int);

        status = add_togglebuttons_helper (pw, hb, name, checked);
        if (0 != status) {
            return status;
        }
    }

    gtk_widget_show_all (GTK_WIDGET (hb));

    gtk_box_pack_start (GTK_BOX(&pw->vbox), GTK_WIDGET(hb), TRUE, TRUE, 0);
    return 0;
}

int gtku_param_widget_add_booleans (GtkuParamWidget *pw, 
        GtkuParamWidgetUIHint ui_hints,
        const char *name1, int initally_checked1,
        ...)
{
    int status = -1;
    va_list ap;
    va_start (ap, initally_checked1);

    switch (ui_hints) {
        case GTKU_PARAM_WIDGET_TOGGLE_BUTTON:
            status = add_togglebuttons (pw, name1, initally_checked1, ap);
            break;
        case GTKU_PARAM_WIDGET_CHECKBOX:
        case GTKU_PARAM_WIDGET_DEFAULTS:
            status = add_checkboxes(pw, name1, initally_checked1, ap);
            break;
        default:
            err("ERROR: param_widget_add_booleans - bad ui_hints\n");
    }
    va_end (ap);
    return status;
}

static int
_add_menu (GtkuParamWidget *pw, const char *name, int initial_value,
        int noptions, const char **names, const int *values)
{
    GtkTreeIter iter;

    GtkListStore * store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
    int selected = -1;

    int i;
    for (i=0; i<noptions; i++) {
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, names[i], 1, values[i], -1);
        if (values[i] == initial_value) selected = i;
    }

    GtkWidget *mb = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));
    GtkCellRenderer * renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (mb), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (mb), renderer, "text", 0);
    g_object_set (G_OBJECT (renderer), "font", "monospace", NULL);
    g_object_unref (store);
    gtk_combo_box_set_active (GTK_COMBO_BOX(mb), selected);

    add_row (pw, name, mb, "changed");
    g_object_set_data (G_OBJECT(mb), "data-type", "enum");
    return 0;
}

int 
gtku_param_widget_add_enumv (GtkuParamWidget *pw,
        const char *name, GtkuParamWidgetUIHint ui_hints, int initial_value,
        int noptions, const char **names, const int *values)
{
    switch (ui_hints) {
        case GTKU_PARAM_WIDGET_MENU:
        case GTKU_PARAM_WIDGET_DEFAULTS:
            return _add_menu (pw, name, initial_value, 
                    noptions, names, values);
            break;
        default:
            err("ERROR: param_widget_add_enum - bad ui_hints\n");
    }
    return -1;
}

int 
gtku_param_widget_add_enum (GtkuParamWidget *pw,
        const char *name, GtkuParamWidgetUIHint ui_hints, int initial_value,
        const char *string1, int value1, ...)
{
    va_list ap;
    va_start (ap, value1);

    GPtrArray *names = g_ptr_array_new ();
    GArray *values = g_array_new (FALSE, FALSE, sizeof (int));
    g_ptr_array_add (names, strdup (string1));
    g_array_append_val (values, value1);

    while (1) {
        const char *str = va_arg (ap, const char *);
        if (! str || strlen (str) == 0) break;
        int val = va_arg (ap, int);

        g_ptr_array_add (names, strdup (str));
        g_array_append_val (values, val);
    }
    va_end (ap);

    int status = gtku_param_widget_add_enumv (pw, name, ui_hints, 
            initial_value, names->len, (const char **)names->pdata, 
            (int*) values->data);
    for (int i=0; i<values->len; i++) free (g_ptr_array_index (names, i));
    g_ptr_array_free (names, TRUE);
    g_array_free (values, TRUE);

    return status;
}

int gtku_param_widget_get_int (GtkuParamWidget *pw, const char *name)
{
    if (! have_parameter_key (pw, name)) {
        fprintf(stderr, "param_widget: invalid parameter [%s]\n", name);
        return 0;
    }
    GtkWidget *w = g_hash_table_lookup (pw->params, name);

    GType type = G_OBJECT_TYPE (w);

    if (GTK_TYPE_HSCALE == type) {
        return (int) gtk_range_get_value (GTK_RANGE (w));
    } else if ( GTK_TYPE_SPIN_BUTTON == type) {
        return (int) gtk_spin_button_get_value (GTK_SPIN_BUTTON (w));
    } else {
        fprintf(stderr, "param_widget:  can't retrieve parameter [%s] "
                "as integer.\n", name);
        return 0;
    }

    return 0;
}

double gtku_param_widget_get_double (GtkuParamWidget *pw, const char *name)
{
    if (! have_parameter_key (pw, name)) {
        fprintf(stderr, "param_widget: invalid parameter [%s]\n", name);
        return 0;
    }
    GtkWidget *w = g_hash_table_lookup (pw->params, name);

    GType type = G_OBJECT_TYPE (w);

    if (GTK_TYPE_HSCALE == type) {
        return gtk_range_get_value (GTK_RANGE (w));
    } else if ( GTK_TYPE_SPIN_BUTTON == type) {
        return gtk_spin_button_get_value (GTK_SPIN_BUTTON (w));
    } else {
        fprintf(stderr, "param_widget:  can't retrieve parameter [%s] "
                "as double.\n", name);
        return 0;
    }

    return 0;
}


int gtku_param_widget_get_bool (GtkuParamWidget *pw, const char *name)
{
    if (! have_parameter_key (pw, name)) {
        fprintf(stderr, "param_widget: invalid parameter [%s]\n", name);
        return 0;
    }
    GtkWidget *w = g_hash_table_lookup (pw->params, name);

    if (GTK_IS_TOGGLE_BUTTON (w)) {
        return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w));
    } else {
        fprintf(stderr, "param_widget:  can't retrieve parameter [%s] "
                "as boolean.\n", name);
        return 0;
    }

    return 0;
}

const char *
gtku_param_widget_get_enum_str (GtkuParamWidget *pw, const char *name)
{
    if (! have_parameter_key (pw, name)) {
        fprintf(stderr, "param_widget: invalid parameter [%s]\n", name);
        return NULL;
    }
    GtkWidget *w = g_hash_table_lookup (pw->params, name);

    GType type = G_OBJECT_TYPE (w);
    if (pw->estr) {
        free (pw->estr);
        pw->estr = NULL;
    }

    if (GTK_TYPE_COMBO_BOX == type) {
        GtkTreeIter iter;
        gtk_combo_box_get_active_iter (GTK_COMBO_BOX(w), &iter);
        GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX(w));
        gtk_tree_model_get (model, &iter, 0, &pw->estr, -1);
        return pw->estr;
    } else {
        fprintf(stderr, "param_widget:  can't retrieve parameter [%s] "
                "as string.\n", name);
        return NULL;
    }
    return NULL;
}

int 
gtku_param_widget_get_enum (GtkuParamWidget *pw, const char *name)
{
    if (! have_parameter_key (pw, name)) {
        fprintf(stderr, "param_widget: invalid parameter [%s]\n", name);
        return -1;
    }
    GtkWidget *w = g_hash_table_lookup (pw->params, name);

    int result;
    GType type = G_OBJECT_TYPE (w);

    if (GTK_TYPE_COMBO_BOX == type) {
        GtkTreeIter iter;
        gtk_combo_box_get_active_iter (GTK_COMBO_BOX(w), &iter);
        GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX(w));
        gtk_tree_model_get (model, &iter, 1, &result, -1);
        return result;
    } else {
        fprintf(stderr, "param_widget:  can't retrieve parameter [%s] "
                "as string.\n", name);
        return -1;
    }
    return -1;
}

void 
gtku_param_widget_set_int (GtkuParamWidget *pw, const char *name, 
        int val)
{
    if (! have_parameter_key (pw, name)) {
        fprintf(stderr, "param_widget: invalid parameter [%s]\n", name);
        return;
    }
    GtkWidget *w = g_hash_table_lookup (pw->params, name);
    GType type = G_OBJECT_TYPE (w);

    if (GTK_TYPE_SPIN_BUTTON == type) {
        gtk_spin_button_set_value (GTK_SPIN_BUTTON(w), val);
    } else if (GTK_TYPE_HSCALE == type) {
        gtk_range_set_value (GTK_RANGE(w), val);
    }
}

void 
gtku_param_widget_set_double (GtkuParamWidget *pw, const char *name,
        double val)
{
    if (! have_parameter_key (pw, name)) {
        fprintf(stderr, "param_widget: invalid parameter [%s]\n", name);
        return;
    }
    GtkWidget *w = g_hash_table_lookup (pw->params, name);
    GType type = G_OBJECT_TYPE (w);

    if (GTK_TYPE_SPIN_BUTTON == type) {
        gtk_spin_button_set_value (GTK_SPIN_BUTTON(w), val);
    } else if (GTK_TYPE_HSCALE == type) {
        gtk_range_set_value (GTK_RANGE(w), val);
    }
}

void 
gtku_param_widget_set_bool (GtkuParamWidget *pw, const char *name,
        int val)
{
    if (! have_parameter_key (pw, name)) {
        fprintf(stderr, "param_widget: invalid parameter [%s]\n", name);
        return;
    }
    GtkWidget *w = g_hash_table_lookup (pw->params, name);
    GType type = G_OBJECT_TYPE (w);
    if (GTK_TYPE_CHECK_BUTTON == type) {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), val);
    }
}

typedef struct _tree_model_match_data {
    int val;
    GtkComboBox *combo;
} tree_model_match_data_t;

gboolean
activate_if_matched (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
        void *user_data)
{
    tree_model_match_data_t *md = (tree_model_match_data_t*) user_data;

    int entry_val;
    gtk_tree_model_get (model, iter, 1, &entry_val, -1);
    
    if (entry_val == md->val) {
        gtk_combo_box_set_active_iter (md->combo, iter);
        return TRUE;
    }
    return FALSE;
}

void 
gtku_param_widget_set_enum (GtkuParamWidget *pw, const char *name,
        int val)
{
    if (! have_parameter_key (pw, name)) {
        fprintf(stderr, "param_widget: invalid parameter [%s]\n", name);
        return;
    }
    GtkWidget *w = g_hash_table_lookup (pw->params, name);
    GType type = G_OBJECT_TYPE (w);
    if (GTK_TYPE_COMBO_BOX == type) {
        GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX(w));
        tree_model_match_data_t matchdata = {
            val, 
            GTK_COMBO_BOX(w)
        };
        gtk_tree_model_foreach (model, activate_if_matched, &matchdata);
    }
}

typedef struct {
    GKeyFile *keyfile;
    const char *group_name;
    GtkuParamWidget *pw;
} _keyfile_data_t;

static void
_param_load_from_keyfile (void *key, void *value, void *user_data)
{
    GtkWidget *w = GTK_WIDGET (value);
    _keyfile_data_t *data = user_data;
    const char *param_name = key;
    const char *type = g_object_get_data (G_OBJECT(w), "data-type");
    GError *gerr = NULL;
    if (!strcmp (type, "boolean")) {
        gboolean val = g_key_file_get_boolean (data->keyfile, data->group_name,
                param_name, &gerr);
        if (!gerr) {
            gtku_param_widget_set_bool (data->pw, param_name, val);
        }
    } else if (!strcmp (type, "double")) {
        double val = g_key_file_get_double (data->keyfile, data->group_name,
                param_name, &gerr);
        if (!gerr) {
            gtku_param_widget_set_double (data->pw, param_name, val);
        }
    } else if (!strcmp (type, "int")) {
        int val = g_key_file_get_integer (data->keyfile, data->group_name,
                param_name, &gerr);
        if (!gerr) {
            gtku_param_widget_set_int (data->pw, param_name, val);
        }
    } else if (!strcmp (type, "enum")) {
        int val = g_key_file_get_integer (data->keyfile, data->group_name,
                param_name, &gerr);
        if (!gerr) {
            gtku_param_widget_set_enum (data->pw, param_name, val);
        }
    } else {
        // TODO
    }
    if (gerr) {
//        g_warning ("couldn't load param %s from GKeyFile\n", param_name);
//        g_warning ("%s\n", gerr->message);
        g_error_free (gerr);
    }
}

void 
gtku_param_widget_load_from_key_file (GtkuParamWidget *pw, 
        GKeyFile *keyfile, const char *group_name)
{
    _keyfile_data_t data = {
        .keyfile = keyfile,
        .group_name = group_name,
        .pw = pw
    };
    g_hash_table_foreach (pw->params, _param_load_from_keyfile, &data);
}

static void
_param_save_to_keyfile (void *key, void *value, void *user_data)
{
    GtkWidget *w = GTK_WIDGET (value);
    _keyfile_data_t *data = user_data;
    const char *param_name = key;
    const char *type = g_object_get_data (G_OBJECT(w), "data-type");
    if (!strcmp (type, "boolean")) {
        int val = gtku_param_widget_get_bool (data->pw, param_name);
        g_key_file_set_boolean (data->keyfile, data->group_name,
                param_name, val);
    } else if (!strcmp (type, "double")) {
        double val = gtku_param_widget_get_double (data->pw, param_name);
        g_key_file_set_double (data->keyfile, data->group_name,
                param_name, val);
    } else if (!strcmp (type, "int")) {
        int val = gtku_param_widget_get_int (data->pw, param_name);
        g_key_file_set_integer (data->keyfile, data->group_name,
                param_name, val);
    } else if (!strcmp (type, "enum")) {
        int val = gtku_param_widget_get_enum (data->pw, param_name);
        g_key_file_set_integer (data->keyfile, data->group_name,
                param_name, val);
    } else {
        // TODO
    }
}

void 
gtku_param_widget_save_to_key_file (GtkuParamWidget *pw,
        GKeyFile *keyfile, const char *group_name)
{
    _keyfile_data_t data = {
        .keyfile = keyfile,
        .group_name = group_name,
        .pw = pw
    };
    g_hash_table_foreach (pw->params, _param_save_to_keyfile, &data);
}
