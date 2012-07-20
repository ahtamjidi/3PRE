include ../common.mk

CFLAGS = $(CFLAGS_STD) $(CFLAGS_COMMON) $(CFLAGS_GTK) $(CFLAGS_GL) $(CFLAGS_JPEG) -fPIC
LDFLAGS = $(LDFLAGS_STD) $(LDFLAGS_COMMON) $(LDFLAGS_GTK) $(LDFLAGS_GL) $(LDFLAGS_JPEG)

LIBGTKUTIL_OBJS = gtk_util.o param_widget.o rgb_canvas.o \
	glx_context.o gtkgldrawingarea.o user_control.o gtkarrowbin.o \
	fbgldrawingarea.o gl_image_area.o

LIBGTKUTIL = $(LIB_PATH)/libgtk_util.a

param_widget_tester:=param_widget_tester
rgb_canvas_tester:=rgb_canvas_tester
gtkgl_tester:=gtkgl_tester
gl_image_area_tester:=gl_image_area_tester

param_widget_tester_obj:=param_widget_tester.o
rgb_canvas_tester_obj:=rgb_canvas_tester.o
gtkgl_tester_obj:=gtkgl_tester.o
gl_image_area_tester_obj:=gl_image_area_tester.o

LDFLAGS_TESTER:=$(LDFLAGS_GTK) $(LIBGTKUTIL)

all: $(LIBGTKUTIL) $(param_widget_tester) $(rgb_canvas_tester) \
	$(gtkgl_tester) $(gl_image_area_tester)


$(LIBGTKUTIL): $(LIBGTKUTIL_OBJS)
	ar rc $@ $(LIBGTKUTIL_OBJS)

param_widget_tester: $(param_widget_tester_obj) $(LIBGTKUTIL)
	@echo "    [$@]"
	gcc -o $@ $(param_widget_tester_obj) $(LDFLAGS_TESTER)

rgb_canvas_tester: $(rgb_canvas_tester_obj) $(LIBGTKUTIL)
	@echo "    [$@]"
	gcc -o $@ $(rgb_canvas_tester_obj) $(LDFLAGS_TESTER)

ezavi_button_tester: $(ezavi_button_tester_obj) $(LIBGTKUTIL)
	@echo "    [$@]"
	gcc -o $@ $(ezavi_button_tester_obj) $(LDFLAGS_TESTER)

gtkgl_tester: $(gtkgl_tester_obj) $(LIBGTKUTIL)
	@echo "    [$@]"
	gcc -o $@ $(gtkgl_tester_obj) $(LDFLAGS_TESTER) $(LDFLAGS_GL)

gl_image_area_tester: $(gl_image_area_tester_obj) $(LIBGTKUTIL)
	@echo "    [$@]"
	gcc -o $@ $(gl_image_area_tester_obj) $(LDFLAGS_TESTER) $(LDFLAGS_GL)

clean:
	rm -f *.o *.a *~ $(param_widget_tester) $(rgb_canvas_tester) \
		$(gtkgl_tester) $(gl_image_area_tester) $(LIBGTKUTIL)

