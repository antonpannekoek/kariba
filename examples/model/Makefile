CONFIG = ../../make.config
include $(CONFIG)

# Path to Kariba root
KARIBA = ../../src

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


SOURCES = bhjet.cpp jetpars.cpp utils.cpp
OBJECTS = $(subst .cpp,.o,$(SOURCES))
TARGET_OBJ = bhwrap.o
TARGET = bhwrap
PYOBJ = pyjetmain.o
PYLIB = pyjetmain.so

# Kariba library
LIBKARIBA = $(KARIBA)/libkariba.a
LIBPATH = $(shell dirname $(realpath $(LIBKARIBA)))
LIBSHARED = -L$(LIBPATH) -Wl,-rpath,$(LIBPATH) -lkariba


.PHONY: all clean distclean


all: $(TARGET) $(PYLIB)

$(TARGET): $(OBJECTS) $(LIBKARIBA) $(TARGET_OBJ)
	$(LD) -o $(TARGET) $(OBJECTS) $(TARGET_OBJ) $(LDFLAGS) $(LIBKARIBA) $(LDLIBS)

$(PYLIB): $(OBJECTS) $(LIKARIBA) $(PYOBJ)
	$(LD) $(LDFLAGS) -shared -o $(PYLIB) $(OBJECTS) $(PYOBJ) $(LIBKARIBA) $(LDLIBS)

shared: $(OBJECTS) $(TARGET_OBJ)
	$(LD) -o $(TARGET) $(OBJECTS) $(TARGET_OBJ) $(LDFLAGS) $(LIBSHARED) $(LDLIBS)

clean:
	$(RM) $(OBJECTS) $(KARIBA_OBJECTS) $(TARGET_OBJ) $(PYOBJ)

distclean: clean
	$(RM) $(TARGET) $(PYLIB)
