#!/bin/sh

rm -f rel/*.img

#

cp ver/annex_a.h mydef.h
make clean
make
mv uboot.bin rel/uboot_n55u.img

#

cp ver/annex_b.h mydef.h
make clean
make
mv uboot.bin rel/uboot_n55u_b.img
