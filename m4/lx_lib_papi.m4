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
# LX_LIB_PAPI ([default-location])
#  ------------------------------------------------------------------------
# Tests for presence of PAPI library and headers.
#
AC_DEFUN([LX_LIB_PAPI],
[
  AC_ARG_WITH([papi],
              AS_HELP_STRING([--with-papi=<dir>],
                             [Path to the installation directory of PAPI.]),
              [papidir=$withval
               LX_HEADER_SUBST(papi, [papi.h], PAPI, [-I$papidir/include])           
               if test -d "$papidir/lib64"; then
                 papilibdir="$papidir/lib64"
               elif test -d "$papidir/lib"; then
                 papilibdir="$papidir/lib"
               fi
               LX_LIB_SUBST(papi, PAPI_library_init, PAPI, [$papilibdir], [-lpapi])
              ],
              [LX_HEADER_SUBST(papi, [papi.h], PAPI, [])
               LX_LIB_SUBST(papi, PAPI_library_init, PAPI, [$1], [-lpapi])])

  if test "x$have_papi" != xno; then
    have_papi=yes
  fi
])

