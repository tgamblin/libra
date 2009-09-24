#
#  LX_PNMPI ([action-if-found], [action-if-not-found])
#  ------------------------------------------------------------------------
#  Adds --with-pnmpi=<dir> to the configure help message.  
#  If not found , have_pnmpi is set to "no".  If found, exports 
#  PNMPI_LDFLAGS and PNMPI_CPPFLAGS with AC_SUBST.
#
AC_DEFUN([LX_PNMPI],
[
    AC_ARG_WITH([pnmpi],
        # Help string
        AS_HELP_STRING([--with-pnmpi=<dir>], 
                       [Path to the location of PnMPI tools.  If provided, builds PnMPI modules.]),
        # --with-pnmpi provided
        [
             if test "x$withval" = xno; then
                 have_pnmpi=no    # explicitly disabled
                 $2
             else
                 pnmpi_home="$withval"
                 
                 # Export location of wrapper generator, even if we don't have pnmpi.
                 # Figure out a better way to do this.
                 if [[ -x "$pnmpi_home/bin/wrappergen" ]]; then
                     # This is the standard gnu install location of things
                     PNMPI_WRAPPERGEN="$pnmpi_home/bin/wrappergen"
                     PNMPI_WRAPPERGEN="$PNMPI_WRAPPERGEN -p $pnmpi_home/share/pnmpi/mpi_pnmpi_proto"
                     PNMPI_WRAPPERGEN="$PNMPI_WRAPPERGEN -f $pnmpi_home/share/pnmpi/mpi_pnmpi_fct"

                     PNMPI_PATCH="$pnmpi_home/bin/patch"                     
                     pnmpi_include="$pnmpi_home/include"
                     pnmpi_lib="$pnmpi_home/lib"
                     pnmpidir='${prefix}/lib'

                 elif [[ -x "$pnmpi_home/wrappergen/wrappergen" ]]; then
                     # If we find stuff here, it's all in some source directory
                     PNMPI_WRAPPERGEN="$withval/wrappergen/wrappergen"
                     PNMPI_WRAPPERGEN="$PNMPI_WRAPPERGEN -p $pnmpi_home/wrapper/mpi_pnmpi_proto"
                     PNMPI_WRAPPERGEN="$PNMPI_WRAPPERGEN -f $pnmpi_home/wrapper/mpi_pnmpi_fct"

                     if test "x$SYS_TYPE" == "x"; then
                         SYS_TYPE=`uname -s`
                     fi
                     PNMPI_PATCH="$pnmpi_home/patch/patch"
                     pnmpi_include="$pnmpi_home/include"
                     pnmpi_lib="$pnmpi_home/lib/${SYS_TYPE}"
                     pnmpidir="$pnmpi_lib"
                 else
                     AC_MSG_ERROR([Couldn't find PnMPI wrappergen!])
                 fi
                 AC_SUBST(PNMPI_WRAPPERGEN)
                 AC_SUBST(PNMPI_PATCH)
                 AC_SUBST(pnmpidir)
                  
                 # Export PNMPI_CPPFLAGS and PNMPI_LDFLAGS
                 LX_HEADER_SUBST(pnmpi, pnmpi.h, PNMPI, [-I$pnmpi_include])
                 LX_LIB_SUBST(pnmpi, PNMPI_Service_RegisterService, PNMPI, [$pnmpi_lib])
                  
                 if test "x$have_pnmpi" != xno; then
                     $1
                 else
                     $2
                 fi
             fi
        ],
        # --with-pnmpi not provided
        [
             AC_MSG_NOTICE([Building without PnMPI tool modules.])
             have_pnmpi=no
        ]
    )
])
