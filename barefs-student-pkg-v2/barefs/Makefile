PROGRAMS = barefs 

COMPILE = $(CC) $(DEFS) $(CFLAGS)
CC = gcc
CFLAGS = -g  -O0 -Wall 
CPFLAGS = $(shell pkg-config fuse --cflags)
LDFLAGS = $(shell pkg-config fuse --libs)
DEFS = -DHAVE_SETXATTR
OBJECTS = fs.o block.o barefs.o 

barefs: $(OBJECTS)
	gcc $(OBJECTS) -o barefs $(LDFLAGS)

barefs.o: barefs.c fs.h block.h
	gcc $(CFLAGS) $(DEFS) -g -c barefs.c $(CPFLAGS) 
	
fs.o: fs.h fs.c
	$(COMPILE) -std=c99 -c fs.c $(CPFLAGS) 
	
block.o: block.h block.c 
	$(COMPILE) -c block.c $(CPFLAGS) 
	
clean: clean-PROGRAMS
	rm -f *.o
	rm -f $(PROGRAMS)

	
clean-PROGRAMS:
	@list='$(EXECUTABLE)'; for p in $$list; do \
	  f=`echo $$p|sed 's/$$//'`; \
	  echo " rm -f $$p $$f"; \
	  rm -f $$p $$f ; \
	done

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:

