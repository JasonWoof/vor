// pass in a random seed (integer) to this file with +K

#include "colors.inc"

#declare RS = seed(clock + 1);

#macro rnd() (rand(RS)) #end


camera
{
	orthographic
	// the _length_ of the up and right vectors determine the aspect ratio of the image (and in the case of an orthographic projection, also the _area_ visible (unless angle is defined, in which case the angle determines the visible area))
	up <0, 0.40, 0>
	right <30, 0, 0>
	location <0, 0, -200>
	look_at <0, 0, 0>

}

// same light sorce as ship.pov
light_source { <-500, 500, -700> White }


blob{
	#include "font_guts.pov"
	threshold 3
	texture {
		pigment { rgb < 0.75, 0.75, 1.0 > }
		finish {
			ambient 0.35
			brilliance 2
			diffuse 0.3
			metallic
			specular 0.6
			roughness 1/60
			reflection 0.25
		}
		normal { bumps 0.1 scale 0.25 }
	}
}
