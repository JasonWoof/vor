#include "colors.inc"
#include "metals.inc"

#ifndef(xr)
	#declare xr = 0;
#end

#ifndef(yr)
	#declare yr = 0;
#end

#declare r = 2.0;
#declare r2 = 2.01;
#declare cr = 1/16;

camera {
	up <0, 1, 0>
	right <1, 0, 0>
	location <0, 0, -r/sin(radians(15))>
	angle 30
	look_at <0, 0, 0>
}
light_source { <-500, 500, -700> White }

#declare xring = intersection {
	difference {
		sphere { 0, r }
		sphere { 0, r-cr }
	}
	box { <-0.25, -r2, -r2>, <0.25, r2, r2> }
}

#declare yring = intersection {
	difference {
		sphere { 0, r }
		sphere { 0, r-cr }
	}
	box { <-r2, -0.25, -r2>, <r2, 0.25, r2> }
}

#declare zring = intersection {
	difference {
		sphere { 0, r }
		sphere { 0, r-cr }
	}
	box { <-r2, -r2, -0.25>, <r2, r2, 0.25> }
}


union {
	sphere { 0, r-0.1 }
	intersection {
		sphere { 0, r-0.09 }
		box { <0, 0, -r2>, <r2, r2, r2> }
		pigment { rgbf < 0.5, 0.75, 0.5, 0.6 > }
	}
	object { xring }
	object { yring }
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

	rotate <xr*360/32, 0, 0>
	rotate <0, yr*360/32, 0>
}
