#
# LX_SPRNG ([default-location])
#  ------------------------------------------------------------------------
# Tests for presence of SPRNG parallel random number library and its 
# headers.
#
AC_DEFUN([LX_SPRNG],
[
  AC_LANG_PUSH([C++])
  AC_ARG_WITH([sprng],
              AS_HELP_STRING([--with-sprng=<dir>],
                             [Path to the installation directory of Scalable Parallel Random Number Generator Library.]),
              [#LX_LIB_SUBST(sprng, [init_rng_simple_mpi], SPRNG, [$withval/lib])
               if [[ "x$have_sprng" != xno ]]; then
                 LX_HEADER_SUBST(sprng, [sprng_cpp.h], SPRNG, [-I $withval/include])
               fi
  if [[ "x$have_sprng" = xyes ]]; then
    SPRNG_LDFLAGS="-L$withval/lib -lsprng"
    AC_SUBST(SPRNG_LDFLAGS)
  fi
              ],
              [#LX_LIB_SUBST(sprng, finit_rng, SPRNG, [$1/lib])
               if [[ "x$have_sprng" != xno ]]; then
                 LX_HEADER_SUBST(sprng, [sprng_cpp.h], SPRNG, [-I $1/include])
               fi
  if [[ "x$have_sprng" = xyes ]]; then
    SPRNG_LDFLAGS="-L$withval/lib -lsprng"
    AC_SUBST(SPRNG_LDFLAGS)
  fi
              ]
  )
  AC_LANG_POP
])
