
gfx-deps := gfx.mk povimg.sh

data/icon.png: ship.pov $(gfx-deps)
	./povimg.sh +W48 +H48 $< > $@ || sh -c "rm $@; false"

data/ship.png: ship.pov $(gfx-deps)
	./povimg.sh +W32 +H32 $< > $@ || sh -c "rm $@; false"

data/life.png: ship.pov $(gfx-deps)
	./povimg.sh +W17 +H17 $< > $@ || sh -c "rm $@; false"

data/rock%.png: rocks.pov $(gfx-deps)
	./povimg.sh +H52 +W52 +K$* $< > $@ || sh -c "rm $@; false"

font_guts: font_guts.c

font_guts.pov: font_guts font_template.txt
	./font_guts < font_template.txt > $@ || sh -c "rm $@ && false"

data/font.png: font.pov font_guts.pov
	./povimg.sh +W3000 +H40 $< > $@ || sh -c "rm $@; false"
