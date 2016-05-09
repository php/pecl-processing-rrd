dnl config.m4 for extension php_rrd
dnl Comments in this file start with the string 'dnl'.

PHP_ARG_WITH(rrd, for rrdtool support,
[  --with-rrd              Include rrdtool support (requires rrdtool >= 1.3.0)], yes)

AC_ARG_WITH(rrd-binary,
[AC_HELP_STRING([--with-rrd-binary][=PATH], [rrd binary dir path, mostly for testing (default=$PATH)])],
[AC_PATH_PROG(RRDTOOL_BIN, rrdtool, no, $withval)],
[AC_PATH_PROG(RRDTOOL_BIN, rrdtool, no, $PATH)])

AC_SUBST(RRDTOOL_BIN)
if test -f $srcdir/tests/rrdtool-bin.inc.in; then
  AC_OUTPUT(tests/rrdtool-bin.inc)
  AC_OUTPUT(tests/data/Makefile)
fi

if test "$PHP_RRD" != "no"; then
	AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
	AC_MSG_CHECKING(for librdd)
	if test -x "$PKG_CONFIG" && $PKG_CONFIG --exists librrd && $PKG_CONFIG librrd --atleast-version 1.3.0; then
		AC_MSG_RESULT(found)
		LIBRRD_CFLAGS=`$PKG_CONFIG librrd --cflags`
		LIBRRD_LDFLAGS=`$PKG_CONFIG librrd --libs`

		PHP_EVAL_LIBLINE($LIBRRD_LDFLAGS, RRD_SHARED_LIBADD)
		PHP_EVAL_INCLINE($LIBRRD_CFLAGS)
    AC_DEFINE(HAVE_RRDTOOL, 1, [ ])
	else
		AC_MSG_ERROR(pkgconfig and librrd in version >= 1.3.0 must be installed)
	fi

  dnl rrd_lastupdate_r available in 1.4.0+
  AC_CHECK_LIB([rrd], [rrd_lastupdate_r],
	[ 
		AC_DEFINE(HAVE_RRD_LASTUPDATE_R, 1, [ ])
	], , [$LIBRRD_LDFLAGS])


  dnl rrdc_disconnect available in 1.4.0+
  AC_CHECK_LIB([rrd], [rrdc_disconnect],
	[ 
		AC_DEFINE(HAVE_RRDC_DISCONNECT, 1, [ ])
	], , [$LIBRRD_LDFLAGS])

  PHP_NEW_EXTENSION(rrd, rrd.c rrd_graph.c rrd_create.c rrd_update.c rrd_info.c, $ext_shared)
  PHP_SUBST(RRD_SHARED_LIBADD)
fi
