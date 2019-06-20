CC = gcc


CFLAGS=-std=c11 -Wall -Werror -O3 -Wpedantic

.SUFFIXES: .c .o

BINARY=project3_17
FLEXRAW=cm.l
FLEXC=lex.yy.c
BISONRAW=cm.y
BISONC=y.tab.c
BISONH=y.tab.h
BISONOUTPUT=y.output

SRCS=main.c util.c symtab.c analyze.c parse.c code.c cgen.c $(FLEXC) $(BISONC)
OBJS=$(SRCS:.c=.o)

$(BINARY): $(FLEXC) $(BISONC) $(OBJS)
	$(CC) -o $@ $(OBJS)

lex.yy.c: $(FLEXRAW)
	flex -o $(FLEXC) $(FLEXRAW)

y.tab.c: $(BISONRAW)
	bison -o $(BISONC) -d $(BISONRAW) --report=all --report-file=$(BISONOUTPUT)

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINARY) $(FLEXC) $(BISONC) $(BISONH) $(BISONOUTPUT)