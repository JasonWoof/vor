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

DATA_PREFIX := /usr/share/vor
PROGRAM_PREFIX := /usr/games/bin

CFLAGS := -Wall -ggdb
LDFLAGS := 

paths := -DDATA_PREFIX=\"$(DATA_PREFIX)\"
sdl-cflags := $(shell sdl-config --cflags)
sdl-ldflags := $(shell sdl-config --libs)

ldflags := $(sdl-ldflags) -lSDL_image -lSDL_mixer $(LDFLAGS)
cflags := $(sdl-cflags) $(paths) $(CFLAGS)

my_objects := args.o dust.o file.o mt.o rocks.o score.o sprite.o sound.o
my_objects += main.o
libs := SFont.o
objects := $(libs) $(my_objects)

rocks := 00 01 02 03 04 05 06 07 08 09
rocks += 10 11 12 13 14 15 16 17 18 19
rocks += 20 21 22 23 24 25 26 27 28 29
rocks += 30 31 32 33 34 35 36 37 38 39
rocks += 40 41 42 43 44 45 46 47 48 49
rocks := $(rocks:%=data/sprites/rock%.png)
graphics := data/sprites/ship.png data/indicators/life.png $(rocks)

INSTALL := install
INSTALL_PROGRAM := $(INSTALL) -o games -g games
INSTALL_DATA := $(INSTALL) -m 644


.PHONY: all clean maintainer-clean install uninstall
.PHONY: mkinstalldirs rminstalldirs
.PHONY: program program-clean install-program uninstall-program
.PHONY: data data-clean install-data uninstall-data

all: program data

data: $(graphics)

program: vor

%.o: %.c
	@echo compiling $@ from $<
	@$(CC) $(cflags) -c -o $@ $<

$(my_objects): config.h

SFont.o: SFont.h

args.o: args.h

dust.o: globals.h dust.h mt.h

file.o: file.h common.h

main.o: args.h common.h dust.h file.h globals.h mt.h rocks.h score.h sprite.h sound.h

mt.o: mt.h

rocks.o: rocks.h common.h file.h globals.h mt.h sprite.h

score.o: score.h common.h file.h

sound.o: sound.h args.h common.h

sprite.o: sprite.h common.h

vor: $(objects)
	@echo linking $@ from $^
	@$(CC) $(ldflags) -o $@ $^ $(LIBRARIES)

include gfx.mk

tags: *.c *.h
	exuberant-ctags *.c *.h /usr/include/SDL/*

clean: program-clean
	rm -f tags

maintainer-clean: program-clean data-clean

program-clean:
	rm -f *.o vor

data-clean:
	rm -f $(graphics)

mkinstalldirs:
	if [ ! -d $(DATA_PREFIX) ]; then mkdir $(DATA_PREFIX); fi
	if [ ! -d $(DATA_PREFIX)/banners ]; then mkdir $(DATA_PREFIX)/banners; fi
	if [ ! -d $(DATA_PREFIX)/fonts ]; then mkdir $(DATA_PREFIX)/fonts; fi
	if [ ! -d $(DATA_PREFIX)/icons ]; then mkdir $(DATA_PREFIX)/icons; fi
	if [ ! -d $(DATA_PREFIX)/indicators ]; then mkdir $(DATA_PREFIX)/indicators; fi
	if [ ! -d $(DATA_PREFIX)/music ]; then mkdir $(DATA_PREFIX)/music; fi
	if [ ! -d $(DATA_PREFIX)/sounds ]; then mkdir $(DATA_PREFIX)/sounds; fi
	if [ ! -d $(DATA_PREFIX)/sprites ]; then mkdir $(DATA_PREFIX)/sprites; fi

rminstalldirs:
	if [ -d $(DATA_PREFIX)/banners ]; then rmdir $(DATA_PREFIX)/banners; fi
	if [ -d $(DATA_PREFIX)/fonts ]; then rmdir $(DATA_PREFIX)/fonts; fi
	if [ -d $(DATA_PREFIX)/icons ]; then rmdir $(DATA_PREFIX)/icons; fi
	if [ -d $(DATA_PREFIX)/indicators ]; then rmdir $(DATA_PREFIX)/indicators; fi
	if [ -d $(DATA_PREFIX)/music ]; then rmdir $(DATA_PREFIX)/music; fi
	if [ -d $(DATA_PREFIX)/sounds ]; then rmdir $(DATA_PREFIX)/sounds; fi
	if [ -d $(DATA_PREFIX)/sprites ]; then rmdir $(DATA_PREFIX)/sprites; fi
	if [ -d $(DATA_PREFIX) ]; then rmdir $(DATA_PREFIX); fi

install: all mkinstalldirs install-program install-data

install-program: program
	$(INSTALL_PROGRAM) ./vor $(PROGRAM_PREFIX)

install-data: data
	$(INSTALL_DATA) ./data/banners/* $(DATA_PREFIX)/banners/
	$(INSTALL_DATA) ./data/fonts/* $(DATA_PREFIX)/fonts/
	$(INSTALL_DATA) ./data/icons/* $(DATA_PREFIX)/icons/
	$(INSTALL_DATA) ./data/indicators/* $(DATA_PREFIX)/indicators/
	$(INSTALL_DATA) ./data/music/* $(DATA_PREFIX)/music/
	$(INSTALL_DATA) ./data/sounds/* $(DATA_PREFIX)/sounds/
	$(INSTALL_DATA) ./data/sprites/* $(DATA_PREFIX)/sprites/

uninstall: uninstall-program uninstall-data rminstalldirs

uninstall-program:
	rm -f $(PROGRAM_PREFIX)/vor

uninstall-data:
	rm -f $(DATA_PREFIX)/banners/*
	rm -f $(DATA_PREFIX)/fonts/*
	rm -f $(DATA_PREFIX)/icons/*
	rm -f $(DATA_PREFIX)/indicators/*
	rm -f $(DATA_PREFIX)/music/*
	rm -f $(DATA_PREFIX)/sounds/*
	rm -f $(DATA_PREFIX)/sprites/*
	rm -f $(DATA_PREFIX)/scores
