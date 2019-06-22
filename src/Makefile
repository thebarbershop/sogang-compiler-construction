CC = gcc
LEX = flex
YACC = bison

CFLAGS=-std=c11 -Wall -Werror -O3 -Wpedantic

.SUFFIXES: .c .o

BINARY=project4_17
LEXRAW=cm.l
LEXC=lex.yy.c
YACCRAW=cm.y
YACCC=y.tab.c
YACCH=y.tab.h
YACCOUTPUT=y.output

SRCS=main.c util.c symtab.c analyze.c parse.c code.c cgen.c $(LEXC) $(YACCC)
OBJS=$(SRCS:.c=.o)

$(BINARY): $(LEXC) $(YACCC) $(OBJS)
	$(CC) -o $@ $(OBJS)

$(LEXC): $(LEXRAW)
	$(LEX) -o $(LEXC) $(LEXRAW)

$(YACCC): $(YACCRAW)
	$(YACC) -o $(YACCC) -d $(YACCRAW) --report=all --report-file=$(YACCOUTPUT)

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINARY) $(LEXC) $(YACCC) $(YACCH) $(YACCOUTPUT)
