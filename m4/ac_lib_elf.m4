#
# AC_LIB_ELF ([default-location])
#  ------------------------------------------------------------------------
# Tests for presence of libelf, adding libiberty only if necessary.
#
AC_DEFUN([AC_LIB_ELF],
[
  AC_ARG_WITH([elf],
              AS_HELP_STRING([--with-elf=<dir>],
                             [Path to the installation directory of libelf.]),
              [AC_LIB_SUBST(elf, elf_end, ELF, [$withval/lib], [-lelf])],
              [AC_LIB_SUBST(elf, elf_end, ELF, [$1/lib],       [-lelf])])
])

