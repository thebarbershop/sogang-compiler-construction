CC = gcc

CFLAGS=-std=c11 -Wall -Werror -O3 -Wpedantic -Wconversion

.SUFFIXES: .c .o

SRCS=main.c util.c lex.yy.c
OBJS=$(SRCS:.c=.o)

20120085: lex.yy.c $(OBJS)
	$(CC) -o $@ $(OBJS)

lex.yy.c: cmin.l
	flex cmin.l

.PHONY: clean
clean:
	rm -f $(OBJS) 20120085 lex.yy.c