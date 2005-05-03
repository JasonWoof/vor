#   Rock Dodger! Avoid the rocks as long as you can!
#   Copyright (C) 2001  Paul Holt <pad@pcholt.com>

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

debug := $(if $(DEBUG),1,0)
ldflags := $(shell sdl-config --libs) -lSDL_image -lSDL_mixer
cflags := $(shell sdl-config --cflags) -Wall -DDEBUG=$(debug) $(CFLAGS)

my_objects := file.o rocks.o score.o shape.o sound.o main.o $(if $(DEBUG),debug.o)
libs := SFont.o
objects := $(libs) $(my_objects)

rocks := 00 01 02 03 04 05 06 07 08 09
rocks += 10 11 12 13 14 15 16 17 18 19
rocks += 20 21 22 23 24 25 26 27 28 29
rocks += 30 31 32 33 34 35 36 37 38 39
rocks += 40 41 42 43 44 45 46 47 48 49

graphics := data/sprites/ship.png data/indicators/life.png $(rocks:%=data/sprites/rock%.png)

INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -o games -g games
INSTALL_DATA = $(INSTALL) -m 644

DATA_PREFIX = /usr/share/vor
PROGRAM_PREFIX = /usr/games/bin


all: vor $(graphics)

%.o: %.c
	$(CC) $(cflags) -c -o $@ $<

$(my_objects): config.h

main.o file.o: file.h

vor: $(objects)
	$(CC) $(ldflags) -o $@ $^ $(LIBRARIES)

include gfx.mk

clean:
	rm -f *.o vor $(graphics)

install:	all
	if [ ! -d $(DATA_PREFIX) ]; then mkdir $(DATA_PREFIX); fi
	if [ ! -d $(DATA_PREFIX)/banners ]; then mkdir $(DATA_PREFIX)/banners; fi
	if [ ! -d $(DATA_PREFIX)/fonts ]; then mkdir $(DATA_PREFIX)/fonts; fi
	if [ ! -d $(DATA_PREFIX)/icons ]; then mkdir $(DATA_PREFIX)/icons; fi
	if [ ! -d $(DATA_PREFIX)/indicators ]; then mkdir $(DATA_PREFIX)/indicators; fi
	if [ ! -d $(DATA_PREFIX)/music ]; then mkdir $(DATA_PREFIX)/music; fi
	if [ ! -d $(DATA_PREFIX)/sounds ]; then mkdir $(DATA_PREFIX)/sounds; fi
	if [ ! -d $(DATA_PREFIX)/sprites ]; then mkdir $(DATA_PREFIX)/sprites; fi

	$(INSTALL_PROGRAM) ./vor $(PROGRAM_PREFIX)
	$(INSTALL_DATA) ./data/banners/* $(DATA_PREFIX)/banners/
	$(INSTALL_DATA) ./data/fonts/* $(DATA_PREFIX)/fonts/
	$(INSTALL_DATA) ./data/icons/* $(DATA_PREFIX)/icons/
	$(INSTALL_DATA) ./data/indicators/* $(DATA_PREFIX)/indicators/
	$(INSTALL_DATA) ./data/music/* $(DATA_PREFIX)/music/
	$(INSTALL_DATA) ./data/sounds/* $(DATA_PREFIX)/sounds/
	$(INSTALL_DATA) ./data/sprites/* $(DATA_PREFIX)/sprites/
	touch $(DATA_PREFIX)/scores
	chmod a+rw $(DATA_PREFIX)/scores

uninstall:
	rm -f $(PROGRAM_PREFIX)/vor
	rm -f $(DATA_PREFIX)/banners/*
	rm -f $(DATA_PREFIX)/fonts/*
	rm -f $(DATA_PREFIX)/icons/*
	rm -f $(DATA_PREFIX)/indicators/*
	rm -f $(DATA_PREFIX)/music/*
	rm -f $(DATA_PREFIX)/sounds/*
	rm -f $(DATA_PREFIX)/sprites/*
	rm -f $(DATA_PREFIX)/scores $(DATA_PREFIX)/.highscore

	if [ -d $(DATA_PREFIX)/banners ]; then rmdir $(DATA_PREFIX)/banners; fi
	if [ -d $(DATA_PREFIX)/fonts ]; then rmdir $(DATA_PREFIX)/fonts; fi
	if [ -d $(DATA_PREFIX)/icons ]; then rmdir $(DATA_PREFIX)/icons; fi
	if [ -d $(DATA_PREFIX)/indicators ]; then rmdir $(DATA_PREFIX)/indicators; fi
	if [ -d $(DATA_PREFIX)/music ]; then rmdir $(DATA_PREFIX)/music; fi
	if [ -d $(DATA_PREFIX)/sounds ]; then rmdir $(DATA_PREFIX)/sounds; fi
	if [ -d $(DATA_PREFIX)/sprites ]; then rmdir $(DATA_PREFIX)/sprites; fi
	if [ -d $(DATA_PREFIX) ]; then rmdir $(DATA_PREFIX); fi
