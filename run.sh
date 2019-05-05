#!/bin/bash
make
#ls
mpirun -np 9 --oversubscribe ./run 2