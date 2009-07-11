#
# AC_AMPL ([default-location])
#  ------------------------------------------------------------------------
# Tests for presence of libampl and its headers.
#
AC_DEFUN([AC_AMPL],
[
  AC_LANG_PUSH([C++])
  AC_ARG_WITH([ampl],
              AS_HELP_STRING([--with-ampl=<dir>],
                             [Path to the installation directory of AMPL sampling library.]),
              [AC_LIB_SUBST(ampl, ampl_init, AMPL, [$withval/lib])
               if [[ "x$have_ampl" != xno ]]; then
                 AC_HEADER_SUBST(ampl, [ampl/interface/AMPLInterface.h], AMPL, [-I $withval/include])
               fi
              ],
              [AC_LIB_SUBST(ampl, ampl_init, AMPL, [$1/lib])
               if [[ "x$have_ampl" != xno ]]; then
                 AC_HEADER_SUBST(ampl, [ampl/interface/AMPLInterface.h], AMPL, [-I $1/include])
               fi
              ]
  )
  AC_LANG_POP
])

