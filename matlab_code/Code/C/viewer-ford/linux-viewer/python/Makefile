pythonpath:=$(shell pwd)
scripts:=procman_sheriff 
script_targets:=$(scripts:%=../bin/%)

lcmtypes:=$(shell ls ../lcmtypes/*.lcm)
basenames:=$(basename $(notdir $(lcmtypes)))
python_lcmtypes:=$(basenames:%=lcmtypes/%.py)

CLASSPATH:=$(CLASSPATH):src

all: $(python_lcmtypes) $(script_targets)

lcmtypes/%.py: ../lcmtypes/%.lcm
	lcm-gen $^ -p

../bin/%: scripts/%
	@echo "    [$@]"
	@./make-bin-script $@ $(pythonpath) $(pythonpath)/$^

clean:
	rm -f $(script_targets)
	rm -f $(python_lcmtypes)
