#
# AC_SPRNG ([default-location])
#  ------------------------------------------------------------------------
# Tests for presence of SPRNG and its headers.
#
AC_DEFUN([AC_SPRNG],
[
  AC_LANG_PUSH([C++])
  AC_ARG_WITH([sprng],
              AS_HELP_STRING([--with-sprng=<dir>],
                             [Path to the installation directory of Scalable Parallel Random Number Generator Library.]),
              [AC_LIB_SUBST(sprng, finit_rng, SPRNG, [$withval/lib])
               if [[ "x$have_sprng" != xno ]]; then
                 AC_HEADER_SUBST(sprng, [sprng_cpp.h], SPRNG, [-I $withval/include])
               fi
              ],
              [AC_LIB_SUBST(sprng, finit_rng, SPRNG, [$1/lib])
               if [[ "x$have_sprng" != xno ]]; then
                 AC_HEADER_SUBST(sprng, [sprng_cpp.h], SPRNG, [-I $1/include])
               fi
              ]
  )
  AC_LANG_POP
])

