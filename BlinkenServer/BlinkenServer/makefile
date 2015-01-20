PROGRAM=BlinkenServer
CC=cc
CFLAGS=
LDFLAGS=
INSTALL_ROOT := /usr/local
UOWNER := root
GOWNER := wheel

ifdef	HAS_BLINKEN
	CFLAGS := -DHAS_BLINKEN=$(HAS_BLINKEN)
	LDFLAGS := -lwiringPi
endif


C_FILES := $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(C_FILES))

ALL :	depend $(PROGRAM)

$(PROGRAM):	.depend $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(PROGRAM)

depend: .depend

.depend:	cmd = gcc -MM -MF depend $(var); cat depend >> .depend;
.depend:	
	@echo "Generating dependencies..."
	@$(foreach var, $(C_FILES), $(cmd))
	@rm -f depend

-include .depend

clean:	
	rm -f .depend *.o

install: $(PROGRAM)
	install -d -g $(GOWNER) -o $(UOWNER) -v $(INSTALL_ROOT)/bin
	install -b -C -v -o $(UOWNER) $(PROGRAM) $(INSTALL_ROOT)/bin

.PHONY:	clean depend install

