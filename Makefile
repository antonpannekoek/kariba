CONFIG = make.config
include $(CONFIG)


.PHONY: all lib examples tests install docs clean distclean


all: lib tests examples

lib:
	$(MAKE) -C src

examples: lib
	$(MAKE) -C examples

tests: lib
	$(MAKE) -C tests

install: lib
	$(MAKE) -C src install
	$(MAKE) -C tests install

docs:
	$(MAKE) -C docs


clean:
	$(MAKE) -C src clean
	$(MAKE) -C examples clean
	$(MAKE) -C tests clean

distclean:
	$(MAKE) -C src distclean
	$(MAKE) -C examples distclean
	$(MAKE) -C tests distclean

help:
	@echo Possible targets
	@echo
	@echo "'make lib': build the library"
	@echo "'make examples': build the examples"
	@echo "'make all': build all of the three above. This is the default for just running 'make'"
	@echo
	@echo "'make clean': remove the intermediate (object) files for each target"
	@echo "'make distclean': remove the intermediate and final files for each target"
