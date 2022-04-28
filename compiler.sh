#! /bin/bash
make
./compiler $1
spim -file assembly.asm