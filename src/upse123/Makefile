PROG = upse123${PROG_SUFFIX}

SRCS = upse123_main.c

MAN = upse123.1

include ../../buildsys.mk
include ../../extra.mk

plugindir := ${plugindir}/${INPUT_PLUGIN_DIR}

CFLAGS += 
CPPFLAGS += $(AO_CFLAGS) -I../libupse
LIBS += -lz -L../libupse -lupse $(AO_LIBS)
