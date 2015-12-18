#!/bin/bash
x=0
echo "enter count devices for module (1,2 - n):"
read x
let "rez = $x - 1"
echo "$rez"
sudo insmod ./kbuf.ko  countDev=$rez
