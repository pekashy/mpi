#!/bin/bash
make
ls
mpirun -np 16 --oversubscribe ./run 4
