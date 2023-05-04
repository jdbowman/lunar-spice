#!/usr/bin/env bash

spice_dir=/home/jdbowman/cspice
gcc lunar_astro.c -o lunar_astro -I$spice_dir/include $spice_dir/lib/cspice.a -lm -std=gnu99


