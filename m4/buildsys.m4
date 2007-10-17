dnl
dnl Copyright (c) 2007, Jonathan Schleifer <js@h3c.de>
dnl
dnl Permission to use, copy, modify, and/or distribute this software for any
dnl purpose with or without fee is hereby granted, provided that the above
dnl copyright notice and this permission notice is present in all copies.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
dnl AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
dnl IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
dnl ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
dnl LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
dnl CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
dnl SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
dnl INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
dnl CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
dnl ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
dnl POSSIBILITY OF SUCH DAMAGE.
dnl

AC_DEFUN([BUILDSYS_PROG_IMPLIB], [
	AC_MSG_CHECKING(whether we need an implib)
	case "$target" in
		*-*-cygwin | *-*-mingw32)
			AC_MSG_RESULT(yes)
			PROG_IMPLIB_NEEDED='yes'
			PROG_IMPLIB_LDFLAGS='-Wl,-export-all-symbols,--out-implib,lib${PROG}.a'
			;;
		*)
			AC_MSG_RESULT(no)
			PROG_IMPLIB_NEEDED='no'
			PROG_IMPLIB_LDFLAGS=''
			;;
	esac
	
	AC_SUBST(PROG_IMPLIB_NEEDED)
	AC_SUBST(PROG_IMPLIB_LDFLAGS)
])

AC_DEFUN([BUILDSYS_SHARED_LIB], [
	AC_MSG_CHECKING(for shared library system)
	case "$target" in
		intel-apple-*)
			AC_MSG_RESULT([Mac OS X (Intel)])
			LIB_CPPFLAGS='-DPIC'
			LIB_CFLAGS='-fPIC'
			LIB_LDFLAGS='-dynamiclib -fPIC -install_name ${libdir}/${LIB}'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.dylib'
			PLUGIN_CPPFLAGS=''
			PLUGIN_CFLAGS=''
			PLUGIN_LDFLAGS='-bundle -fno-common -flat_namespace -undefined suppress'
			PLUGIN_SUFFIX='.impl'
			INSTALL_LIB='${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib && ${LN_S} -f $${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.dylib && ${LN_S} -f $${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib ${DESTDIR}${libdir}/$$i'
			UNINSTALL_LIB='rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.dylib ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib'
			CLEAN_LIB=''
			;;
		*-apple-*)
			AC_MSG_RESULT(Mac OS X)
			LIB_CPPFLAGS='-DPIC'
			LIB_CFLAGS=''
			LIB_LDFLAGS='-dynamiclib -fPIC -install_name ${libdir}/${LIB}'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.dylib'
			PLUGIN_CPPFLAGS=''
			PLUGIN_CFLAGS=''
			PLUGIN_LDFLAGS='-bundle -fno-common -flat_namespace -undefined suppress'
			PLUGIN_SUFFIX='.impl'
			INSTALL_LIB='${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib && ${LN_S} -f $${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.dylib && ${LN_S} -f $${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib ${DESTDIR}${libdir}/$$i'
			UNINSTALL_LIB='rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.dylib ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib'
			CLEAN_LIB=''
			;;
		*-*-solaris* | *-openbsd-* | *-mirbsd-*)
			AC_MSG_RESULT(Solaris)
			LIB_CPPFLAGS='-DPIC'
			LIB_CFLAGS='-fPIC'
			LIB_LDFLAGS='-shared -fPIC -Wl,-soname=${LIB}.${LIB_MAJOR}.${LIB_MINOR}'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.so'
			PLUGIN_CPPFLAGS='-DPIC'
			PLUGIN_CFLAGS='-fPIC'
			PLUGIN_LDFLAGS='-shared -fPIC'
			PLUGIN_SUFFIX='.so'
			INSTALL_LIB='${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR} && rm -f ${DESTDIR}${libdir}/$$i && ${LN_S} $$i.${LIB_MAJOR}.${LIB_MINOR} ${DESTDIR}${libdir}/$$i'
			UNINSTALL_LIB='rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR}'
			CLEAN_LIB=''
			;;
		*-*-cygwin | *-*-mingw32)
			AC_MSG_RESULT(Win32)
			LIB_CPPFLAGS='-DPIC'
			LIB_CFLAGS=''
			LIB_LDFLAGS='-shared -Wl,--out-implib,${LIB}.a'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.dll'
			PLUGIN_CPPFLAGS=''
			PLUGIN_CFLAGS=''
			PLUGIN_LDFLAGS='-shared'
			PLUGIN_SUFFIX='.dll'
			INSTALL_LIB='${INSTALL} -m 755 $$i ${DESTDIR}${bindir}/$$i && ${INSTALL} -m 755 $$i.a ${DESTDIR}${libdir}/$$i.a'
			UNINSTALL_LIB='rm -f ${DESTDIR}${bindir}/$$i ${DESTDIR}${libdir}/$$i.a'
			CLEAN_LIB='${LIB}.a'
			;;
		*)
			AC_MSG_RESULT(POSIX)
			LIB_CPPFLAGS='-DPIC'
			LIB_CFLAGS='-fPIC'
			LIB_LDFLAGS='-shared -fPIC -Wl,-soname=${LIB}.${LIB_MAJOR}.${LIB_MINOR}.0'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.so'
			PLUGIN_CPPFLAGS='-DPIC'
			PLUGIN_CFLAGS='-fPIC'
			PLUGIN_LDFLAGS='-shared -fPIC'
			PLUGIN_SUFFIX='.so'
			INSTALL_LIB='${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR}.0 && ${LN_S} -f $$i.${LIB_MAJOR}.${LIB_MINOR}.0 ${DESTDIR}${libdir}/$$i.${LIB_MAJOR} && ${LN_S} -f $$i.${LIB_MAJOR}.${LIB_MINOR}.0 ${DESTDIR}${libdir}/$$i'
			UNINSTALL_LIB='rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR} ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR}.0'
			CLEAN_LIB=''
			;;
	esac

	AC_SUBST(LIB_CPPFLAGS)
	AC_SUBST(LIB_CFLAGS)
	AC_SUBST(LIB_LDFLAGS)
	AC_SUBST(LIB_PREFIX)
	AC_SUBST(LIB_SUFFIX)
	AC_SUBST(PLUGIN_CPPFLAGS)
	AC_SUBST(PLUGIN_CFLAGS)
	AC_SUBST(PLUGIN_LDFLAGS)
	AC_SUBST(PLUGIN_SUFFIX)
	AC_SUBST(INSTALL_LIB)
	AC_SUBST(UNINSTALL_LIB)
	AC_SUBST(CLEAN_LIB)
])
