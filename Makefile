# Hardcoded...
all: libgamepad

libgamepad: libgamepad.c libgamepad.h
	clang -Wall -g -c libgamepad.c -o libgamepad.o
	clang -dynamiclib -current_version 0.1 libgamepad.o /usr/lib/hidapi.o -framework IOKit -framework CoreFoundation -framework ApplicationServices -o libgamepad.dylib

install: libgamepad
	cp libgamepad.dylib /usr/lib/libgamepad.dylib
	cp libgamepad.h /usr/include/libgamepad.h
	chmod ugo+xr /usr/lib/libgamepad.dylib

clean:
	rm -rf *.dylib
	rm -rf *.o

.PHONY: clean
