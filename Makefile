#
# Makefile
#
# calc
#

calc: calc.c cs50.h cs50.c
	clang -ggdb3 -O0 -std=c99 -Wall -Werror -o calc calc.c -lm

clean:
	rm -f *.o a.out core calc
