#!/bin/sh
#
# Usage: wrap_vtk.sh <vtkWrapPython> <vtkWrapPythonInit> <srcdir> <initfile> <pyfiles ...>
# 
# This script takes at least 4 parameters:
#   vtkWrapPython:         path to vtkWrapPython in vtk installation
# 
#   vtkWrapPythonInit:     path to vtkWrapPythonInit in vtk installation
#
#   srcdir                 Source directory where headers to be wrapped are located (for when
#                          there's a separate build directory).
#
#   initfile:              Name of C file that should contain the library initialization routine.
#                          This will be created using vtkWrapPythonInit and it will be passed
#                          the names of all the files in <pyfiles>.
# 
#   pyfiles:               These should be of the form vtkClassNamePython.C.  This script will
#                          create them by wrapping the corresponding vtkClassName.h file, if it
#                          exists.  If not this will throw an error.
#

VTK_WRAP_PYTHON=$1;      shift 1
VTK_WRAP_PYTHON_INIT=$1; shift 1
SRC_DIR=$1;              shift 1
INIT_FILE=$1;            shift 1

#
# Figure out what the name of the library is and create an init file for it.
#
libname=`echo $INIT_FILE | sed 's/lib\(.*\)PythonInit.C/\1/'`
init_file="lib${libname}_init.txt"
echo $libname > ${init_file}

#
# Touch the hints file in case it doesn't exist already.
#
HINT_FILE="${SRC_DIR}/hints"
touch ${HINT_FILE}

#
# Wrap each of the python files with vtkWrapPython and append its name to the init file
#
for file in $@; do
    classname=`echo $file | sed 's/Python.C//'`
    hfile="${SRC_DIR}/${classname}.h"
    if [ ! -e "$hfile" ] ; then
        echo "Error wrapping $hfile.  '$hfile' does not exist."
        exit 1
    fi

    # Wrap only if the h file is newer than the python wrapper file
    if [ ! -e "$file" -o "$hfile" -nt "$file" ]; then
        cmd="${VTK_WRAP_PYTHON} $hfile ${HINT_FILE} 1 $file"
        echo $cmd
        $cmd || (echo "Error wrapping $hfile."; exit 1)
    fi

    echo $classname >> $init_file
done

#
# Finally wrap the init file using vtkWrapPythonInit and the init_file we made.
#
cmd="${VTK_WRAP_PYTHON_INIT} ${init_file} ${INIT_FILE}"
echo $cmd
$cmd || (echo "Error creating ${INIT_FILE}."; exit 1)
