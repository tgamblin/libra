#
#  AC_OPTIONS_VTK
#  ------------------------------------------------------------------------
#  Adds the --with-vtk=path option to the configure options
#
AC_DEFUN([AC_OPTIONS_VTK],
[ AC_ARG_WITH([vtk], 
              [AC_HELP_STRING([--with-vtk],  [The prefix where VTK is installed (default is /usr/local)])],
              [with_vtk=$withval], 
	      [with_vtk="/usr/local"])
	      
  AC_ARG_WITH([vtk-version], 
              [AC_HELP_STRING([--with-vtk-version],
    	                      [VTK's include directory name is vtk-suffix, e.g. vtk-5.0. What's the suffix? (Finds highest available by default).  NOTE YOU MUST INCLUDE THE DASH.])],
              [vtk_suffix=-$withval], 
              [vtk_include_dir="$with_vtk/include"
               vtk_suffix=`ls $vtk_include_dir | sort -r | head -1 | sed 's/^vtk//'`
               if test "x$vtk_suffix" = "x"; then
                 with_vtk=no
                 echo "Couldn't find VTK include directory."
               fi
              ])
])


#
#  AC_PATH_VTK([minimum-version], [action-if-found], [action-if-not-found])
#  ------------------------------------------------------------------------
#
#  NOTE: [minimum-version] must be in the form [X.Y.Z]
#
AC_DEFUN([AC_PATH_VTK],
[
        # do we want to check for VTK ?
        if [[ $with_vtk = "yes" ]]; then
            # in case user wrote --with-vtk=yes
            with_vtk="/usr/local"
        fi

        if [[ $with_vtk != "no" ]]; then
            VTK_PREFIX="$with_vtk"
            AC_CHECK_FILE([$VTK_PREFIX/include/vtk$vtk_suffix/vtkCommonInstantiator.h], [vtkFound="OK"])
            AC_MSG_CHECKING([if VTK is installed in $VTK_PREFIX])
            if [[ -z "$vtkFound" ]]; then
                # not found !
                AC_MSG_RESULT([no])
                $3
            else
                # found !
                AC_MSG_RESULT([yes])

                # these are the VTK libraries of a default build
                VTK_DEFAULT_LIBS="-lvtkCommon -lvtkDICOMParser -lvtkHybrid -lvtkexpat -lvtkmetaio -lvtksqlite -lvtkexoIIc -lvtkNetCDF -lvtksys -lvtkFiltering -lvtkfreetype -lvtkftgl -lvtkGraphics -lvtkImaging -lvtkIO -lvtkverdict -lvtkjpeg -lvtkpng -lvtkRendering -lvtktiff -lvtkzlib"

                # set VTK c,cpp,ld flags
                VTK_CPPFLAGS="-I$VTK_PREFIX/include/vtk$vtk_suffix"
                VTK_LIB_PATH="$VTK_PREFIX/lib/vtk$vtk_suffix"
                VTK_LIBS="-L$VTK_LIB_PATH -Wl,-rpath -Wl,$VTK_LIB_PATH $VTK_DEFAULT_LIBS"

                # now, eventually check version
                if [[ -n "$1" ]]; then

                    # the version of VTK we need:
                    maj=`echo $1 | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
                    min=`echo $1 | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
                    rel=`echo $1 | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
                    AC_MSG_CHECKING([if VTK version is at least $maj.$min.$rel])

                    # in order to be able to compile the following test program, we need to add
                    # to the current flags, the VTK settings...
                    OLD_CPPFLAGS=$CPPFLAGS
                    OLD_LIBS=$LIBS

                    CPPFLAGS="$VTK_CPPFLAGS $CFLAGS"
                    LIBS="$VTK_LIBS $LIBS"

                    # check if the installed VTK is greater or not
                    AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
                         [
                            #include <vtkConfigure.h>
                            #include <stdio.h>
                         ],
                         [
                             printf("VTK version is: %d.%d.%d", VTK_MAJOR_VERSION, VTK_MINOR_VERSION, VTK_BUILD_VERSION);
                             #if VTK_MAJOR_VERSION < $maj
                                 #error Installed VTK is too old !
                             #endif
                             #if VTK_MINOR_VERSION < $min
                                 #error Installed VTK is too old !
                             #endif
                             #if VTK_BUILD_VERSION < $rel
                                 #error Installed VTK is too old !
                             #endif
                         ])
                     ], [vtkVersion="OK"])

                    if [[ "$vtkVersion" = "OK" ]]; then
                        AC_SUBST(VTK_CPPFLAGS)
                        AC_SUBST(VTK_LIBS)
                        AC_MSG_RESULT([yes])
                        $2
                    else
                        AC_MSG_RESULT([no])
                  			$3
                   fi
                   
                   #Reset CPPFLAGS, etc. to their original state after we are done with the test.
                   CPPFLAGS="$OLD_CPPFLAGS"
                   LIBS="$OLD_LIBS"
               else
                    # if we don't have to check for minimum version (because the user did not set that option),
                    # then we can execute here the block action-if-found
                    AC_SUBST(VTK_CPPFLAGS)
                    AC_SUBST(VTK_LIBS)
                    $2
               fi

               VTK_WRAP_PYTHON="$VTK_PREFIX/bin/vtkWrapPython"
               VTK_WRAP_PYTHON_INIT="$VTK_PREFIX/bin/vtkWrapPythonInit"
               AC_CHECK_FILE([$VTK_WRAP_PYTHON], [], 
                             [AC_MSG_ERROR([You must build VTK with Python wrappings!])])
               AC_CHECK_FILE([$VTK_WRAP_PYTHON_INIT], [], 
                             [AC_MSG_ERROR([You must build VTK with Python wrappings!])])
               AC_SUBST(VTK_WRAP_PYTHON)
               AC_SUBST(VTK_WRAP_PYTHON_INIT)
           fi
       else
           $3
       fi
])


