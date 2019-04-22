CC = gcc

CFLAGS=-std=c11 -Wall -Werror -O3 -Wpedantic

.SUFFIXES: .c .o

FLEXRAW=cm.l
FLEXC=lex.yy.c
BISONRAW=cm.y
BISONC=y.tab.c
BISONH=y.tab.h

SRCS=main.c util.c $(FLEXC) $(BISONC)
OBJS=$(SRCS:.c=.o)

20120085: $(FLEXC) $(BISONC) $(OBJS)
	$(CC) -o $@ $(OBJS)

lex.yy.c: $(FLEXRAW)
	flex -o $(FLEXC) $(FLEXRAW)

y.tab.c: $(BISONRAW)
	bison -o $(BISONC) -d $(BISONRAW)

.PHONY: clean
clean:
	rm -f $(OBJS) 20120085 $(FLEXC) $(BISONC) $(BISONH)