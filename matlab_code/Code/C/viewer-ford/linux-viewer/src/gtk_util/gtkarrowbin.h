#ifndef __GTK_ARROW_BIN_H__
#define __GTK_ARROW_BIN_H__


#include <gdk/gdk.h>
#include <gtk/gtkbin.h>


G_BEGIN_DECLS

#define GTK_TYPE_ARROW_BIN                  (gtk_arrow_bin_get_type ())
#define GTK_ARROW_BIN(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_ARROW_BIN, GtkArrowBin))
#define GTK_ARROW_BIN_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_ARROW_BIN, GtkArrowBinClass))
#define GTK_IS_ARROW_BIN(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_ARROW_BIN))
#define GTK_IS_ARROW_BIN_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_ARROW_BIN))
#define GTK_ARROW_BIN_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_ARROW_BIN, GtkArrowBinClass))


typedef struct _GtkArrowBin       GtkArrowBin;
typedef struct _GtkArrowBinClass  GtkArrowBinClass;
typedef struct _GtkArrowBinPrivate GtkArrowBinPrivate;

struct _GtkArrowBin
{
  GtkBin bin;
};

struct _GtkArrowBinClass
{
  GtkBinClass parent_class;
};


GType      gtk_arrow_bin_get_type   (void) G_GNUC_CONST;
GtkWidget* gtk_arrow_bin_new        ();
void       gtk_arrow_bin_set_padding (GtkArrowBin      *arrow_bin,
				      guint              padding_top,
				      guint              padding_bottom,
				      guint              padding_left,
				      guint              padding_right);

void       gtk_arrow_bin_get_padding (GtkArrowBin      *arrow_bin,
				      guint             *padding_top,
				      guint             *padding_bottom,
				      guint             *padding_left,
				      guint             *padding_right);

G_END_DECLS


#endif /* __GTK_ARROW_BIN_H__ */
