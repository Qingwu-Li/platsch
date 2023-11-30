AC_PREREQ([2.69])
AC_INIT([platsch], [2019.12.0], [oss-tools@pengutronix.de])
AC_CONFIG_SRCDIR([platsch.c])
AM_INIT_AUTOMAKE([foreign dist-xz])

AC_PROG_CC
AC_PROG_MAKE_SET

PKG_CHECK_MODULES([LIBDRM], [libdrm >= 2.4.112])

AC_ARG_ENABLE([cairo],
  [AS_HELP_STRING([--disable-cairo],
    [Enable CAIRO support @<:@default=check@:>@])],
  [],
  [enable_cairo=check])

AS_IF([test "$enable_cairo" != "no"],
  [PKG_CHECK_MODULES([CAIRO],
    [cairo >= 1.0],
    [enable_cairo=yes
     AC_DEFINE(HAVE_CAIRO)],
    [AS_IF([test "$enable_cairo" = "yes"],
      [AC_MSG_ERROR([cairo required, but not found.])],
      [enable_cairo=no])
    ])
  ])

AM_CONDITIONAL([ENABLE_CAIRO], [test "$enable_cairo" = "yes"])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
