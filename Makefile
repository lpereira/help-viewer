GTK_LIBS = `pkg-config gtk+-2.0 --libs`
GTK_CFLAGS = `pkg-config gtk+-2.0 --cflags`

CCFLAGS = -fPIC -pipe -Wall -g 
CFLAGS = $(GTK_CFLAGS)
CC = gcc -Os $(ARCHOPTS) -g $(CFLAGS)

# ----------------------------------------------------------------------------

OBJECTS = egg-markdown.o markdown-text-view.o help-viewer.o

all:	$(OBJECTS)
	$(CC) $(CCFLAGS) -o help-viewer $(OBJECTS) $(GTK_LIBS)

clean:
	rm -rf .xvpics pixmaps/.xvpics *.o *.so help-viewer
	find . -name \*~ -exec rm -v {} \;
