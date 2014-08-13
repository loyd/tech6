MAKEFLAGS += --no-builtin-rules --no-builtin-variables

#### Default values
TOOLCHAIN := tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64
CC = $(TOOLCHAIN)/bin/arm-linux-gnueabihf-gcc -std=c99
BUILD := build

EXCLUDE :=

CFLAGS := -pedantic -Werror -Wall -Wextra -Wswitch-default -Wswitch-enum      \
          -Wshadow -Wundef -Wpointer-arith -Wcast-align -Winit-self           \
          -Wstrict-overflow=3 -Wlogical-op -Wwrite-strings -Wnested-externs   \
          -Wbad-function-cast -Wold-style-definition -Wunreachable-code       \
          -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations     \
          -Wno-logical-op-parentheses -Wno-unused-parameter -Wno-float-equal

CFLAGS += -D_GNU_SOURCE
CFLAGS += -iquote./embed -I./vendor/include

LFLAGS :=  -L./vendor/lib -lm -luv -liniparser

RHOST :=
RPATH :=

PYTHON2.7 := python2.7
CLINTFLAGS := --verbose=0 --extensions=c,h --root=embed --filter="+,          \
              -legal,-build/header_guard,-readability/casting"

-include config.mk


#### Vars
OBJDIR := $(BUILD)/objs
EXCLUDE := $(addprefix embed/,$(EXCLUDE))
SOURCES := $(filter-out $(EXCLUDE),$(shell find embed -name '*.c'))
HEADERS := $(filter-out $(EXCLUDE),$(shell find embed -name '*.h'))
OBJECTS := $(patsubst embed/%.c,$(OBJDIR)/%.o,$(SOURCES))


#### Targets
$(BUILD)/embed: $(OBJECTS)
	$(CC) $^ $(LFLAGS) -o $@

$(BUILD):
	mkdir -p $(sort $(dir $(OBJECTS)))

$(OBJDIR)/%.o: embed/%.c | $(BUILD)
	$(CC) -c $(CFLAGS) $< -o $@
	$(CC) -MM $(CFLAGS) $< > $(@:.o=.d.tmp)
	@sed -e 's|.*:|$@:|' < $(@:.o=.d.tmp) > $(@:.o=.d)
	@sed -e 's/.*://' -e 's/\\$$//' < $(@:.o=.d.tmp) | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $(@:.o=.d)
	@rm -f $(@:.o=.d.tmp)

tools:
	git clone git://github.com/raspberrypi/tools.git --depth=1
	curl http://google-styleguide.googlecode.com/svn/trunk/cpplint/cpplint.py -O
	mv cpplint.py $@

vendor:
	mkdir -p $@/lib $@/include
	scp $(RHOST):/usr/lib/libuv*.so* vendor/lib
	scp $(RHOST):/usr/include/uv*.h vendor/include
	scp $(RHOST):/usr/lib/libiniparser*.so* vendor/lib
	scp $(RHOST):/usr/include/iniparser.h vendor/include


#### Tasks
.PHONY: deploy remrun lint clean

deploy: $(BUILD)/embed
	scp $< $(RHOST):$(RPATH)

remrun: deploy
	ssh -t $(RHOST) 'cd $(RPATH) && ./embed'

lint:
	$(PYTHON2.7) tools/cpplint.py $(CLINTFLAGS) $(SOURCES) $(HEADERS)

clean:
	rm -rf $(BUILD)

-include $(OBJECTS:.o=.d)
