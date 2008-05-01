LIB = ${LIB_PREFIX}upse${LIB_SUFFIX}
LIB_MAJOR = 2
LIB_MINOR = 0

SRCS = upse_string.c			\
       upse_util.c			\
       upse_unpack_psf.c		\
       upse_bios_manager.c 		\
       upse_r3000_bytecode_evaluator.c	\
       upse_r3000_abstract.c		\
       upse_ps1_executive.c		\
       upse_ps1_counters.c		\
       upse_ps1_gpu.c			\
       upse_ps1_hal.c			\
       upse_ps1_bios_base.c		\
       upse_ps1_memory_manager.c	\
       upse_ps1_dma_manager.c		\
       upse_ps1_spu_base.c		\
       upse_ps1_spu_dma_handler.c	\
       upse_ps1_spu_register_io.c	\
       upse_ps1_spu_reverb.c		\
       upse_ps1_spu_adsr_filter.c	\

INCLUDES = upse.h upse-types.h

include ../../buildsys.mk
include ../../extra.mk

CFLAGS += ${PLUGIN_CFLAGS}
CPPFLAGS += ${PLUGIN_CPPFLAGS} -I../../intl -I../.. -Ispu/ -I.
LIBS += -lz
