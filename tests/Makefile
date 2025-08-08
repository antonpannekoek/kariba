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


# Detect OS: Darwin or Linux
UNAME := $(shell uname -s)


CXXFLAGS += $(OPENMP)
CPPFLAGS += $(GSL_CPPFLAGS) -I${KARIBA} -I.
LD = $(CXX)
LDFLAGS += $(GSL_LDFLAGS) $(OPENMP) $(EXTRA_LDFLAGS)
LDLIBS += -lgsl -lgslcblas -lm
RM = rm -f


# Kariba library
LIBKARIBA = $(KARIBA)/libkariba.a
LIBPATH = $(shell dirname $(realpath $(LIBKARIBA)))
#LIBSHARED = -L$(LIBPATH) -Wl,-rpath,$(LIBPATH) -lkariba
LIBSHARED = -L$(DESTDIR)$(PREFIX)/lib -lkariba

SOURCES = test_bknpower.cpp test_compton.cpp test_cyclosyn.cpp test_distributions.cpp test_ebl.cpp test_particles.cpp test_powerlaw.cpp test_radiation.cpp
OBJECTS = $(subst .cpp,.o,$(SOURCES))
MAIN_OBJ = test_main.cpp
TEST_MAIN = kariba_test


.PHONY: all tests main clean distclean

all: main

main: $(OBJECTS) $(MAIN_OBJ)
	$(LD) -o $(TEST_MAIN) $(OBJECTS) $(MAIN_OBJ) $(LDFLAGS) $(LIBKARIBA) $(LDLIBS)

# Unit tests that use the shared library version
shared: $(OBJECTS) $(MAIN_OBJ)
	$(LD) -o $(TEST_MAIN) $(OBJECTS) $(MAIN_OBJ) $(LDFLAGS) $(LIBSHARED) $(LDLIBS)

install: shared
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 $(TEST_MAIN) $(DESTDIR)$(PREFIX)/bin/

clean:
	$(RM) $(OBJECTS)

distclean: clean
	$(RM) $(TEST_MAIN)
