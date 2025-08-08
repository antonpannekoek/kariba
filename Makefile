CONFIG = make.config
include $(CONFIG)


.PHONY: all lib examples model tests install docs clean distclean


all: lib tests examples model

lib:
	$(MAKE) -C src

examples: lib
	$(MAKE) -C examples

model: lib
	$(MAKE) -C examples/model

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
	$(MAKE) -C examples/model clean
	$(MAKE) -C tests clean

distclean:
	$(MAKE) -C src distclean
	$(MAKE) -C examples distclean
	$(MAKE) -C examples/model distclean
	$(MAKE) -C tests distclean

help:
	@echo Possible targets
	@echo
	@echo "'make lib': build the library"
	@echo "'make examples': build the examples"
	@echo "'make model': build the BHJet example model"
	@echo "'make all': build all of the three above. This is the default for just running 'make'"
	@echo
	@echo "'make clean': remove the intermediate (object) files for each target"
	@echo "'make distclean': remove the intermediate and final files for each target"
