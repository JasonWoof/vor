#!/bin/sh

povray -D +A -O- +FP $* 2>/dev/null | pnmcrop | pnmtopng -transparent =black 2>/dev/null
