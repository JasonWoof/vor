
gfx-deps := gfx.mk povimg.sh

data/ship.png: ship.pov $(gfx-deps)
	./povimg.sh +W32 +H32 $< > $@

data/life.png: ship.pov $(gfx-deps)
	./povimg.sh +W17 +H17 $< > $@

data/rock%.png: rocks.pov $(gfx-deps)
	./povimg.sh +H52 +W52 +K$* $< > $@
