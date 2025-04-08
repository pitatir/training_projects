all: main.o iobmp.o functions.o
	gcc main.o iobmp.o functions.o -o cw_bmp

main.o: main.c errors.h structs.h iobmp.h functions.h
	gcc -c main.c

iobmp.o: iobmp.h errors.h structs.h
	gcc -c iobmp.c

functions.o: functions.h errors.h structs.h iobmp.h
	gcc -c functions.c

