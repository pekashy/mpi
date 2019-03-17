#!/bin/bash
mpicc -std=gnu99 main.c -o gravedigger4
ls
mpirun -np 50 --oversubscribe ./gravedigger4 4
