dnl 
dnl This is a safe version of AC_CHECK_LIB, which will NOT append to 
dnl $LIBS.  The default version will add the library to the link line
dnl for every target, while this just makes sure that you have the lib.
dnl 
AC_DEFUN([AC_SAFE_CHECK_LIB],
[
    OLD_LIBS="$LIBS"
    AC_CHECK_LIB($1,$2,$3,$4)
    LIBS="$OLD_LIBS"
])

