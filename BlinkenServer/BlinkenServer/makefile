PROGRAM=BlinkenServer
CC=cc
CFLAGS=
LDFLAGS=

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
	install -d -g wheel -o root -v $(INSTALL_ROOT)/bin
	install -b -C -v -o root $(PROGRAM) $(INSTALL_ROOT)/bin

.PHONY:	clean depend install

