#
#  Copyright (c) 2007, Jonathan Schleifer <js@h3c.de>
#
#  Permission to use, copy, modify, and/or distribute this software for any
#  purpose with or without fee is hereby granted, provided that the above
#  copyright notice and this permission notice is present in all copies.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

PACKAGE = audacious-plugins
CC = gcc
CXX = g++
CPP = gcc -E
DC = @DC@
ERLC = @ERLC@
OBJC = @OBJC@
AR = /usr/bin/ar
LD = ${CC}
CFLAGS = -g -O2 -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/gtk-2.0 -I/usr/include/libmowgli -I/usr/lib64/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/freetype2 -I/usr/include/libpng12   -I/usr/include/dbus-1.0 -I/usr/lib64/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include   
CXXFLAGS = -g -O2 -INONE/include -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/gtk-2.0 -I/usr/include/libmowgli -I/usr/lib64/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/freetype2 -I/usr/include/libpng12   -I/usr/include/dbus-1.0 -I/usr/lib64/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include  
CPPFLAGS = 
DFLAGS = @DFLAGS@
ERLCFLAGS = @ERLCFLAGS@
OBJCFLAGS = @OBJCFLAGS@
LDFLAGS = 
LIBS = 
LIB_CPPFLAGS = -DPIC
LIB_CFLAGS = -fPIC
LIB_LDFLAGS = -shared -fPIC -Wl,-soname=${LIB}.${LIB_MAJOR}.${LIB_MINOR}.0
LIB_PREFIX = lib
LIB_SUFFIX = .so
PLUGIN_CPPFLAGS = -DPIC
PLUGIN_CFLAGS = -fPIC
PLUGIN_LDFLAGS = -shared -fPIC
PLUGIN_SUFFIX = .so
INSTALL_LIB = ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR}.0 && ${LN_S} -f $$i.${LIB_MAJOR}.${LIB_MINOR}.0 ${DESTDIR}${libdir}/$$i.${LIB_MAJOR} && ${LN_S} -f $$i.${LIB_MAJOR}.${LIB_MINOR}.0 ${DESTDIR}${libdir}/$$i
UNINSTALL_LIB = rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR} ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR}.0
CLEAN_LIB = 
LN_S = ln -s
MKDIR_P = mkdir -p
INSTALL = /usr/bin/install -c
SHELL = /bin/sh
prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
libdir = ${exec_prefix}/lib
plugindir ?= ${PACKAGE}
datarootdir = ${prefix}/share
datadir = ${datarootdir}
includedir = ${prefix}/include
includesubdir ?= ${PACKAGE}
mandir = ${datarootdir}/man
mansubdir ?= man1

OBJS1 = ${SRCS:.c=.o}
OBJS2 = ${OBJS1:.cc=.o}
OBJS3 = ${OBJS2:.cxx=.o}
OBJS4 = ${OBJS3:.d=.o}
OBJS5 = ${OBJS4:.erl=.beam}
OBJS += ${OBJS5:.m=.o}

.SILENT:
.SUFFIXES: .beam .c .cc .cxx .d .erl .m
.PHONY: all subdirs depend install uninstall clean distclean

all:
	for i in subdirs depend ${OBJS} ${STATIC_LIB} ${STATIC_LIB_NOINST} ${LIB} ${LIB_NOINST} ${PLUGIN} ${PLUGIN_NOINST} ${PROG} ${PROG_NOINST}; do \
		${MAKE} ${MFLAGS} $$i; \
	done

subdirs:
	for i in ${SUBDIRS}; do \
		${DIR_ENTER}; \
		${MAKE} ${MFLAGS} || exit 1; \
		${DIR_LEAVE}; \
	done

depend: pre-depend ${SRCS}
	regen=0; \
	for i in ${SRCS}; do [ $$i -nt .deps ] && regen=1; done; \
	if [ $$regen = 1 ]; then \
		error=0; \
		${DEPEND_STATUS}; \
		rm -f .deps; \
		for i in ${SRCS}; do \
			case $${i##*.} in \
			c|cc|cxx|m) \
				${CPP} ${CPPFLAGS} -M $$i >>.deps || error=1; \
				;; \
			esac; \
		done; \
		if [ $$error = 0 ]; then \
			${DEPEND_OK}; \
		else \
			${DEPEND_FAILED}; \
		fi; \
	fi

pre-depend:

${PROG} ${PROG_NOINST}: ${EXT_DEPS} ${OBJS}
	${LINK_STATUS}
	if ${LD} -o $@ ${OBJS} ${LDFLAGS} ${LIBS}; then \
		${LINK_OK}; \
	else \
		${LINK_FAILED}; \
	fi

${LIB} ${LIB_NOINST}: ${EXT_DEPS} ${OBJS}
	${LINK_STATUS}
	if ${LD} -o $@ ${OBJS} ${LIB_LDFLAGS} ${LDFLAGS} ${LIBS}; then \
		${LINK_OK}; \
	else \
		${LINK_FAILED}; \
	fi

${PLUGIN} ${PLUGIN_NONST}: ${EXT_DEPS} ${OBJS}
	${LINK_STATUS}
	if ${LD} -o $@ ${OBJS} ${PLUGIN_LDFLAGS} ${LDFLAGS} ${LIBS}; then \
		${LINK_OK}; \
	else \
		${LINK_FAILED}; \
	fi

${STATIC_LIB} ${STATIC_LIB_NOINST}: ${EXT_DEPS} ${OBJS}
	${LINK_STATUS}
	if ${AR} cr $@ ${OBJS}; then \
		${LINK_OK}; \
	else \
		${LINK_FAILED}; \
	fi

#${EXT_DEPS}: subdirs

.c.o:
	${COMPILE_STATUS}
	if ${CC} ${CFLAGS} ${CPPFLAGS} -c -o $@ $<; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi

.cc.o .cxx.o:
	${COMPILE_STATUS}
	if ${CXX} ${CXXFLAGS} ${CPPFLAGS} -c -o $@ $<; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi

.d.o:
	${COMPILE_STATUS}
	if test x"$(basename ${DC})" = x"dmd"; then \
		if ${DC} ${DFLAGS} -c -of$@ $<; then \
			${COMPILE_OK}; \
		else \
			${COMPILE_FAILED}; \
		fi \
	else \
		if ${DC} ${DFLAGS} -c -o $@ $<; then \
			${COMPILE_OK}; \
		else \
			${COMPILE_FAILED}; \
		fi \
	fi

.erl.beam:
	${COMPILE_STATUS}
	if ${ERLC} ${ERLCFLAGS} -o $@ $<; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi

.m.o:
	${COMPILE_STATUS}
	if ${OBJC} ${OBJCFLAGS} ${CPPFLAGS} -c -o $@ $<; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi

install: ${LIB} ${STATIC_LIB} ${PLUGIN} ${PROG} install-extra
	for i in ${SUBDIRS}; do \
		${DIR_ENTER}; \
		${MAKE} ${MFLAGS} install || exit 1; \
		${DIR_LEAVE}; \
	done

	for i in ${LIB}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} ${DESTDIR}${libdir} && ${INSTALL_LIB}; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

	for i in ${STATIC_LIB}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} ${DESTDIR}${libdir} && ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

	for i in ${PLUGIN}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} ${DESTDIR}${libdir}/${plugindir} && ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/${plugindir}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

	for i in ${DATA}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} $$(dirname ${DESTDIR}${datadir}/${PACKAGE}/$$i) && ${INSTALL} -m 644 $$i ${DESTDIR}${datadir}/${PACKAGE}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

	for i in ${PROG}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} ${DESTDIR}${bindir} && ${INSTALL} -m 755 -s $$i ${DESTDIR}${bindir}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

	for i in ${INCLUDES}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} ${DESTDIR}${includedir}/${includesubdir} && ${INSTALL} -m 644 $$i ${DESTDIR}${includedir}/${includesubdir}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

	for i in ${MAN}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} ${DESTDIR}${mandir}/${mansubdir} && ${INSTALL} -m 644 $$i ${DESTDIR}${mandir}/${mansubdir}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

install-extra:

uninstall: uninstall-extra
	for i in ${SUBDIRS}; do \
		${DIR_ENTER}; \
		${MAKE} ${MFLAGS} uninstall || exit 1; \
		${DIR_LEAVE}; \
	done

	for i in ${LIB}; do \
		if [ -f ${DESTDIR}${libdir}/$$i ]; then \
			if ${UNINSTALL_LIB}; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi; \
	done

	for i in ${STATIC_LIB}; do \
		if [ -f ${DESTDIR}${libdir}/$$i ]; then \
			if rm -f ${DESTDIR}${libdir}/$$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done

	for i in ${PLUGIN}; do \
		if [ -f ${DESTDIR}${libdir}/${plugindir}/$$i ]; then \
			if rm -f ${DESTDIR}${libdir}/${plugindir}/$$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done
	-rmdir ${DESTDIR}${libdir}/${plugindir} >/dev/null 2>&1

	for i in ${DATA}; do \
		if [ -f ${DESTDIR}${datadir}/${PACKAGE}/$$i ]; then \
			if rm -f ${DESTDIR}${datadir}/${PACKAGE}/$$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done

	for i in ${PROG}; do \
		if [ -f ${DESTDIR}${bindir}/$$i ]; then \
			if rm -f ${DESTDIR}${bindir}/$$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done

	for i in ${INCLUDES}; do \
		if [ -f ${DESTDIR}${includedir}/${includesubdir}/$$i ]; then \
			if rm -f ${DESTDIR}${includedir}/${includesubdir}/$$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done
	-rmdir ${DESTDIR}${includedir}/${includesubdir} >/dev/null 2>&1

	for i in ${MAN}; do \
		if [ -f ${DESTDIR}${mandir}/${mansubdir}/$$i ]; then \
			if rm -f ${DESTDIR}${mandir}/${mansubdir}/$$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done

uninstall-extra:

clean:
	for i in ${SUBDIRS}; do \
		${DIR_ENTER}; \
		${MAKE} ${MFLAGS} clean || exit 1; \
		${DIR_LEAVE}; \
	done
	
	for i in ${OBJS} ${CLEAN} ${CLEAN_LIB} .deps *~; do \
		if [ -f $$i -o -d $$i ]; then \
			if rm -fr $$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done

distclean: clean
	for i in ${SUBDIRS}; do \
		${DIR_ENTER}; \
		${MAKE} ${MFLAGS} distclean || exit 1; \
		${DIR_LEAVE}; \
	done
	
	for i in ${PROG} ${PROG_NOINST} ${STATIC_LIB} ${STATIC_LIB_NOINST}  ${PLUGIN} ${PLUGIN_NOINST} ${DISTCLEAN}; do \
		if [ -f $$i -o -d $$i ]; then \
			if rm -fr $$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done

DIR_ENTER = printf "\033[0;36mEntering directory \033[1;36m$$i\033[0;36m.\033[0m\n"; cd $$i || exit 1
DIR_LEAVE = printf "\033[0;36mLeaving directory \033[1;36m$$i\033[0;36m.\033[0m\n"; cd .. || exit 1
DEPEND_STATUS = printf "\033[0;33mGenerating dependencies...\033[0m\r"
DEPEND_OK = printf "\033[0;32mSuccessfully generated dependencies.\033[0m\n"
DEPEND_FAILED = printf "\033[0;31mFailed to generate dependencies!\033[0m\n"; exit 1
COMPILE_STATUS = printf "\033[0;33mCompiling \033[1;33m$<\033[0;33m...\033[0m\r"
COMPILE_OK = printf "\033[0;32mSuccessfully compiled \033[1;32m$<\033[0;32m.\033[0m\n"
COMPILE_FAILED = printf "\033[0;31mFailed to compile \033[1;31m$<\033[0;31m!\033[0m\n"; exit 1
LINK_STATUS = printf "\033[0;33mLinking \033[1;33m$@\033[0;33m...\033[0m\r"
LINK_OK = printf "\033[0;32mSuccessfully linked \033[1;32m$@\033[0;32m.\033[0m\n"
LINK_FAILED = printf "\033[0;31mFailed to link \033[1;31m$@\033[0;31m!\033[0m\n"; exit 1
INSTALL_STATUS = printf "\033[0;33mInstalling \033[1;33m$$i\033[0;33m...\033[0m\r"
INSTALL_OK = printf "\033[0;32mSuccessfully installed \033[1;32m$$i\033[0;32m.\033[0m\n"
INSTALL_FAILED = printf "\033[0;31mFailed to install \033[1;31m$$i\033[0;31m!\033[0m\n"; exit 1
DELETE_OK = printf "\033[0;34mDeleted \033[1;34m$$i\033[0;34m.\033[0m\n"
DELETE_FAILED = printf "\033[0;31mFailed to delete \033[1;31m$$i\033[0;31m!\033[0m\n"; exit 1

-include .deps
