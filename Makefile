CC:=g++
CFLAGS:=-Wall -O2 -fPIC -shared -Wno-deprecated -D__STDC_LIMIT_MACROS -std=gnu++0x -fno-strict-aliasing -msse4
#CFLAGS:=-Wall -g -fPIC -shared -Wno-deprecated -D__STDC_LIMIT_MACROS -std=gnu++0x -fno-strict-aliasing -msse4 
DIRS=.
SRC:=$(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
OBJ:=$(SRC:.cpp=.o)
INCLUDE:=
LIBPATH:=
#if you want to store stream use below
LIB+=pthread rt 
LIB+=
LIBA:=
SUBDIR=
JOBS:=$(shell grep process /proc/cpuinfo  | wc -l)

#define make_subdir
#	@for dir in $(SUBDIR); do \
#	make -C $$dir -j$(JOBS) $1; \
#	done;
#endef

TARGET:=libydx.so

all:
	make -j$(JOBS) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -fPIC -shared -o $@ $^ $(addprefix /usr/local/lib/, $(LIBA)) $(addprefix -L,$(LIBPATH)) $(addprefix -l,$(LIB))
#	$(call make_subdir)

%.o: %.cpp
	$(CC) $(CFLAGS) $(addprefix -I,$(INCLUDE)) -c $< -o $@

clean:
	$(RM) $(TARGET) $(OBJ) $(DEP)
	#$(call make_subdir, clean)

install:
#	cp dpi /usr/local/dpi/
#	cp plugin/*.so /usr/local/dpi/plugin/
