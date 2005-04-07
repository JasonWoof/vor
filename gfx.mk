

data/sprites/ship.png: ship.pov pnmoutline gfx.mk
	povray -GA -D +A +UA +W32 +H32 $< 2>/dev/null
	pngtopnm ship.png >ship.pnm
	./pnmoutline <ship.pnm >data/sprites/ship.pnm
	pnmtopng -transparent =white data/sprites/ship.pnm >$@
	rm ship.png ship.pnm data/sprites/ship.pnm

data/indicators/life.png: ship.pov gfx.mk
	povray -D +A +UA +W17 +H17 $< -O$@

data/sprites/rock%.png: rocks.pov gfx.mk
	povray -Irocks.pov -D +H52 +W52 +K`echo "$@" | grep -o '[0-9][0-9]'` +Fp -O$@.pnm 2>/dev/null
	pnmcrop < $@.pnm > $@-c.pnm
	pnmtopng -transparent black < $@-c.pnm > $@
	rm $@.pnm $@-c.pnm
