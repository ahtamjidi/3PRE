LIB_PATH:=../../lib

CFLAGS:=`pkg-config --cflags glib-2.0` -std=gnu99 -g -Wall \
		-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE 

static_lib:=$(LIB_PATH)/libglutil.a
shared_lib:=$(LIB_PATH)/libglutil.so

lib_obj:=glutil.o texture.o splot2d.o console.o

#$(shared_lib) 
targets:=$(static_lib) # $(shared_lib)

all: $(targets)
	
$(static_lib): $(lib_obj)
	@echo "    [$@]"
	ar rc $@ $(lib_obj)

%.o: %.c %.h
	@echo "    [$@]"
	gcc -c -o $@ $< $(CFLAGS) 

%.o: %.c
	@echo "    [$@]"
	gcc -c -o $@ $< $(CFLAGS) 

.PHONY: clean
clean: tidy
	rm -f $(targets)

.PHONY: tidy
tidy:
	rm -f $(lib_obj)
