# How the font is generated, and how to edit it.

The font used in VoR is generated from the pixel layout in
`font_template.txt.gz`. You can edit this file easily with vim. Don't unzip it,
vim handles this for you. Just open it up and do `:set nowrap`. You should now
see the font and be able to edit it nicely with replace mode (hit capital `R`).

The font is generated from this template by creating a huge blob object in
povray. A small diffuse sphere is created at the location of each `#` in the
template file, and the result is rendered where the density of all those
combined spheres is high enough.

If you create a font that has considerably different dimensions you will have
to fiddle with a few numbers in the sources:

  * update the `TEMPLATE_WIDTH` and `TEMPLATE_HEIGHT` constants in `font_guts.c`

  * If you've changed the aspect ratio considerably: update the "up" and
    "right" vectors in `font.pov` These determine the aspect ratio, and what area
    of the coordinate space is rendered. Be sure to leave enough space around
    the edges so the font doesn't get clipped. Don't worry about excess black
    around the edges, it will be cropped automatically after rendering. The
    constant `OUTPUT_WIDTH` in `font_guts.c` is in povray units.


The font template was created from the 10x20 font from the misc-fixed pack. It
was tweaked slightly in particular so the dots on the `j`, `i` and `!`
characters don't touch the rest of the character.
