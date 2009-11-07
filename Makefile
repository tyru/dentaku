.PHONY: all clean stack

CC = gcc
PROG = dentaku
CFLAGS = -g -W -Wall -std=gnu99
LDFLAGS = -lm

SRC = main.c dentaku-core.c dentaku-stack.c myparser.c token.c util.c op.c alloc-list.c

STACK_SRC = libdatastruct/stack.c libdatastruct/common.c
STACK_OBJS = $(STACK_SRC:.c=.o)
LIST_SRC = mylib/list/list.c
LIST_OBJS = $(LIST_SRC:.c=.o)

ALL_OBJS = $(SRC:.c=.o) $(STACK_OBJS) $(LIST_OBJS)



all: $(PROG)

test: $(PROG)
	\perl test.pl
leak-test: $(PROG)
	\valgrind --leak-check=full ./$(PROG)



$(PROG): $(ALL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(ALL_OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@



depend:
	\makedepend -- $(CFLAGS) -- $(SRC)

clean:
	-\rm -f $(PROG) $(ALL_OBJS) stack.o
# DO NOT DELETE

main.o: dentaku.h common.h digit.h /usr/include/stdio.h
main.o: /usr/include/features.h /usr/include/bits/predefs.h
main.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
main.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
main.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
main.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
main.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
main.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
main.o: /usr/include/endian.h /usr/include/bits/endian.h
main.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
main.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
main.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
main.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
main.o: /usr/include/string.h /usr/include/xlocale.h /usr/include/errno.h
main.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
main.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
main.o: /usr/include/asm-generic/errno-base.h /usr/include/setjmp.h
main.o: /usr/include/bits/setjmp.h /usr/include/assert.h
dentaku-core.o: dentaku-core.h common.h digit.h /usr/include/stdio.h
dentaku-core.o: /usr/include/features.h /usr/include/bits/predefs.h
dentaku-core.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
dentaku-core.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
dentaku-core.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
dentaku-core.o: /usr/include/libio.h /usr/include/_G_config.h
dentaku-core.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
dentaku-core.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
dentaku-core.o: /usr/include/sys/types.h /usr/include/time.h
dentaku-core.o: /usr/include/endian.h /usr/include/bits/endian.h
dentaku-core.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
dentaku-core.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
dentaku-core.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
dentaku-core.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
dentaku-core.o: /usr/include/string.h /usr/include/xlocale.h
dentaku-core.o: /usr/include/errno.h /usr/include/bits/errno.h
dentaku-core.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
dentaku-core.o: /usr/include/asm-generic/errno.h
dentaku-core.o: /usr/include/asm-generic/errno-base.h /usr/include/setjmp.h
dentaku-core.o: /usr/include/bits/setjmp.h libdatastruct/stack.h
dentaku-core.o: libdatastruct/common_public.h dentaku-stack.h util.h op.h
dentaku-core.o: alloc-list.h token.h /usr/include/assert.h
dentaku-core.o: /usr/include/getopt.h /usr/include/unistd.h
dentaku-core.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
dentaku-stack.o: dentaku-stack.h common.h digit.h /usr/include/stdio.h
dentaku-stack.o: /usr/include/features.h /usr/include/bits/predefs.h
dentaku-stack.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
dentaku-stack.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
dentaku-stack.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
dentaku-stack.o: /usr/include/libio.h /usr/include/_G_config.h
dentaku-stack.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
dentaku-stack.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
dentaku-stack.o: /usr/include/sys/types.h /usr/include/time.h
dentaku-stack.o: /usr/include/endian.h /usr/include/bits/endian.h
dentaku-stack.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
dentaku-stack.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
dentaku-stack.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
dentaku-stack.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
dentaku-stack.o: /usr/include/string.h /usr/include/xlocale.h
dentaku-stack.o: /usr/include/errno.h /usr/include/bits/errno.h
dentaku-stack.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
dentaku-stack.o: /usr/include/asm-generic/errno.h
dentaku-stack.o: /usr/include/asm-generic/errno-base.h /usr/include/setjmp.h
dentaku-stack.o: /usr/include/bits/setjmp.h libdatastruct/stack.h
dentaku-stack.o: libdatastruct/common_public.h dentaku-core.h util.h
dentaku-stack.o: myparser.h op.h token.h
myparser.o: myparser.h common.h digit.h /usr/include/stdio.h
myparser.o: /usr/include/features.h /usr/include/bits/predefs.h
myparser.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
myparser.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
myparser.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
myparser.o: /usr/include/libio.h /usr/include/_G_config.h
myparser.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
myparser.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
myparser.o: /usr/include/sys/types.h /usr/include/time.h
myparser.o: /usr/include/endian.h /usr/include/bits/endian.h
myparser.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
myparser.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
myparser.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
myparser.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
myparser.o: /usr/include/string.h /usr/include/xlocale.h /usr/include/errno.h
myparser.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
myparser.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
myparser.o: /usr/include/asm-generic/errno-base.h /usr/include/setjmp.h
myparser.o: /usr/include/bits/setjmp.h token.h util.h /usr/include/ctype.h
token.o: token.h common.h digit.h /usr/include/stdio.h
token.o: /usr/include/features.h /usr/include/bits/predefs.h
token.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
token.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
token.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
token.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
token.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
token.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
token.o: /usr/include/endian.h /usr/include/bits/endian.h
token.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
token.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
token.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
token.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
token.o: /usr/include/string.h /usr/include/xlocale.h /usr/include/errno.h
token.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
token.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
token.o: /usr/include/asm-generic/errno-base.h /usr/include/setjmp.h
token.o: /usr/include/bits/setjmp.h alloc-list.h
util.o: util.h common.h digit.h /usr/include/stdio.h /usr/include/features.h
util.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
util.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
util.o: /usr/include/gnu/stubs-32.h /usr/include/bits/types.h
util.o: /usr/include/bits/typesizes.h /usr/include/libio.h
util.o: /usr/include/_G_config.h /usr/include/wchar.h
util.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
util.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
util.o: /usr/include/endian.h /usr/include/bits/endian.h
util.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
util.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
util.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
util.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
util.o: /usr/include/string.h /usr/include/xlocale.h /usr/include/errno.h
util.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
util.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
util.o: /usr/include/asm-generic/errno-base.h /usr/include/setjmp.h
util.o: /usr/include/bits/setjmp.h token.h
op.o: op.h common.h digit.h /usr/include/stdio.h /usr/include/features.h
op.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
op.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
op.o: /usr/include/gnu/stubs-32.h /usr/include/bits/types.h
op.o: /usr/include/bits/typesizes.h /usr/include/libio.h
op.o: /usr/include/_G_config.h /usr/include/wchar.h
op.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
op.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
op.o: /usr/include/endian.h /usr/include/bits/endian.h
op.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
op.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
op.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
op.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
op.o: /usr/include/string.h /usr/include/xlocale.h /usr/include/errno.h
op.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
op.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
op.o: /usr/include/asm-generic/errno-base.h /usr/include/setjmp.h
op.o: /usr/include/bits/setjmp.h util.h /usr/include/math.h
op.o: /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h
op.o: /usr/include/bits/huge_vall.h /usr/include/bits/inf.h
op.o: /usr/include/bits/nan.h /usr/include/bits/mathdef.h
op.o: /usr/include/bits/mathcalls.h
alloc-list.o: alloc-list.h common.h digit.h /usr/include/stdio.h
alloc-list.o: /usr/include/features.h /usr/include/bits/predefs.h
alloc-list.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
alloc-list.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
alloc-list.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
alloc-list.o: /usr/include/libio.h /usr/include/_G_config.h
alloc-list.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
alloc-list.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
alloc-list.o: /usr/include/sys/types.h /usr/include/time.h
alloc-list.o: /usr/include/endian.h /usr/include/bits/endian.h
alloc-list.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
alloc-list.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
alloc-list.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
alloc-list.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
alloc-list.o: /usr/include/string.h /usr/include/xlocale.h
alloc-list.o: /usr/include/errno.h /usr/include/bits/errno.h
alloc-list.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
alloc-list.o: /usr/include/asm-generic/errno.h
alloc-list.o: /usr/include/asm-generic/errno-base.h /usr/include/setjmp.h
alloc-list.o: /usr/include/bits/setjmp.h util.h mylib/list/list.h
