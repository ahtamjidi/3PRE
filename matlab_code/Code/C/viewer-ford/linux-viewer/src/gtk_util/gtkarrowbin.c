#include <stdio.h>
#include "gtkarrowbin.h"

#define ARROW_LEN_TOP   7
#define ARROW_LEN_BOTTOM 11

enum {
  PROP_0,

  PROP_TOP_PADDING,
  PROP_BOTTOM_PADDING,
  PROP_LEFT_PADDING,
  PROP_RIGHT_PADDING,
  
  PROP_LAST
};

#define GTK_PARAM_READWRITE G_PARAM_READWRITE|G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB

#define GTK_ARROW_BIN_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GTK_TYPE_ARROW_BIN, GtkArrowBinPrivate))

struct _GtkArrowBinPrivate
{
  guint padding_top;
  guint padding_bottom;
  guint padding_left;
  guint padding_right;
};

static void gtk_arrow_bin_size_request  (GtkWidget         *widget,
					 GtkRequisition    *requisition);
static void gtk_arrow_bin_size_allocate (GtkWidget         *widget,
					 GtkAllocation     *allocation);
static void gtk_arrow_bin_set_property (GObject         *object,
                                        guint            prop_id,
                                        const GValue    *value,
                                        GParamSpec      *pspec);
static void gtk_arrow_bin_get_property (GObject         *object,
                                        guint            prop_id,
                                        GValue          *value,
                                        GParamSpec      *pspec);
static gint gtk_arrow_bin_expose    (GtkWidget * widget,
        GdkEventExpose * event);

G_DEFINE_TYPE (GtkArrowBin, gtk_arrow_bin, GTK_TYPE_BIN)

static void
gtk_arrow_bin_class_init (GtkArrowBinClass *class)
{
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;

  gobject_class = (GObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;
  
  gobject_class->set_property = gtk_arrow_bin_set_property;
  gobject_class->get_property = gtk_arrow_bin_get_property;

  widget_class->size_request = gtk_arrow_bin_size_request;
  widget_class->size_allocate = gtk_arrow_bin_size_allocate;
  widget_class->expose_event = gtk_arrow_bin_expose;


/**
 * GtkArrowBin:top-padding:
 *
 * The padding to insert at the top of the widget.
 */
  g_object_class_install_property (gobject_class,
                                   PROP_TOP_PADDING,
                                   g_param_spec_uint("top-padding",
                                                      "Top Padding",
                                                      "The padding to insert at the top of the widget.",
                                                      0,
                                                      G_MAXINT,
                                                      ARROW_LEN_TOP,
                                                      GTK_PARAM_READWRITE));

/**
 * GtkArrowBin:bottom-padding:
 *
 * The padding to insert at the bottom of the widget.
 */
  g_object_class_install_property (gobject_class,
                                   PROP_BOTTOM_PADDING,
                                   g_param_spec_uint("bottom-padding",
                                                      "Bottom Padding",
                                                      "The padding to insert at the bottom of the widget.",
                                                      0,
                                                      G_MAXINT,
                                                      ARROW_LEN_BOTTOM,
                                                      GTK_PARAM_READWRITE));

/**
 * GtkArrowBin:left-padding:
 *
 * The padding to insert at the left of the widget.
 */
  g_object_class_install_property (gobject_class,
                                   PROP_LEFT_PADDING,
                                   g_param_spec_uint("left-padding",
                                                      "Left Padding",
                                                      "The padding to insert at the left of the widget.",
                                                      0,
                                                      G_MAXINT,
                                                      0,
                                                      GTK_PARAM_READWRITE));

/**
 * GtkArrowBin:right-padding:
 *
 * The padding to insert at the right of the widget.
 */
  g_object_class_install_property (gobject_class,
                                   PROP_RIGHT_PADDING,
                                   g_param_spec_uint("right-padding",
                                                      "Right Padding",
                                                      "The padding to insert at the right of the widget.",
                                                      0,
                                                      G_MAXINT,
                                                      0,
                                                      GTK_PARAM_READWRITE));

  g_type_class_add_private (gobject_class, sizeof (GtkArrowBinPrivate));  
}

static void
gtk_arrow_bin_init (GtkArrowBin *arrow_bin)
{
  GtkArrowBinPrivate *priv;
  
  GTK_WIDGET_SET_FLAGS (arrow_bin, GTK_NO_WINDOW);
  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (arrow_bin), FALSE);

  /* Initialize padding with default values: */
  priv = GTK_ARROW_BIN_GET_PRIVATE (arrow_bin);
  priv->padding_top = ARROW_LEN_TOP;
  priv->padding_bottom = ARROW_LEN_BOTTOM;
  priv->padding_left = 0;
  priv->padding_right = 0;
}

GtkWidget*
gtk_arrow_bin_new ()
{
  GtkArrowBin *arrow_bin;

  arrow_bin = g_object_new (GTK_TYPE_ARROW_BIN, NULL);

  return GTK_WIDGET (arrow_bin);
}

static void
gtk_arrow_bin_set_property (GObject         *object,
			    guint            prop_id,
			    const GValue    *value,
			    GParamSpec      *pspec)
{
  GtkArrowBin *arrow_bin;
  GtkArrowBinPrivate *priv;
  
  arrow_bin = GTK_ARROW_BIN (object);
  priv = GTK_ARROW_BIN_GET_PRIVATE (arrow_bin);
  
  switch (prop_id)
    {
    /* Padding: */
    case PROP_TOP_PADDING:
      gtk_arrow_bin_set_padding (arrow_bin,
			 g_value_get_uint (value),
			 priv->padding_bottom,
			 priv->padding_left,
			 priv->padding_right);
      break;
    case PROP_BOTTOM_PADDING:
      gtk_arrow_bin_set_padding (arrow_bin,
			 priv->padding_top,
			 g_value_get_uint (value),
			 priv->padding_left,
			 priv->padding_right);
      break;
    case PROP_LEFT_PADDING:
      gtk_arrow_bin_set_padding (arrow_bin,
			 priv->padding_top,
			 priv->padding_bottom,
			 g_value_get_uint (value),
			 priv->padding_right);
      break;
    case PROP_RIGHT_PADDING:
      gtk_arrow_bin_set_padding (arrow_bin,
			 priv->padding_top,
			 priv->padding_bottom,
			 priv->padding_left,
			 g_value_get_uint (value));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gtk_arrow_bin_get_property (GObject         *object,
			    guint            prop_id,
			    GValue          *value,
			    GParamSpec      *pspec)
{
  GtkArrowBin *arrow_bin;
  GtkArrowBinPrivate *priv;

  arrow_bin = GTK_ARROW_BIN (object);
  priv = GTK_ARROW_BIN_GET_PRIVATE (arrow_bin);
   
  switch (prop_id)
    {
    /* Padding: */
    case PROP_TOP_PADDING:
      g_value_set_uint (value, priv->padding_top);
      break;
    case PROP_BOTTOM_PADDING:
      g_value_set_uint (value, priv->padding_bottom);
      break;
    case PROP_LEFT_PADDING:
      g_value_set_uint (value, priv->padding_left);
      break;
    case PROP_RIGHT_PADDING:
      g_value_set_uint (value, priv->padding_right);
      break;
      
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


static void
gtk_arrow_bin_size_request (GtkWidget      *widget,
			    GtkRequisition *requisition)
{
    GtkBin *bin;
    GtkArrowBinPrivate *priv;

    bin = GTK_BIN (widget);
    priv = GTK_ARROW_BIN_GET_PRIVATE (widget);

    requisition->width = GTK_CONTAINER (widget)->border_width * 2;
    requisition->height = GTK_CONTAINER (widget)->border_width * 2;

    /* Request extra space for the padding: */
    requisition->width += (priv->padding_left + priv->padding_right);
    requisition->height += (priv->padding_top + priv->padding_bottom);

    if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
        GtkRequisition child_requisition;

        gtk_widget_size_request (bin->child, &child_requisition);

        requisition->width += child_requisition.width;
        requisition->height += child_requisition.height;
    }
}

static void
gtk_arrow_bin_size_allocate (GtkWidget     *widget,
			     GtkAllocation *allocation)
{
  GtkArrowBin *arrow_bin;
  GtkBin *bin;
  GtkAllocation child_allocation;
  GtkRequisition child_requisition;
  gint width, height;
  gint border_width;
  gint padding_horizontal, padding_vertical;
  GtkArrowBinPrivate *priv;

  padding_horizontal = 0;
  padding_vertical = 0;

  widget->allocation = *allocation;
  arrow_bin = GTK_ARROW_BIN (widget);
  bin = GTK_BIN (widget);
  
  if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
  {
      gtk_widget_get_child_requisition (bin->child, &child_requisition);

      border_width = GTK_CONTAINER (arrow_bin)->border_width;

      priv = GTK_ARROW_BIN_GET_PRIVATE (widget);
      padding_horizontal = priv->padding_left + priv->padding_right;
      padding_vertical = priv->padding_top + priv->padding_bottom;

      width = allocation->width - padding_horizontal - 2 * border_width;
      height = allocation->height - padding_vertical - 2 * border_width;

      child_allocation.width = MAX (1, width);
      child_allocation.height = MAX (1, height);

      child_allocation.x = allocation->x + border_width + priv->padding_left;
      child_allocation.y = allocation->y + border_width + priv->padding_top;

      gtk_widget_size_allocate (bin->child, &child_allocation);
  }
}

/**
 * gtk_arrow_bin_set_padding:
 * @arrow_bin: a #GtkArrowBin
 * @padding_top: the padding at the top of the widget
 * @padding_bottom: the padding at the bottom of the widget
 * @padding_left: the padding at the left of the widget
 * @padding_right: the padding at the right of the widget.
 *
 * Sets the padding on the different sides of the widget.
 * The padding adds blank space to the sides of the widget. For instance,
 * this can be used to indent the child widget towards the right by adding
 * padding on the left.
 */
void
gtk_arrow_bin_set_padding (GtkArrowBin    *arrow_bin,
			   guint            padding_top,
			   guint            padding_bottom,
			   guint            padding_left,
			   guint            padding_right)
{
  GtkArrowBinPrivate *priv;
  
  g_return_if_fail (GTK_IS_ARROW_BIN (arrow_bin));

  priv = GTK_ARROW_BIN_GET_PRIVATE (arrow_bin);

  g_object_freeze_notify (G_OBJECT (arrow_bin));

  if (priv->padding_top != padding_top)
    {
      priv->padding_top = padding_top;
      g_object_notify (G_OBJECT (arrow_bin), "top-padding");
    }
  if (priv->padding_bottom != padding_bottom)
    {
      priv->padding_bottom = padding_bottom;
      g_object_notify (G_OBJECT (arrow_bin), "bottom-padding");
    }
  if (priv->padding_left != padding_left)
    {
      priv->padding_left = padding_left;
      g_object_notify (G_OBJECT (arrow_bin), "left-padding");
    }
  if (priv->padding_right != padding_right)
    {
      priv->padding_right = padding_right;
      g_object_notify (G_OBJECT (arrow_bin), "right-padding");
    }

  g_object_thaw_notify (G_OBJECT (arrow_bin));
  
  /* Make sure that the widget and children are redrawn with the new setting: */
  if (GTK_BIN (arrow_bin)->child)
    gtk_widget_queue_resize (GTK_BIN (arrow_bin)->child);

  gtk_widget_queue_draw (GTK_WIDGET (arrow_bin));
}

/**
 * gtk_arrow_bin_get_padding:
 * @arrow_bin: a #GtkArrowBin
 * @padding_top: location to store the padding for the top of the widget, or %NULL
 * @padding_bottom: location to store the padding for the bottom of the widget, or %NULL
 * @padding_left: location to store the padding for the left of the widget, or %NULL
 * @padding_right: location to store the padding for the right of the widget, or %NULL
 *
 * Gets the padding on the different sides of the widget.
 * See gtk_arrow_bin_set_padding ().
 *
 * Since: 2.4
 */
void
gtk_arrow_bin_get_padding (GtkArrowBin    *arrow_bin,
			   guint           *padding_top,
			   guint           *padding_bottom,
			   guint           *padding_left,
			   guint           *padding_right)
{
  GtkArrowBinPrivate *priv;
 
  g_return_if_fail (GTK_IS_ARROW_BIN (arrow_bin));

  priv = GTK_ARROW_BIN_GET_PRIVATE (arrow_bin);
  if(padding_top)
    *padding_top = priv->padding_top;
  if(padding_bottom)
    *padding_bottom = priv->padding_bottom;
  if(padding_left)
    *padding_left = priv->padding_left;
  if(padding_right)
    *padding_right = priv->padding_right;
}

static gboolean
gtk_arrow_bin_expose (GtkWidget * widget, GdkEventExpose * event)
{
    if (GTK_WIDGET_DRAWABLE (widget)) {
        GtkContainer * cont = GTK_CONTAINER (widget);
        int x = widget->allocation.x + widget->allocation.width / 2;
        int y = widget->allocation.y + cont->border_width;
        int yb = widget->allocation.y + widget->allocation.height -
            cont->border_width;
        GdkPoint points[] = {
            { x+0, y+0 },
            { x+6, y+0 },
            { x+6, yb-6 },
            { x+9, yb-6 },
            { x+3, yb },
            { x-3, yb-6 },
            { x+0, yb-6 },
        };
        gdk_draw_polygon (widget->window, widget->style->black_gc,
                TRUE, points, sizeof (points) / sizeof (GdkPoint));
        GTK_WIDGET_CLASS (gtk_arrow_bin_parent_class)->expose_event (widget,
                event);
    }
    return FALSE;
}
