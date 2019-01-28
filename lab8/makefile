
lab8 : function.o lab8.o
	gcc -g function.o lab8.o -o lab8 -lm -std=c99

function.o : function.c function.h
	gcc -g -c function.c -std=c99

lab8.o : lab8.c function.h
	gcc -g -c lab8.c -std=c99
