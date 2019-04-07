#!/bin/bash
scp main.c frtk2019077@head.vdi.mipt.ru:~/recv
ssh frtk2019077@head.vdi.mipt.ru <<'ENDSSH'
cd recv
make
ls
mpirun -np 32 --oversubscribe ./run 5

exit
ENDSSH