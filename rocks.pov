// pass in a random seed (integer) to this file with +K

#include "colors.inc"

#declare RS = seed(clock);

#macro rnd() (rand(RS)) #end

// rock size. The larger this number, the closer the camera, and the smaller the bumps.
#declare rsize = 1 + rnd();

camera
{
	up <0, 1, 0>
	right <1, 0, 0>
	location <0, 0, -4 / rsize>
	angle 30
	look_at <0, 0, 0>

	angle 55
}

// same light sorce as ship.pov
light_source { <-500, 500, -700> White }


blob{
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	sphere { <0.5 - rnd(), 0.5 - rnd(), 0.5 - rnd()>, .1 + rnd(), 1 }
	threshold 2
	texture{
		pigment{ color rgb <53 / 55, 44 / 55, 36 / 55>}
	}
	normal { bumps 1 scale (0.16 / rsize) }

	finish { ambient .1}
}
