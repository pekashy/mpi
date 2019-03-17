#!/bin/bash
scp main.c frtk2019077@head.vdi.mipt.ru:~/recv
ssh frtk2019077@head.vdi.mipt.ru <<'ENDSSH'
cd recv
mpicc -std=gnu99 main.c -o gravedigger3
ls
mpirun -np 20 --oversubscribe gravedigger3 4
exit
ENDSSH