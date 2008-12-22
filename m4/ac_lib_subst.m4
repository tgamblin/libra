
dnl
dnl AC_LIB_SUBST (name, symbol, NAME, dir, [libs])
dnl  ------------------------------------------------------------------------
dnl This tests for the presence of a library, given a libname and a symbol.
dnl If found, it uses AC_SUBST to export NAME_LDFLAGS for that library.  
dnl Standard vars like LIBS, LDFLAGS, etc. are unmodified.  If the library
dnl is not found, then have_name is set to "no".
dnl 
dnl You can optionally set libs to the ACTUAL "-lfoo -lbar -lbaz" part of the link
dnl line, if it's not just -lname that you want to use.
dnl
AC_DEFUN([AC_LIB_SUBST],
[
  if test "x$4" != x; then
    $3_LDFLAGS="-L$4"
  else
    $3_LDFLAGS=""
  fi

  if test "x$5" != "x"; then
    $3_LDFLAGS="$$3_LDFLAGS $5"
  else
    $3_LDFLAGS="$$3_LDFLAGS -l$1"
  fi

  OLD_LDFLAGS="$LDFLAGS"
  LDFLAGS="$$3_LDFLAGS $LDFLAGS"
  
  AC_SAFE_CHECK_LIB([$1], [$2],
                    [have_$1=yes],
                    [AC_MSG_NOTICE([Couldn't find lib$1.])
                     $3_LDFLAGS=""
                     have_$1=no])

  AC_SUBST($3_LDFLAGS)
  LDFLAGS="$OLD_LDFLAGS"
])

