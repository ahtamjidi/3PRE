lcmtypes:=$(shell ls ../lcmtypes/*.lcm)
basenames:=$(basename $(notdir $(lcmtypes)))
java_lcmtypes:=$(basenames:%=src/lcmtypes/%.java)

CLASSPATH_LCM:=`pkg-config --variable=classpath lcm-java`

CLASSPATH:=$(shell pwd)/src:$(CLASSPATH_LCM)

all: $(java_lcmtypes) 
	CLASSPATH=$(CLASSPATH) ant

src/lcmtypes/%.java: ../lcmtypes/%.lcm
	lcm-gen $^ -j --jpath=src --lazy

clean:
	rm -f $(java_lcmtypes) 
	ant clean
