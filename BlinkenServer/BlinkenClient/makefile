INSTALL_ROOT=/usr/local
CC=cc
CFLAGS=
LDFLAGS=

C_FILES := $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(C_FILES))
HEADERS := $(wildcard *.h)

ALL :	depend libblinken.a

depend: .depend
 
.depend:	cmd = gcc -MM -MF depend $(var); cat depend >> .depend;
.depend:	
	@echo "Generating dependencies..."
	@$(foreach var, $(C_FILES), $(cmd))
	@rm -f depend
 
-include .depend

libblinken.a :		$(OBJS)
	ar -r $@ $?

clean:	
	rm -f .depend *.o
 
install: libblinken.a $(HEADERS)
	install -d -g root -o root -v $(INSTALL_ROOT)/include
	install -d -g root -o root -v $(INSTALL_ROOT)/lib
	install -b -C -v -o root libblinken.a $(INSTALL_ROOT)/lib
	install -b -C -v -o root $(HEADERS) $(INSTALL_ROOT)/include

.PHONY:	clean depend install

