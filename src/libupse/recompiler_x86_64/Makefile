STATIC_LIB_NOINST = upse_recompiler_x64.a

SRCS = upse_recompiler_x64_base.c	\
       upse_recompiler_x64_cpudetect.c  \
       upse_recompiler_x64_impl.c	\
       upse_recompiler_x64_primitives.c

include ../../../buildsys.mk
include ../../../extra.mk

CFLAGS += ${PLUGIN_CFLAGS}
CPPFLAGS += ${PLUGIN_CPPFLAGS} -I../../../intl -I../../.. -Ispu/ -I.
LIBS += -lz
