
dnl
dnl LX_LIB_XML2 ([default-location])
dnl  ------------------------------------------------------------------------
dnl Tests for presence of libxml2 using AC_LIB_SUBST
dnl
AC_DEFUN([LX_LIB_XML2],
[
  AC_ARG_WITH([xml2],
              AS_HELP_STRING([--with-xml2=<dir>],
                             [Path to the installation directory of libxml2.]),
              [LX_LIB_SUBST(xml2, attribute, XML2, [$withval/lib])],
              [LX_LIB_SUBST(xml2, attribute, XML2, [$1])])
])
