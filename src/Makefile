CONFIG = ../make.config
include $(CONFIG)


ifndef GSL
	GSL_CPPFLAGS=
	GSL_LDFLAGS=
else
	GSL_CPPFLAGS=-I$(GSL)/include
	GSL_LDFLAGS=-L$(GSL)/lib
endif


# Detect OS: Darwin or Linux
UNAME := $(shell uname -s)

CXXFLAGS += $(OPENMP)
CPPFLAGS += $(GSL_CPPFLAGS)
LD = $(CXX)
LDFLAGS += $(OPENMP) $(GSL_LDFLAGS) $(EXTRA_LDFLAGS)
LDLIBS += -lgsl -lgslcblas -lm
AR = ar
AR_OPTS = rcs
RM = rm -f



SOURCES = BBody.cpp Bknpower.cpp Compton.cpp Cyclosyn.cpp EBL.cpp Electrons.cpp GammaRays.cpp Kappa.cpp Mixed.cpp Neutrinos_pg.cpp Neutrinos_pp.cpp Particles.cpp Powerlaw.cpp Radiation.cpp ShSDisk.cpp Thermal.cpp
OBJECTS = $(subst .cpp,.o,$(SOURCES))
LIBSTATIC = libkariba.a

# Platform specific stuff for building the shared library
ifeq ($(UNAME), Darwin)  # macOS
    DYN_EXT = dylib
    DYLIB_INSTALL_NAME = @rpath/libkariba.dylib
else ifeq ($(UNAME), Linux)
    DYN_EXT = so
endif
LIBSHARED = libkariba.$(DYN_EXT)



.PHONY: all clean distclean


all: shared static

# macOS compatible way to create a static library
static: $(OBJECTS)
	$(AR) $(AR_OPTS) $(LIBSTATIC) $(OBJECTS)

shared: $(OBJECTS)
ifeq ($(UNAME), Darwin)
	$(LD) -dynamiclib -o $(LIBSHARED) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -install_name $(DYLIB_INSTALL_NAME)
else ifeq ($(UNAME), Linux)
	$(LD) -shared -fPIC -o $(LIBSHARED) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -Wl,-soname,$(LIBSHARED)
endif

install: shared
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(LIBSHARED) $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/kariba/
	install -m 644 kariba/*.hpp $(DESTDIR)$(PREFIX)/include/kariba/

clean:
	$(RM) $(OBJECTS)

distclean: clean
	$(RM) $(LIBSHARED) $(LIBSTATIC)
