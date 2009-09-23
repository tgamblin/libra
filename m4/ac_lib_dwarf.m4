#
# AC_LIB_DWARF ([default-location])
#  ------------------------------------------------------------------------
# Tests for presence of libdwarf.
#
AC_DEFUN([AC_LIB_DWARF],
[
  AC_ARG_WITH([dwarf],
              AS_HELP_STRING([--with-dwarf=<dir>],
                             [Path to the installation directory of libdwarf.]),
              [AC_LIB_SUBST(dwarf, dwarf_attr, DWARF, [$withval/lib], [-ldwarf $ELF_LDFLAGS])],
              [AC_LIB_SUBST(dwarf, dwarf_attr, DWARF, [$1],           [-ldwarf $ELF_LDFLAGS])]
  )
])

