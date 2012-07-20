include ../common.mk

CFLAGS = $(CFLAGS_STD) $(CFLAGS_COMMON) $(CFLAGS_GTK) $(CFLAGS_GL) $(CFLAGS_JPEG) -fPIC
LDFLAGS = $(LDFLAGS_STD) $(LDFLAGS_COMMON) $(LDFLAGS_GTK) $(LDFLAGS_GL) $(LDFLAGS_JPEG) 

LIBVIEWER_OBJS = viewer.o default_view_handler.o rwx.o tokenize.o
LIBVIEWER = $(LIB_PATH)/libviewer.a

all: $(LIBVIEWER)


$(LIBVIEWER): $(LIBVIEWER_OBJS)
	ar rc $@ $(LIBVIEWER_OBJS)

#%.o: %.c
#	gcc -c -o $@ $< $(CFLAGS) 

clean: 
	rm -f *.a *.o *~

