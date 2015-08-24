CC=gcc
CFLAGS=-c -Wall -std=c11 -Ofast
LDFLAGS=
SOURCES=vm.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=vm

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o $(EXECUTABLE)
