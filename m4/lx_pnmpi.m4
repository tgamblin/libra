#################################################################################################
# Copyright (c) 2010, Lawrence Livermore National Security, LLC.  
# Produced at the Lawrence Livermore National Laboratory  
# Written by Todd Gamblin, tgamblin@llnl.gov.
# LLNL-CODE-417602
# All rights reserved.  
# 
# This file is part of Libra. For details, see http://github.com/tgamblin/libra.
# Please also read the LICENSE file for further information.
# 
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
# 
#  * Redistributions of source code must retain the above copyright notice, this list of
#    conditions and the disclaimer below.
#  * Redistributions in binary form must reproduce the above copyright notice, this list of
#    conditions and the disclaimer (as noted below) in the documentation and/or other materials
#    provided with the distribution.
#  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
#    or promote products derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
# LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#################################################################################################

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
