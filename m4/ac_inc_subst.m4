
dnl
dnl AC_INC_SUBST (name, header, NAME, includes)
dnl  ------------------------------------------------------------------------
dnl This tests for the presence of a header file, given its name, and a directory
dnl to search.  If found, it uses AC_SUBST to export NAME_CPPFLAGS for the header.
dnl Standard var CPPFLAGS is unmodified.  If the header is not found, then have_name
dnl is set to "no".
dnl
AC_DEFUN([AC_HEADER_SUBST],
[
  $3_CPPFLAGS="$4"
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$$3_CPPFLAGS $CPPFLAGS"

  AC_CHECK_HEADER([$2],
                  [have_$1=yes],
                  [AC_MSG_NOTICE([Couldn't find $2.])
                   have_$1=no])
  AC_SUBST($3_CPPFLAGS)
  CPPFLAGS="$OLD_CPPFLAGS"
])

