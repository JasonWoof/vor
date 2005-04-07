

data/sprites/ship.png: ship.pov gfx.mk
	povray -D +A +W32 +H32 +FN -O$@ $< 2>/dev/null

data/indicators/life.png: ship.pov gfx.mk
	povray -D +A +UA +W17 +H17 $< -O$@ 2>/dev/null

data/sprites/rock%.png: rocks.pov gfx.mk
	povray -Irocks.pov -D +H52 +W52 +K`echo "$@" | grep -o '[0-9][0-9]'` +Fp -O$@.pnm 2>/dev/null
	pnmcrop < $@.pnm > $@-c.pnm
	pnmtopng -transparent black < $@-c.pnm > $@
	rm $@.pnm $@-c.pnm
