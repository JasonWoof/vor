#!/bin/sh

povray -D +A -O- +FP $* 2>/dev/null | pnmcrop | pnmtopng -transparent '#000000' -gamma .45 2>/dev/null
