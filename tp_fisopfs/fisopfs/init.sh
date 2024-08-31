#!/bin/bash

make clean

rmdir prueba

#Si queres que no haya persistencia descomenta la siguiente linea
#rm fs.fisopfs

make

mkdir prueba

./fisopfs -f prueba/