.PHONY: all clean stack

CC = gcc
PROG = dentaku
CFLAGS = -g -W -Wall -std=gnu99
LDFLAGS = -lm

YACC = yacc
LEX = lex


SRC_DIR = .
SRC = main.c dentaku-core.c dentaku-stack.c dentaku-parser.c dentaku-recursion.c lexer.c token.c util.c op.c alloc-list.c

STACK_SRC = libdatastruct/stack.c libdatastruct/common.c
STACK_OBJS = $(STACK_SRC:.c=.o)

LIST_SRC = mylib/list/list.c
LIST_OBJS = $(LIST_SRC:.c=.o)

PARSER_YACC_SRC = parser/calc.y
PARSER_LEX_SRC = parser/calc.l
PARSER_SRC = $(PARSER_YACC_SRC) $(PARSER_LEX_SRC)
PARSER_OBJS = y.tab.o lex.yy.o

ALL_OBJS = $(SRC:.c=.o) $(STACK_OBJS) $(LIST_OBJS) $(PARSER_OBJS)



all: $(PROG)

test: $(PROG)
	-@echo -n "\n\n"
	\perl test.pl ./$(PROG) -f stack
	\perl test.pl ./$(PROG) -f parser
	\perl test.pl ./$(PROG) -f recursion
leak-test: $(PROG)
	-@echo -n "\n\n"
	\valgrind --leak-check=full ./$(PROG) -f stack
	\valgrind --leak-check=full ./$(PROG) -f parser
	\valgrind --leak-check=full ./$(PROG) -f recursion



$(PROG): $(ALL_OBJS)
	-@echo -n "\n\n"
	-\rm -f y.tab.c y.tab.h y.output lex.yy.c    # clean up yacc&lex's garbages
	$(CC) $(LDFLAGS) -o $@ $(ALL_OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@


$(PARSER_OBJS): $(PARSER_SRC)
	-@echo -n "\n\n"
	$(YACC) -dv $(PARSER_YACC_SRC)
	$(CC) $(CFLAGS) -c y.tab.c -o y.tab.o
	$(LEX) $(PARSER_LEX_SRC)
	$(CC) $(CFLAGS) -c lex.yy.c -o lex.yy.o


depend:
	-@echo -n "\n\n"
	\makedepend -Y$(SRC_DIR) -- $(CFLAGS) -- $(SRC) 2>/dev/null

clean:
	-@echo -n "\n\n"
	-\rm -f $(PROG) $(ALL_OBJS) y.tab.c y.tab.h lex.yy.c
# DO NOT DELETE

main.o: dentaku.h common.h digit.h
dentaku-core.o: dentaku-core.h common.h digit.h libdatastruct/stack.h
dentaku-core.o: libdatastruct/common_public.h dentaku-stack.h
dentaku-core.o: dentaku-parser.h dentaku-recursion.h util.h op.h alloc-list.h
dentaku-core.o: token.h
dentaku-stack.o: dentaku-stack.h common.h digit.h dentaku-core.h util.h
dentaku-stack.o: lexer.h op.h token.h libdatastruct/stack.h
dentaku-stack.o: libdatastruct/common_public.h
dentaku-parser.o: dentaku-parser.h common.h digit.h dentaku-core.h
dentaku-parser.o: libdatastruct/stack.h libdatastruct/common_public.h
dentaku-recursion.o: dentaku-recursion.h common.h digit.h dentaku-core.h
dentaku-recursion.o: lexer.h
lexer.o: lexer.h common.h digit.h token.h util.h
token.o: token.h common.h digit.h alloc-list.h
util.o: util.h common.h digit.h token.h
op.o: op.h common.h digit.h util.h
alloc-list.o: alloc-list.h common.h digit.h util.h mylib/list/list.h
