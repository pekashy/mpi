#!/bin/bash
make
#ls
mpirun -np 4 --oversubscribe ./run 2