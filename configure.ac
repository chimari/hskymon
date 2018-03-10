dnl Process this file with autoconf to produce a configure script.
AC_INIT(main.c)
PACKAGE=hskymon

dnl version number
MAJOR_VERSION=3
MINOR_VERSION=5
MICRO_VERSION=0
BUILD_REVISION=0
EXTRA_VERSION=
VERSION=$MAJOR_VERSION.$MINOR_VERSION.$MICRO_VERSION$EXTRA_VERSION

dnl set $target
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)
dnl AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
dnl AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)
AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(MICRO_VERSION)
AC_SUBST(BUILD_REVISION)


AM_CONFIG_HEADER(config.h)
AM_PROG_LIBTOOL
dnl AC_ARG_PROGRAM
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_CHECK_PROG(WINDRES, windres, windres)
AM_CONDITIONAL(HAVE_WINDRES, test x"$WINDRES" != x)

AC_PATH_PROGS(OBJCOPY, gobjcopy objcopy)

# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([pwd.h sys/utsname.h])

os_osx=no

case "$target" in
*-darwin*)
	os_osx=yes
        AC_DEFINE([USE_OSX],[],[Use Mac OS X])
dnl	CFLAGS="$CFLAGS -no-cpp-precomp -fno-common"
	;;
esac

dnl Win32
AC_ARG_WITH(win32,
		 AC_HELP_STRING([--with-win32],
		            [Make for Win32]),
			    [use_win32=yes])
if test x"${use_win32}" = "xyes"; then
AC_DEFINE(USE_WIN32,[], [Make for Win32])
CFLAGS="$CFLAGS -mms-bitfields -DWINVER=0x0501"
LDFLAGS="$LDFLAGS -mwindows"
WINMM_LIBS="-lwinmm"
WINSOCK_LIBS="-lws2_32"
localedir='share/locale'
else
WINMM_LIBS=""
WINSOCK_LIBS=""
localedir='${prefix}/${DATADIRNAME}/locale'
fi
AC_SUBST(localedir)
AC_SUBST(WINMM_LIBS)
AC_SUBST(WINSOCK_LIBS)

dnl Gtk+
AM_PATH_GLIB_2_0(2.24.0,, AC_MSG_ERROR([glib-2.x is required]), gthread gobject)
dnl GTK 3 or 2 
AC_ARG_WITH(gtk3,
             AC_HELP_STRING([--with-gtk3],
                        [use Gtk+3 instead of Gtk+2]),
                        [use_gtk3=yes])
if test x"${use_gtk3}" = "xyes"; then
GTK3_CFLAGS="`pkg-config gtk+-3.0 --cflags`"
GTK3_LIBS="`pkg-config gtk+-3.0 --libs`"
dnl    AM_PATH_GTK_3_0(3.22.0,, AC_MSG_ERROR([gtk+-2.x is required]), gthread gobject)
AC_DEFINE([USE_GTK3],[],[Use Gtk+3.x instead of Gtk+2])
else
AM_PATH_GTK_2_0(2.24.0,, AC_MSG_ERROR([gtk+-2.x is required]), gthread gobject)
AC_DEFINE([USE_GTK2],[],[Use Gtk+2.x instead of Gtk+3])
GTK3_CFLAGS=""
GTK3_LIBS=""
fi
AC_SUBST(GTK3_LIBS)
AC_SUBST(GTK3_CFLAGS)

dnl gresource
AC_PATH_PROG([GLIB_COMPILE_RESOURCES], [glib-compile-resources])

dnl GtkMACIntegration
 AC_ARG_WITH(gtkmac,
 		 AC_HELP_STRING([--with-gtkmac],
		            [make with gtkmacintegration]),
			    [with_gtkmac=yes])
 
 if test x"${with_gtkmac}" = "xyes"; then
   if test "$os_osx" == yes; then
     AC_DEFINE([USE_GTKMACINTEGRATION],[],[With gtkmacintegration])
     GTKMAC_CFLAGS="`pkg-config gtk-mac-integration-gtk2 --cflags`"
     GTKMAC_LIBS="`pkg-config gtk-mac-integration-gtk2 --libs`"
   else
     GTKMAC_CFALGS=""
     GTKMAC_LIBS=""
   fi
 else
   GTKMAC_CFALGS=""
   GTKMAC_LIBS=""
 fi
 AC_SUBST(GTKMAC_LIBS)
 AC_SUBST(GTKMAC_CFLAGS)

dnl  XMLRPC
 AC_ARG_WITH(xmlrpc,
 		 AC_HELP_STRING([--with-xmlrpc],
		            [make with xmlrpc for Gen2]),
			    [with_xmlrpc=yes])
 
 if test x"${with_xmlrpc}" = "xyes"; then
 if test x"${use_win32}" = "xyes"; then
 XMLRPC_CFALGS=""
 XMLRPC_LIBS=""
 else
 AC_DEFINE([USE_XMLRPC],[],[With xmlrpc for Gen2])

dnl for Ubuntu
dnl   --- from here ---
    XMLRPC_CFLAGS="`xmlrpc-c-config client --cflags`"
    XMLRPC_LIBS="`xmlrpc-c-config client --libs`"
dnl    XMLRPC_CFLAGS=""
dnl    XMLRPC_LIBS="-lxmlrpc_xmltok -lxmlrpc_xmlparse -lxmlrpc_util -lxmlrpc_client -lxmlrpc"
dnl   -----------------

dnl for Fedora
dnl   --- from here ---
dnl    XMLRPC_CFLAGS="`pkg-config xmlrpc --cflags`"
dnl    XMLRPC_CFLAGS="$XMLRPC_CFLAGS `pkg-config xmlrpc_client --cflags`"
dnl    XMLRPC_CFLAGS="$XMLRPC_CFLAGS `pkg-config xmlrpc_util --cflags`"
dnl
dnl    XMLRPC_LIBS="`pkg-config xmlrpc --libs`"
dnl    XMLRPC_LIBS="$XMLRPC_LIBS `pkg-config xmlrpc_client --libs`"
dnl    XMLRPC_LIBS="$XMLRPC_LIBS `pkg-config xmlrpc_util --libs`"
dnl   -----------------
 fi
 else
 XMLRPC_CFALGS=""
 XMLRPC_LIBS=""
 fi
 AC_SUBST(XMLRPC_LIBS)
 AC_SUBST(XMLRPC_CFLAGS)

dnl XML2
AM_PATH_XML2(2.2.6,, AC_MSG_ERROR([xml-2.2.6 or later is required]))

dnl json-c
PKG_CHECK_MODULES(JSONC, json-c, [:], [:])
if test "x$JSONC_LIBS" != "x"; then
	CFLAGS="$CFLAGS $JSONC_CFLAGS"
	LIBS="$LIBS $JSONC_LIBS"
	AC_MSG_CHECKING([if json-c is available])
	AC_MSG_RESULT(yes)
else
        AC_MSG_ERROR([json-c is required.])
fi

dnl libm
AC_SEARCH_LIBS([cos], [m], [], [AC_MSG_ERROR([unable to find the cos() function])])


dnl Maintainer Mode
AC_ARG_WITH(maintain,
		 AC_HELP_STRING([--with-maintain],
		            [For Maintainer]),
			    [use_maint=yes])
             

if test x"${use_maint}" = "xyes"; then
	DISABLE_DEPRECATED="-g -DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE"
else
	DISABLE_DEPRECATED=""
fi
AC_SUBST(DISABLE_DEPRECATED)

dnl Check for OpenSSL
AC_ARG_ENABLE(ssl,
	[  --disable-ssl           Disable SSL support using OpenSSL],
	[ac_cv_enable_ssl=$enableval], [ac_cv_enable_ssl=yes])
AC_MSG_CHECKING([whether to use OpenSSL])
if test $ac_cv_enable_ssl = yes; then
	AC_MSG_RESULT(yes)
	dnl First check with pkg-config
	PKG_CHECK_MODULES(OPENSSL, openssl, [:], [:])
	if test "x$OPENSSL_LIBS" != "x"; then
		CFLAGS="$CFLAGS $OPENSSL_CFLAGS"
		LIBS="$LIBS $OPENSSL_LIBS"
		AC_MSG_CHECKING([if openssl is available])
		AC_MSG_RESULT(yes)
		AC_DEFINE(USE_SSL, 1, Define if you use OpenSSL to support SSL.)
	else
                AC_MSG_ERROR([OpenSSL is required. Configure with --disable-ssl if you do not need HTTPS access.])
	fi
else
	AC_MSG_RESULT(no)
fi



AC_OUTPUT([
Makefile
version.h
hskymon_res.rc
libnova/Makefile
std_data/Makefile
debian/Makefile
sample/Makefile
])


dnl Output the configuration summary
echo ""
echo "####################"
if test x"${use_win32}" = "xyes"; then
echo " $PACKAGE $VERSION build for Win32 "
else
if test x"${with_xmlrpc}" = "xyes"; then
echo " $PACKAGE $VERSION build for Gen2 "
else
echo " $PACKAGE $VERSION"
fi
fi
echo "####################"
echo ""
echo ""
echo "Configure finished, type 'make' to build."
