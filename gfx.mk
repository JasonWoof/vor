
gfx-deps := gfx.mk povimg.sh

data/sprites/ship.png: ship.pov $(gfx-deps)
	./povimg.sh +W32 +H32 $< > $@

data/indicators/life.png: ship.pov $(gfx-deps)
	./povimg.sh +W17 +H17 $< > $@

data/sprites/rock%.png: rocks.pov $(gfx-deps)
	./povimg.sh +H52 +W52 +K$* $< > $@
