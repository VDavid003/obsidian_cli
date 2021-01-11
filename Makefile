CC=gcc
LIBS=-lusb-1.0
VERSION=0.1.0
CFLAGS=-DVERSION=\"$(VERSION)\"

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

%.o: %.c obsidian_cli.h
	$(CC) -c -o $@ $< $(CFLAGS)

obsidian_cli: main.o 
	$(CC) -o $@ main.o $(LIBS)

install: obsidian_cli 60-obsidian.rules
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 obsidian_cli $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)$(PREFIX)/lib/udev/rules.d/
	install -m 644 60-obsidian.rules $(DESTDIR)/lib/udev/rules.d/

clean :
	-rm *.o obsidian_cli
