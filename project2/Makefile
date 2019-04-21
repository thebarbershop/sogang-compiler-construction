CC = gcc

CFLAGS=-std=c11 -Wall -Werror -O3 -Wpedantic -Wconversion

.SUFFIXES: .c .o

SRCS=main.c util.c lex.yy.c y.tab.c
OBJS=$(SRCS:.c=.o)

20120085: $(OBJS)
	$(CC) -o $@ $(OBJS)

lex.yy.c: cmin.l
	flex cmin.l

y.tab.c: cmin.y
	bison -d cmin.y

.PHONY: clean
clean:
	rm -f $(OBJS) 20120085 lex.yy.c y.tab.c