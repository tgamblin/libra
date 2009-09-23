#
#  AC_LIB_SUBST (name, symbol, NAME, [dir], [libs])
#  ------------------------------------------------------------------------
#  This tests for the presence of a library, given a libname and a symbol.
#  If found, it uses AC_SUBST to export NAME_LDFLAGS for that library.  
#  Standard vars like LIBS, LDFLAGS, etc. are unmodified.  If the library
#  is not found, then have_name is set to "no".
#  
#  If <dir> is provided, this will add -L<dir> to the link line.
#
#  You can optionally set libs to the ACTUAL "-lfoo -lbar -lbaz" part of the link
#  line, if it's not just -lname that you want to use.
#
AC_DEFUN([AC_LIB_SUBST],
[
  # If dir param is there, add it to the link line.
  if test "x$4" != x; then
    $3_LDFLAGS="-L$4"
  else
    $3_LDFLAGS=""
  fi

  # If extra libraries are supplied, add those too.
  if test "x$5" != "x"; then
    $3_LDFLAGS="$$3_LDFLAGS $5"
  else
    $3_LDFLAGS="$$3_LDFLAGS -l$1"
  fi

  # search the link line for -Lwhatever and add rpath args for each one.
  RPATHS=""
  for elt in $3_LDFLAGS; do
     if echo $elt | grep -q '^-L'; then
        path=`echo $elt | sed 's/^-L//'`
        rpath="-Wl,-rpath -Wl,$path"

        if [[ -z "$RPATHS" ]]; then
            RPATHS="$rpath"
        else
            RPATHS="$RPATHS $rpath"
        fi
     fi
  done
  $3_LDFLAGS="$3_LDFLAGS $RPATHS"


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

