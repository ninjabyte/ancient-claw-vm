OS=win
ARCH=64
CC=gcc
CFLAGS=-c -Wall -std=c11 -Ofast -Iinclude
SOURCES=sfml-interface.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=sfml-interface
LDOBJS=
LDFLAGS=
OUTDIR=out

ifeq ($(OS),win)
	LDOBJS+=os/$(OS)$(ARCH)/lib/libcsfml-audio.a os/$(OS)$(ARCH)/lib/libcsfml-graphics.a os/$(OS)$(ARCH)/lib/libcsfml-network.a os/$(OS)$(ARCH)/lib/libcsfml-system.a os/$(OS)$(ARCH)/lib/libcsfml-window.a
endif

ifeq ($(OS),linux)
	LDOBJS+=
endif

all: $(SOURCES) $(OUTDIR) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDOBJS) $(LDFLAGS) -o $(OUTDIR)/$@
	cp os/$(OS)$(ARCH)/bin/* $(OUTDIR)/

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
$(OUTDIR):
	mkdir $(OUTDIR)
	
clean:
	rm *.o
	rm -r $(OUTDIR)
