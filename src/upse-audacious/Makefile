PLUGIN = upse-audacious${PLUGIN_SUFFIX}

SRCS = plugin.c

include ../../buildsys.mk
include ../../extra.mk

# XXX
plugindir = `pkg-config audacious --variable=input_plugin_dir`

CFLAGS += ${PLUGIN_CFLAGS}
CPPFLAGS += ${PLUGIN_CPPFLAGS} ${AUDACIOUS_CFLAGS} -I../libupse
LIBS += ${AUDACIOUS_LIBS} -lz -L../libupse -lupse
