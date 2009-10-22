#
# LX_PARADYN_TOOLS
#  ------------------------------------------------------------------------
# Tests for everything needed for paradyn tools API: dwarf, xml2, symtabAPI,
# and DynStackwalkerAPI.  Sets have_symtab, have_stackwalk if either is found.
# Will also set the following variables for different pieces of the SW API 
# infrastucture:
# have_stackwalk, have_symtabAPI, have_common
#
AC_DEFUN([LX_PARADYN_TOOLS],
[
  AC_ARG_WITH(
     [paradyn],
     AS_HELP_STRING([--with-paradyn=<dir>], 
                    [Path to the platform-specific installation directory of ParaDyn tools. Includes, etc. are inferred.]),
  [echo "checking for ParaDyn tools..."
   if [[ -d "$withval" ]]; then
     if [[ -d "$withval/include" ]]; then
         paradyn_home="$withval"
         paradyn_include="$paradyn_home/include"
         paradyn_lib="$paradyn_home/lib"
     else
         paradyn_home=`echo ${withval} | ${SED} 's_/[[^/]]*\$__'`    
         paradyn_platform=`echo ${withval} | ${SED} 's_.*\/\([[^/]]*\)$_\1_'`
         paradyn_include="$paradyn_home/include"
         paradyn_lib="$paradyn_home/$paradyn_platform/lib"
     fi

     if [[ -d "$paradyn_include" ]]; then
       if [[ "x$paradyn_platform" != "xppc32_bgcompute" ]]; then
         LX_LIB_ELF([$paradyn_lib])
         LX_LIB_DWARF([$paradyn_lib])
         LX_LIB_XML2([$paradyn_lib])
       fi

       # Adding boost headers here, just in case we need them (current version of
       # Stackwalker wants them, but Matt tells me it's not a permanent dependence)
       paradyn_headers="-I$paradyn_include ${BOOST_CPPFLAGS}"
       LX_HEADER_SUBST(stackwalk, [walker.h],   SW,     [$paradyn_headers])
       LX_HEADER_SUBST(symtabAPI, [Symtab.h],   SYMTAB, [$paradyn_headers])
       LX_HEADER_SUBST(common,    [dyntypes.h], COMMON, [$paradyn_headers])

       if [[ "x${SW}${SYMTAB}${COMMON}" != x ]]; then 
         PARADYN_CPPFLAGS="$paradyn_headers"
         AC_SUBST(PARADYN_CPPFLAGS)
       fi

       LX_LIB_SUBST(iberty_pic, cplus_demangle, IBERTY, [$paradyn_lib])
       if [[ "x$have_iberty_pic" = "xno" ]]; then
           LX_LIB_SUBST(iberty, cplus_demangle, IBERTY, [$paradyn_lib])
       fi

       common_libs="-lcommon $XML2_LDFLAGS $DWARF_LDFLAGS $IBERTY_LDFLAGS"
       LX_LIB_SUBST(common, _init, COMMON, [$paradyn_lib], [$common_libs])

       symtab_libs="-lsymtabAPI $COMMON_LDFLAGS"
       LX_LIB_SUBST(symtabAPI, _init, SYMTAB, [$paradyn_lib], [$symtab_libs])

       # By the third lib in the chain of dependencies here, this is kind of nasty.
       # Need this if statement for the case where we have common but not symtab.
       # Consider restructuring this if we get more stackwalk libs.
       if [[ "x$have_symtabAPI" = xyes ]]; then
           stackwalk_libs="-lstackwalk $SYMTAB_LDFLAGS"
       else 
           stackwalk_libs="-lstackwalk $COMMON_LDFLAGS"
       fi
       LX_LIB_SUBST(stackwalk, _init, SW, [$paradyn_lib], [$stackwalk_libs])         

     else
       echo "couldn't find ParaDyn headers in $paradyn_include."
       have_common=no
       have_symtabAPI=no
       have_stackwalk=no
     fi
   else 
     echo "couldn't find ParaDyn tools in $withval."
     have_common=no
     have_symtabAPI=no
     have_stackwalk=no
   fi     

   if test "x$have_common" != xno;    then have_common=yes;    fi
   if test "x$have_symtabAPI" != xno; then have_symtabAPI=yes; fi
   if test "x$have_stackwalk" != xno; then have_stackwalk=yes; fi
  ],
  [echo "path to ParaDyn tools not provided."
   have_common=no
   have_symtabAPI=no
   have_stackwalk=no
  ])
])
