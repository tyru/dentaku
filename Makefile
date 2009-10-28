.PHONY: all clean stack

CC = gcc
PROG = dentaku
CFLAGS = -g -W -Wall -std=gnu99
LDFLAGS = -lm

SRC = main.c dentaku-core.c dentaku-stack.c parser.c token.c util.c op.c

STACK_SRC = libdatastruct/stack.c
STACK_OBJS = $(STACK_SRC:.c=.o)
LIST_SRC = mylib/list/list.c
LIST_OBJS = $(LIST_SRC:.c=.o)
OBJS = $(SRC:.c=.o) $(STACK_OBJS) $(LIST_OBJS)


all: $(PROG)

test: $(PROG)
	\perl test.pl
leak-test: $(PROG)
	\valgrind --leak-check=full ./$(PROG)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

# making *.o files
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
$(STACK_OBJS): $(STACK_SRC)
	$(CC) $(CFLAGS) -c $< -o $@
$(LIST_OBJS): $(LIST_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

depend:
	\makedepend -- $(CFLAGS) -- $(SRC)

clean:
	\rm -f $(PROG) $(OBJS) stack.o
# DO NOT DELETE

main.o: dentaku.h common.h digit.h /usr/include/stdio.h
main.o: /usr/include/features.h /usr/include/sys/cdefs.h
main.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
main.o: /usr/include/gnu/stubs-32.h /usr/include/bits/types.h
main.o: /usr/include/bits/typesizes.h /usr/include/libio.h
main.o: /usr/include/_G_config.h /usr/include/wchar.h
main.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
main.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
main.o: /usr/include/endian.h /usr/include/bits/endian.h
main.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
main.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
main.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
main.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
main.o: /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h
main.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
main.o: /usr/include/asm-generic/errno.h
main.o: /usr/include/asm-generic/errno-base.h /usr/include/assert.h
main.o: /usr/include/setjmp.h /usr/include/bits/setjmp.h
dentaku-core.o: dentaku-core.h common.h digit.h /usr/include/stdio.h
dentaku-core.o: /usr/include/features.h /usr/include/sys/cdefs.h
dentaku-core.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
dentaku-core.o: /usr/include/gnu/stubs-32.h /usr/include/bits/types.h
dentaku-core.o: /usr/include/bits/typesizes.h /usr/include/libio.h
dentaku-core.o: /usr/include/_G_config.h /usr/include/wchar.h
dentaku-core.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
dentaku-core.o: /usr/include/stdlib.h /usr/include/sys/types.h
dentaku-core.o: /usr/include/time.h /usr/include/endian.h
dentaku-core.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
dentaku-core.o: /usr/include/sys/select.h /usr/include/bits/select.h
dentaku-core.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
dentaku-core.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
dentaku-core.o: /usr/include/alloca.h /usr/include/string.h
dentaku-core.o: /usr/include/errno.h /usr/include/bits/errno.h
dentaku-core.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
dentaku-core.o: /usr/include/asm-generic/errno.h
dentaku-core.o: /usr/include/asm-generic/errno-base.h /usr/include/assert.h
dentaku-core.o: dentaku.h /usr/include/setjmp.h /usr/include/bits/setjmp.h
dentaku-core.o: token.h libdatastruct/stack.h mylib/list/list.h
dentaku-core.o: dentaku-stack.h util.h op.h /usr/include/getopt.h
dentaku-core.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
dentaku-core.o: /usr/include/bits/confname.h
dentaku-stack.o: dentaku-stack.h common.h digit.h /usr/include/stdio.h
dentaku-stack.o: /usr/include/features.h /usr/include/sys/cdefs.h
dentaku-stack.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
dentaku-stack.o: /usr/include/gnu/stubs-32.h /usr/include/bits/types.h
dentaku-stack.o: /usr/include/bits/typesizes.h /usr/include/libio.h
dentaku-stack.o: /usr/include/_G_config.h /usr/include/wchar.h
dentaku-stack.o: /usr/include/bits/stdio_lim.h
dentaku-stack.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
dentaku-stack.o: /usr/include/sys/types.h /usr/include/time.h
dentaku-stack.o: /usr/include/endian.h /usr/include/bits/endian.h
dentaku-stack.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
dentaku-stack.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
dentaku-stack.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
dentaku-stack.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
dentaku-stack.o: /usr/include/string.h /usr/include/errno.h
dentaku-stack.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
dentaku-stack.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
dentaku-stack.o: /usr/include/asm-generic/errno-base.h /usr/include/assert.h
dentaku-stack.o: dentaku.h /usr/include/setjmp.h /usr/include/bits/setjmp.h
dentaku-stack.o: dentaku-core.h token.h libdatastruct/stack.h
dentaku-stack.o: mylib/list/list.h util.h parser.h
parser.o: parser.h common.h digit.h /usr/include/stdio.h
parser.o: /usr/include/features.h /usr/include/sys/cdefs.h
parser.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
parser.o: /usr/include/gnu/stubs-32.h /usr/include/bits/types.h
parser.o: /usr/include/bits/typesizes.h /usr/include/libio.h
parser.o: /usr/include/_G_config.h /usr/include/wchar.h
parser.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
parser.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
parser.o: /usr/include/endian.h /usr/include/bits/endian.h
parser.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
parser.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
parser.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
parser.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
parser.o: /usr/include/string.h /usr/include/errno.h
parser.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
parser.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
parser.o: /usr/include/asm-generic/errno-base.h /usr/include/assert.h token.h
parser.o: util.h /usr/include/ctype.h
token.o: token.h common.h digit.h /usr/include/stdio.h
token.o: /usr/include/features.h /usr/include/sys/cdefs.h
token.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
token.o: /usr/include/gnu/stubs-32.h /usr/include/bits/types.h
token.o: /usr/include/bits/typesizes.h /usr/include/libio.h
token.o: /usr/include/_G_config.h /usr/include/wchar.h
token.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
token.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
token.o: /usr/include/endian.h /usr/include/bits/endian.h
token.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
token.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
token.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
token.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
token.o: /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h
token.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
token.o: /usr/include/asm-generic/errno.h
token.o: /usr/include/asm-generic/errno-base.h /usr/include/assert.h
util.o: util.h common.h digit.h /usr/include/stdio.h /usr/include/features.h
util.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
util.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
util.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
util.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
util.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
util.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
util.o: /usr/include/endian.h /usr/include/bits/endian.h
util.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
util.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
util.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
util.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
util.o: /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h
util.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
util.o: /usr/include/asm-generic/errno.h
util.o: /usr/include/asm-generic/errno-base.h /usr/include/assert.h
op.o: op.h common.h digit.h /usr/include/stdio.h /usr/include/features.h
op.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
op.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
op.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
op.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
op.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
op.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
op.o: /usr/include/endian.h /usr/include/bits/endian.h
op.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
op.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
op.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
op.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
op.o: /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h
op.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
op.o: /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h
op.o: /usr/include/assert.h util.h /usr/include/math.h
op.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
op.o: /usr/include/bits/mathcalls.h
