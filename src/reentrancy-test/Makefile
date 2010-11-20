PROG_NOINST = reentrancy-test${PROG_SUFFIX}

SRCS = reentrancy_test_main.c

include ../../buildsys.mk
include ../../extra.mk

CFLAGS += 
CPPFLAGS += -pthread -I../libupse
LIBS += -pthread -lz -L../libupse -lupse
