INSTALL_ROOT := /usr/local
UOWNER := root
GOWNER := wheel
MAKE=make
SUBDIRS := BlinkenServer BlinkenClient BlinkenTestClient

.PHONY :	all $(SUBDIRS)

all :	$(SUBDIRS)

$(SUBDIRS) :
	$(MAKE) -C $@ 

clean:
	$(MAKE) -C BlinkenServer clean
	$(MAKE) -C BlinkenClient clean
	$(MAKE) -C BlinkenTestClient clean	

install:
	$(MAKE) -C BlinkenServer install UOWER=$(UOWNER) GOWNER=$(GOWNER)
	$(MAKE) -C BlinkenClient install
	$(MAKE) -C BlinkenTestClient install

.PHONY:	BlinkenServer BlinkenClient BlinkenTestClient


