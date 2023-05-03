#!/bin/sh

spice_dir=/home/jdbowman/cspice
gcc lunar_gateway.c -o lunar_gateway -I$spice_dir/include $spice_dir/lib/cspice.a -lm -std=gnu99


