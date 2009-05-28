#   Variations on Rockdodger
#   Copyright (C) 2004  Joshua Grams <josh@qualdan.com>

#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.

#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

prefix = /usr/local
exec_prefix = $(prefix)

datarootdir = $(prefix)/share
pkgdatadir = $(datarootdir)/vor
bindir = $(exec_prefix)/bin

CFLAGS := -Wall -O3
LDFLAGS := 

paths := -DDATA_PREFIX=\"$(pkgdatadir)\"
sdl-cflags := $(shell sdl-config --cflags)
sdl-ldflags := $(shell sdl-config --libs)

ldflags := $(sdl-ldflags) -lSDL_image -lSDL_mixer $(LDFLAGS)
cflags := $(sdl-cflags) $(paths) $(CFLAGS)

my_objects := args.o dust.o file.o mt.o rocks.o score.o sprite.o sound.o autopilot.o
my_objects += main.o
libs := font.o
objects := $(libs) $(my_objects)

rocks := 00 01 02 03 04 05 06 07 08 09
rocks += 10 11 12 13 14 15 16 17 18 19
rocks += 20 21 22 23 24 25 26 27 28 29
rocks += 30 31 32 33 34 35 36 37 38 39
rocks += 40 41 42 43 44 45 46 47 48 49
rocks := $(rocks:%=data/rock%.png)
graphics := data/ship.png data/icon.png data/life.png data/font.png $(rocks)

INSTALL := install
INSTALL_PROGRAM := $(INSTALL)
INSTALL_DATA := $(INSTALL) -m 644


.PHONY: all clean maintainer-clean install uninstall
.PHONY: mkinstalldirs rminstalldirs
.PHONY: program program-clean install-program uninstall-program
.PHONY: data data-clean install-data uninstall-data

.INTERMEDIATE: font_guts font_guts.pov

all: program data

data: $(graphics)

program: vor

%.o: %.c
	$(CC) $(cflags) -c -o $@ $<

$(my_objects): config.h

font.o: font.h

args.o: args.h

dust.o: globals.h dust.h float.h mt.h

file.o: file.h common.h

main.o: args.h common.h dust.h file.h float.h globals.h mt.h rocks.h score.h sprite.h sound.h autopilot.h

mt.o: mt.h

rocks.o: rocks.h common.h file.h globals.h mt.h sprite.h

score.o: score.h common.h file.h

sound.o: sound.h args.h common.h

sprite.o: sprite.h common.h

vor: $(objects)
	$(CC) $^ $(ldflags) -o $@

include gfx.mk

tags: *.c *.h
	exuberant-ctags *.c *.h /usr/include/SDL/*

clean: program-clean
	rm -f tags

maintainer-clean: program-clean data-clean

program-clean:
	rm -f *.o vor

data-clean:
	rm -f $(graphics) font_guts font_guts.pov

mkinstalldirs:
	if [ ! -d $(DESTDIR)$(pkgdatadir) ]; then mkdir -p $(DESTDIR)$(pkgdatadir); fi
	if [ ! -d $(DESTDIR)$(bindir) ]; then mkdir -p $(DESTDIR)$(bindir); fi

rminstalldirs: uninstall-data
	if [ -d $(DESTDIR)$(pkgdatadir) ]; then rmdir $(DESTDIR)$(pkgdatadir); fi

install: all mkinstalldirs install-program install-data

install-program: program
	$(INSTALL_PROGRAM) ./vor $(DESTDIR)$(bindir)/

install-data: data mkinstalldirs
	$(INSTALL_DATA) ./data/*.png $(DESTDIR)$(pkgdatadir)/
	$(INSTALL_DATA) ./data/*.wav $(DESTDIR)$(pkgdatadir)/
	$(INSTALL_DATA) ./data/*.xm $(DESTDIR)$(pkgdatadir)/
	@echo
	@echo "$(pkgdatadir)/icon.png (48x48) or ship.png (32x32) make good icons."
	@echo

uninstall: uninstall-program uninstall-data rminstalldirs

uninstall-program:
	rm -f $(DESTDIR)$(bindir)/vor

uninstall-data:
	rm -f $(DESTDIR)$(pkgdatadir)/*.png
	rm -f $(DESTDIR)$(pkgdatadir)/*.wav
	rm -f $(DESTDIR)$(pkgdatadir)/*.xm
