INSTALL_ROOT=/usr/local

ALL:	BlinkenServer BlinkenClient BlinkenTestClient

clean:	
	$(MAKE) -C BlinkenServer clean
	$(MAKE) -C BlinkenClient clean
	$(MAKE) -C BlinkenTestClient clean	

install:
	$(MAKE) -C BlinkenServer install
	$(MAKE) -C BlinkenClient install
	$(MAKE) -C BlinkenTestClient install

BlinkenServer:
	$(MAKE) -C $@

BlinkenClient:
	$(MAKE) -C $@

BlinkenTestClient:
	$(MAKE) -C $@


.PHONY:	BlinkenServer BlinkenClient BlinkenTestClient


