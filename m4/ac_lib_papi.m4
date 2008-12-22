dnl
dnl AC_LIB_PAPI ([default-location])
dnl  ------------------------------------------------------------------------
dnl Tests for presence of PAPI library and headers.
dnl
AC_DEFUN([AC_LIB_PAPI],
[
  AC_ARG_WITH([papi],
              AS_HELP_STRING([--with-papi=<dir>],
                             [Path to the installation directory of PAPI.]),
              [papidir=$withval
               AC_HEADER_SUBST(papi, [papi.h], PAPI, [-I$papidir/include])           
               if test -d "$papidir/lib64"; then
                 papilibdir="$papidir/lib64"
               elif test -d "$papidir/lib"; then
                 papilibdir="$papidir/lib"
               fi
               AC_LIB_SUBST(papi, PAPI_library_init, PAPI, [$papilibdir], [-lpapi])
              ],
              [AC_HEADER_SUBST(papi, [papi.h], PAPI, [])
               AC_LIB_SUBST(papi, PAPI_library_init, PAPI, [$1], [-lpapi])])

  if test "x$have_papi" != xno; then
    have_papi=yes
  fi
])

