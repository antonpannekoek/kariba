CONFIG = ../make.config
include $(CONFIG)

# Path to Kariba root
KARIBA = ../src


ifndef GSL
	GSL_CPPFLAGS=
	GSL_LDFLAGS=
else
	GSL_CPPFLAGS=-I$(GSL)/include
	GSL_LDFLAGS=-L$(GSL)/lib
endif


CXXFLAGS += $(OPENMP)
CPPFLAGS += $(GSL_CPPFLAGS) -I${KARIBA}
LD = $(CXX)
LDFLAGS += $(OPENMP) $(GSL_LDFLAGS) $(EXTRA_LDFLAGS)
LDLIBS += -lgsl -lgslcblas -lm
RM = rm -f


SOURCES = utils.cpp
OBJECTS = $(subst .cpp,.o,$(SOURCES))

CORONA = corona
PARTICLES = particles
SINGLEZONE = singlezone
OBJ_CORONA = corona.o
OBJ_PARTICLES = particles.o
OBJ_SINGLEZONE = singlezone.o

# Kariba library
LIBKARIBA = $(KARIBA)/libkariba.a
LIBPATH = $(shell dirname $(realpath $(LIBKARIBA)))
LIBSHARED = -L$(LIBPATH) -Wl,-rpath,$(LIBPATH) -lkariba


.PHONY: all clean distclean


all: corona particles singlezone

shared: $(OBJECTS) $(OBJ_CORONA) $(OBJ_PARTICLES) $(OBJ_SINGLEZONE)
	$(LD) -o corona $(OBJECTS) $(OBJ_CORONA) $(LDFLAGS) $(LIBSHARED) $(LDLIBS)
	$(LD) -o particles $(OBJECTS) $(OBJ_PARTICLES) $(LDFLAGS) $(LIBSHARED) $(LDLIBS)
	$(LD) -o singlezone $(OBJECTS) $(OBJ_SINGLEZONE) $(LDFLAGS) $(LIBSHARED) $(LDLIBS)

corona: $(OBJECTS) $(LIBKARIBA) $(OBJ_CORONA)
	$(LD) $(LDFLAGS) -o corona $(OBJECTS) $(OBJ_CORONA) $(LIBKARIBA) $(LDLIBS)

particles: $(OBJECTS) $(LIBKARIBA) $(OBJ_PARTICLES)
	$(LD) $(LDFLAGS) -o particles $(OBJECTS) $(OBJ_PARTICLES) $(LIBKARIBA) $(LDLIBS)

singlezone: $(OBJECTS) $(LIBKARIBA) $(OBJ_SINGLEZONE)
	$(LD) $(LDFLAGS) -o singlezone $(OBJECTS) $(OBJ_SINGLEZONE) $(LIBKARIBA) $(LDLIBS)


clean:
	$(RM) $(OBJECTS) $(OBJ_CORONA) $(OBJ_PARTICLES) $(OBJ_SINGLEZONE)

distclean: clean
	$(RM) corona particles singlezone
