.PHONY: all release test leak-test deps clean tags

CC = gcc
PROG = dentaku
CFLAGS_RELEASE = -O2 -Wall -std=gnu99
CFLAGS = -g -Wall -std=gnu99
LDFLAGS = -lm -lgmp


YACC = yacc
LEX = lex
STRIP = strip
PERL = perl
VALGRIND = valgrind
CTAGS = ctags


SRC_DIR = .
SRC = main.c dentaku-core.c dentaku-stack.c dentaku-parser.c dentaku-recursion.c lexer.c token.c util.c op.c alloc-list.c digit.c

STACK_SRC = libdatastruct/stack.c libdatastruct/common.c
STACK_OBJS = $(STACK_SRC:.c=.o)

LIST_SRC = nstl/list/list.c
LIST_OBJS = $(LIST_SRC:.c=.o)

PARSER_YACC_SRC = parser/calc.y
PARSER_LEX_SRC = parser/calc.l
PARSER_SRC = $(PARSER_YACC_SRC) $(PARSER_LEX_SRC)
PARSER_OBJS = y.tab.o lex.yy.o

ALL_OBJS = $(SRC:.c=.o) $(STACK_OBJS) $(LIST_OBJS) $(PARSER_OBJS)




all: deps tags $(PROG)

release: clean
	-@\echo -n "\n\n"
	CFLAGS=$(CFLAGS_RELEASE) make $(PROG)
	$(STRIP) $(PROG)



test: $(PROG)
	-@\echo -n "\n\n"
	$(PERL) test.pl ./$(PROG) -f stack
	$(PERL) test.pl ./$(PROG) -f parser
	$(PERL) test.pl ./$(PROG) -f recursion
leak-test: $(PROG)
	-@\echo -n "\n\n"
	$(VALGRIND) --leak-check=full ./$(PROG) -f stack
	$(VALGRIND) --leak-check=full ./$(PROG) -f parser
	$(VALGRIND) --leak-check=full ./$(PROG) -f recursion



$(PROG): $(ALL_OBJS)
	-@\echo -n "\n\n"
	$(CC) $(LDFLAGS) -o $@ $(ALL_OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@


$(PARSER_OBJS): $(PARSER_SRC)
	-@\echo -n "\n\n"
	$(YACC) -dv $(PARSER_YACC_SRC)
	$(CC) $(CFLAGS) -c y.tab.c -o y.tab.o
	$(LEX) $(PARSER_LEX_SRC)
	$(CC) $(CFLAGS) -c lex.yy.c -o lex.yy.o


deps:
	-@\echo -n "\n\n"
	$(CC) -MM $(SRC) > deps

clean:
	-@\echo -n "\n\n"
	-\rm -f $(PROG) $(ALL_OBJS) y.tab.c y.tab.h lex.yy.c

tags:
	$(CTAGS)


include deps
