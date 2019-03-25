CC = gcc

CFLAGS = -Wall -O3

.SUFFIXES: .c .o

LEXCODE=cmin.l
LEXSRC=lex.yy.c

SRCS=main.c util.c lex.yy.c
OBJS=$(SRCS:.c=.o)

20120085: lex.yy.c $(OBJS)
	$(CC) -o $@ $(OBJS)

lex.yy.c: cmin.l
	flex cmin.l

clean:
	rm -f $(OBJS) 20120085 lex.yy.c