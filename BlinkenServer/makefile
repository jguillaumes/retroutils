CC=cc
CCFLAGS=-DHAS_BLINKEN
LIBS=-lwiringPi

all :	BlinkenServer

clean :	
	@rm BlinkenServer BlinkenClient


BlinkenServer :	auxfunc.c  blinkenserver.c  blinkenserver.h logger.c  main.c
	$(CC) $(CCFLAGS) -o BlinkenServer auxfunc.c blinkenserver.c logger.c  main.c $(LIBS)

BlinkenClient :	blinkenclient.c blinkenserver.h auxfunc.c
	$(CC) $(CCFLAGS) -o BlinkenClient blinkenclient.c auxfunc.c