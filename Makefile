.PHONY: all clean stack

CC = gcc
PROG = dentaku
CFLAGS = -g -W -Wall -std=gnu99

SRC = main.c dentaku.c parser.c token.c util.c
OBJS = $(SRC:.c=.o)


all: $(PROG)


test: $(PROG) leak-test
leak-test:
	\valgrind ./$(PROG)


debug: $(PROG)
	\gdb ./$(PROG)


foo:
	\echo "src:" $(SRC)
	\echo "objs:" $(OBJS)

$(PROG): $(OBJS) stack
	$(CC) $(LDFLAGS) -o $@ $(OBJS) stack.o

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

stack: stack.o

stack.o: libdatastruct/stack.c libdatastruct/stack.h
	$(CC) $(CFLAGS) -c libdatastruct/stack.c -o stack.o


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
main.o: /usr/include/asm-generic/errno-base.h /usr/include/assert.h token.h
main.o: libdatastruct/stack.h /usr/include/setjmp.h
main.o: /usr/include/bits/setjmp.h
dentaku.o: dentaku.h common.h digit.h /usr/include/stdio.h
dentaku.o: /usr/include/features.h /usr/include/sys/cdefs.h
dentaku.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
dentaku.o: /usr/include/gnu/stubs-32.h /usr/include/bits/types.h
dentaku.o: /usr/include/bits/typesizes.h /usr/include/libio.h
dentaku.o: /usr/include/_G_config.h /usr/include/wchar.h
dentaku.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
dentaku.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
dentaku.o: /usr/include/endian.h /usr/include/bits/endian.h
dentaku.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
dentaku.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
dentaku.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
dentaku.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
dentaku.o: /usr/include/string.h /usr/include/errno.h
dentaku.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
dentaku.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
dentaku.o: /usr/include/asm-generic/errno-base.h /usr/include/assert.h
dentaku.o: token.h libdatastruct/stack.h /usr/include/setjmp.h
dentaku.o: /usr/include/bits/setjmp.h util.h parser.h /usr/include/getopt.h
dentaku.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
dentaku.o: /usr/include/bits/confname.h
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
