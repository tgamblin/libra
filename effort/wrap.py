#!/usr/bin/env python
#
# Usage: wrap.py [-c mpicc_name] [-o file] wrapper.w [...]
#
# Python script for creating PMPI wrappers. Roughly follows the syntax of 
# the Argonne PMPI wrapper generator, with some enhancements.
#
# by Todd Gamblin
#
import tempfile, getopt, subprocess, sys, re, StringIO

# Default name for the MPI compiler
mpicc = 'mpicc'

# Don't print fortran wrappers by default
output_fortran_wrappers = False

# Don't print reentry guards by default
output_guards = False

# Possible function return types to consider, used for declaration parser.
# In general, all MPI calls we care about return int.  We include double
# to grab MPI_Wtick and MPI_Wtime, but we'll ignore the f2c and c2f calls 
# that return MPI_Datatypes and other such things.
rtypes = ['int', 'double' ]

# If we find these strings in a declaration, exclude it from consideration.
exclude_strings = [ "c2f", "f2c" ]

# Regular expressions for start and end of declarations in mpi.h. These are
# used to get the declaration strings out for parsing with formal_re below.
begin_decl_re = re.compile("(" + "|".join(rtypes) + ")\s+(MPI_\w+)\(")
exclude_re =    re.compile("|".join(exclude_strings))
end_decl_re =   re.compile("\).*\;")

# Regular Expression for splitting up args. Matching against this 
# returns three groups: type info, arg name, and array info
formal_re = re.compile(
    "\s*(" +                       # Start type
    "(?:const)?\s*" +              # Initial const
    "\w+"                          # Type name (note: doesn't handle 'long long', etc. right now)
    ")\s*(" +                      # End type, begin pointers
    "(?:\s*\*(?:\s*const)?)*" +    # Look for 0 or more pointers with optional 'const'
    ")\s*"                         # End pointers
    "(?:(\w+)\s*)?" +              # Argument name. Optional.
     "(\[.*\])?\s*$"               # Array type.  Also optional. Works for multidimensions b/c it's greedy.
    )

# Fortran wrapper suffix
f_wrap_suffix = "_fortran_wrapper"

# Set of MPI Handle types
mpi_handle_types = set(["MPI_Comm", "MPI_Errhandler", "MPI_File", "MPI_Group", "MPI_Info", 
                        "MPI_Op", "MPI_Request", "MPI_Status", "MPI_Datatype", "MPI_Win" ])

# MPI Calls that have array parameters, and mappings from the array positions to the position of its size
mpi_array_calls = {
    "MPI_Startall"           : { 1:0 },
    "MPI_Testall"            : { 1:0, 3:0 },
    "MPI_Testany"            : { 1:0 },
    "MPI_Testsome"           : { 1:0, 4:0 },
    "MPI_Type_create_struct" : { 3:0 },
    "MPI_Type_get_contents"  : { 6:1 },
    "MPI_Type_struct"        : { 3:0 },
    "MPI_Waitall"            : { 1:0, 2:0 },
    "MPI_Waitany"            : { 1:0 },
    "MPI_Waitsome"           : { 1:0, 4:0 }
}

# Returns MPI_Blah_[f2c,c2f] prefix for a handle type
def conversion_prefix(handle_type):
    if handle_type == "MPI_Datatype":
        return "MPI_Type"
    else:
        return handle_type

# Map from function name to declaration created from mpi.h.
mpi_functions = {}

# Global table of macro functions, keyed by name.
macros = {}

class Param:
    """Descriptor for formal parameters of MPI functions.  
       Doesn't represent a full parse, only the initial type information, 
       name, and array info of the argument split up into strings.
    """
    def __init__(self, type, pointers, name, array, pos):
        self.type = type               # Name of arg's type (might include things like 'const')
        self.pointers = pointers       # Pointers
        self.name = name               # Formal parameter name (from header or autogenerated)
        self.array = array             # Any array type information after the name
        self.pos = pos                 # Position of arg in declartion
        self.decl = None               # This gets set later by Declaration

    def setDeclaration(self, decl):
        """Needs to be called by Declaration to finish initing the arg."""
        self.decl = decl
        
    def isHandleArray(self):
        """True if this Param represents an array of MPI handle values."""
        return (self.decl.name in mpi_array_calls
                and self.pos in mpi_array_calls[self.decl.name])

    def countParam(self):
        """If this Param is a handle array, returns the Param that represents the count of its elements"""
        return self.decl.args[mpi_array_calls[self.decl.name][self.pos]]
    
    def isHandle(self):
        """True if this Param is one of the MPI builtin handle types."""
        return self.type in mpi_handle_types

    def isStatus(self):
        """True if this Param is an MPI_Status.  MPI_Status is handled differently 
           in c2f/f2c calls from the other handle types.
        """
        return self.type == "MPI_Status"

    def fortranFormal(self):
        """Prints out a formal parameter for a fortran wrapper."""
        # There are only a few possible fortran arg types in our wrappers, since
        # everything is a pointer.
        if self.type == "MPI_Aint" or self.type.endswith("_function"):
            ftype = self.type
        else:
            ftype = "MPI_Fint"

        # Arrays don't come in as pointers (they're passed as arrays)
        # Everything else is a pointer.
        if self.pointers:
            pointers = self.pointers
        elif self.array:
            pointers = ""
        else:
            pointers = "*"
            
        # Put it all together and return the fortran wrapper type here.
        arr = self.array or ''
        return "%s %s%s%s" % (ftype, pointers, self.name, arr)

    def cFormal(self):
        """Prints out a formal parameter for a C wrapper."""
        if not self.type:
            return self.name  # special case for '...'
        else:
            arr = self.array or ''
            pointers = self.pointers or ''
            return "%s %s%s%s" % (self.type, pointers, self.name, arr)

    def __str__(self):
        return self.cFormal()


class Declaration:
    """ Descriptor for simple MPI function declarations.  
        Contains return type, name of function, and a list of args.
    """
    def __init__(self, rtype, name):
        self.rtype = rtype
        self.name = name
        self.args = []

    def addArgument(self, arg):
        arg.setDeclaration(self)
        self.args.append(arg)

    def __iter__(self):
        for arg in self.args: yield arg

    def __str__(self):
        return self.prototype()

    def retType(self):
        return self.rtype

    def argTypeList(self):
        return "(" + ", ".join(map(Param.cFormal, self.args)) + ")"

    def argsNoEllipsis(self):
        return filter(lambda arg: arg.name != "...", self.args)

    def fortranArgTypeList(self):
        formals = map(Param.fortranFormal, self.argsNoEllipsis())
        if self.name == "MPI_Init": formals = []
        return "(%s)" % ", ".join(formals + ["MPI_Fint *ierr"])

    def argList(self):
        names = [arg.name for arg in self.argsNoEllipsis()]
        return "(%s)" % ", ".join(names)

    def fortranArgList(self):
        names = [arg.name for arg in self.argsNoEllipsis()]
        if self.name == "MPI_Init": names = []
        return "(%s)" % ", ".join(names + ["ierr"])

    def prototype(self):
        return "%s %s%s" % (self.retType(), self.name, self.argTypeList())
    
    def fortranPrototype(self, name=None):
        if not name: name = self.name
        return "void %s%s" % (name, self.fortranArgTypeList())
    

types = set()
all_pointers = set()

def enumerate_mpi_declarations(mpicc = "mpicc"):
    """ Invokes mpicc's C preprocessor on a C file that includes mpi.h.
        Parses the output for declarations, and yields each declaration to
        the caller.
    """
    # Create an input file that just includes <mpi.h> 
    tmpfile = tempfile.NamedTemporaryFile('w+b', -1, '.c')
    tmpname = "%s" % tmpfile.name
    tmpfile.write('#include <mpi.h>')
    tmpfile.write("\n")
    tmpfile.flush()

    # Run the mpicc -E on the temp file and pipe the output 
    # back to this process for parsing.
    mpicc_cmd = "%s -E" % mpicc
    try:
        popen = subprocess.Popen("%s %s" % (mpicc_cmd, tmpname), shell=True,
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except IOError:
        print "IOError: couldn't run '" + mpicc_cmd + "' for parsing mpi.h"
        sys.exit(1)
    
    # Parse out the declarations from the MPI file
    mpi_h = popen.stdout
    for line in mpi_h:
        line = line.strip()
        begin = begin_decl_re.search(line)
        if begin and not exclude_re.search(line):
            # Grab return type and fn name from initial parse
            return_type, fn_name = begin.groups()

            # Accumulate rest of declaration (possibly multi-line)
            while not end_decl_re.search(line):
                line += " " + mpi_h.next().strip()

            # Split args up by commas so we can parse them independently
            arg_string = re.search(fn_name + "\s*\((.*)\)", line).group(1)
            arg_list = map(lambda s: s.strip(), arg_string.split(","))

            # Handle functions that take no args specially
            if arg_list == ['void']:
                arg_list = []
            
            # Parse formal parameter descriptors out of args            
            decl = Declaration(return_type, fn_name)
            arg_num = 0
            for arg in arg_list:
                if arg == '...':   # Special case for Pcontrol.
                    decl.addArgument(Param(None, None, '...', None, arg_num))
                else:
                    match = formal_re.match(arg)
                    if not match:
                        print "MATCH FAILED FOR: '" + arg + "' in " + fn_name
                        sys.exit(1)

                    type, pointers, name, array = match.groups()
                    types.add(type)
                    all_pointers.add(pointers)
                    # If there's no name, make one up.
                    if not name: name = "arg_" + str(arg_num)
                        
                    decl.addArgument(Param(type.strip(), pointers, name, array, arg_num))
                arg_num += 1
                    
            yield decl

    error_status = mpi_h.close()
    if (error_status):
        print "Error: Couldn't run '" + mpicc_cmd + "' for parsing mpi.h."
        print "       Process exited with code " + str(error_status)
        sys.exit(1)

    # Do some cleanup once we're done reading.
    tmpfile.close()


# Possible types of Tokens in input.
LBRACE, RBRACE, TEXT = range(3)

class Token:
    """Represents tokens; generated from input by Lexer and fed to parse()."""
    def __init__(self, type, value):
        self.type = type    # Type of token
        self.value = value  # Text value

    def isa(self, type):
        return self.type == type

class Lexer:
    """Lexes a wrapper file and spits out Tokens in order."""
    def __init__(self, lbrace, rbrace):
        self.lbrace = lbrace
        self.rbrace = rbrace
        self.in_tag = False
        self.text = StringIO.StringIO()
    
    def lex_line(self, line):
        length = len(line)
        start = 0

        while (start < length):
            if self.in_tag:
                brace_type, brace = (RBRACE, self.rbrace)
            else:
                brace_type, brace = (LBRACE, self.lbrace)

            end = line.find(brace, start)
            if (end >= 0):
                self.text.write(line[start:end])
                yield Token(TEXT, self.text.getvalue())
                yield Token(brace_type, brace)
                self.text.close()
                self.text = StringIO.StringIO()
                start = end + len(brace)
                self.in_tag = not self.in_tag
            else:
                self.text.write(line[start:])
                start = length

    def lex(self, file):
        for line in file:
            for token in self.lex_line(line):
                yield token

        # Yield last token if there's anything there.
        last = self.text.getvalue()
        if last:
            yield Token(TEXT, last)


def write_c_wrapper(out, decl, return_val, write_body):
    out.write(decl.prototype())
    out.write(" { \n")
    if output_guards:
        out.write("    if (in_wrapper) return P%s%s;\n" % (decl.name, decl.argList()))
        out.write("    in_wrapper = 1;")
    out.write("    int %s = 0;\n" % return_val)

    write_body(out)

    if output_guards:
        out.write("    in_wrapper = 0;\n")
    out.write("    return %s;\n" % return_val)
    out.write("}\n\n")


def write_fortran_binding(out, decl, delegate_name, binding):
    """Outputs a wrapper for a particular fortran binding that delegates to the
       primary Fortran wrapper.
    """
    out.write(decl.fortranPrototype(binding))
    out.write(" { \n")
    out.write("    %s%s;\n" % (delegate_name, decl.fortranArgList()))
    out.write("}\n\n")
    

class FortranDelegation:
    """Class for constructing a call to a Fortran wrapper delegate function.  Provides
       storage for local temporary variables, copies of parameters, callsites for MPI-1 and
       MPI-2, and writebacks to local pointer types.
    """
    def __init__(self, fn_name, return_val):
        self.fn_name = fn_name
        self.return_val = return_val
        self.locals = []
        self.pre = []
        self.post = []

        self.temps = []
        self.copies = []
        self.writebacks = []
        self.actuals = []
        self.mpich_actuals = []

    def addTemp(self, type, name):
        self.temps.append("    %s %s;" % (type, name))

    def addActual(self, actual):
        self.actuals.append(actual)
        self.mpich_actuals.append(actual)
        
    def addActualMPICH(self, actual):
        self.mpich_actuals.append(actual)

    def addActualMPI2(self, actual):
        self.actuals.append(actual)

    def addWriteback(self, stmt):
        self.writebacks.append("    %s" % stmt)

    def addCopy(self, stmt):
        self.copies.append("    %s" % stmt)

    def write(self, out):
        call = "    int %s = %s" % (self.return_val, self.fn_name)
        assert len(self.actuals) == len(self.mpich_actuals)

        out.write("#if (defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */\n")
        out.write("%s(%s);\n" % (call, ", ".join(self.mpich_actuals)))
        out.write("#else /* MPI-2 safe call */\n")
        out.write("\n".join(self.temps))
        out.write("\n".join(self.copies))
        out.write("\n")
        out.write("%s(%s);\n" % (call, ", ".join(self.actuals)))
        out.write("\n".join(self.writebacks))
        out.write("\n")
        out.write("#endif /* MPICH test */\n")


def write_fortran_wrappers(out, decl, return_val):
    """Writes primary fortran wrapper that handles arg translation.
       Also outputs bindings for this wrapper for different types of fortran compilers.
    """
    delegate_name = decl.name + f_wrap_suffix
    out.write(decl.fortranPrototype(delegate_name))
    out.write(" { \n")

    call = FortranDelegation(decl.name, return_val)
    
    if decl.name == "MPI_Init":
        out.write("    int argc = 0;\n");
        out.write("    char ** argv = NULL;\n");
        out.write("    init_was_fortran = 1;\n");
        
        call.addActual("&argc");
        call.addActual("&argv");
        
        call.write(out)
        out.write("    *ierr = %s;\n" % return_val)
        out.write("}\n\n")
        return

    for arg in decl.args:
        if arg.name == "...":   # skip ellipsis
            continue
    
        if not (arg.pointers or arg.array):
            if not arg.isHandle():
                # These are pass-by-value arguments, so just deref and pass thru
                dereferenced = "*(%s)" % arg.name
                call.addActual(dereferenced)
            else:
                # Non-ptr, non-arr handles need to be converted with MPI_Blah_f2c
                # No special case for MPI_Status here because MPI_Statuses are never passed by value.
                call.addActualMPI2("%s_f2c(*(%s))" % (conversion_prefix(arg.type), arg.name))
                call.addActualMPICH("(%s)(*(%s))" % (arg.type, arg.name))

        else:
            if not arg.isHandle():
                # Non-MPI handle pointer types can be passed w/o dereferencing
                call.addActual(arg.name)
            else:
                # For MPI-1, assume ints, cross fingers, and pass things straight through.
                call.addActualMPICH("(%s*)%s" % (arg.type, arg.name))
                conv = conversion_prefix(arg.type)
                temp = "temp_%s" % arg.name

                # For MPI-2, other pointer and array types need temporaries and special conversions.
                if not arg.isHandleArray():
                    call.addTemp(arg.type, temp)
                    call.addActualMPI2("&%s" % temp)

                    if arg.isStatus():
                        call.addCopy("%s_f2c(%s, &%s);"  % (conv, arg.name, temp))
                        call.addWriteback("%s_c2f(&%s, %s);" % (conv, temp, arg.name))
                    else:
                        call.addCopy("%s = %s_f2c(*(%s));"  % (temp, conv, arg.name))
                        call.addWriteback("*(%s) = %s_c2f(%s);" % (arg.name, conv, temp))
                else:
                    # Make a temporary variable for the array
                    temp_arr_type = "%s*" % arg.type
                    call.addTemp(temp_arr_type, temp)
                
                    # generate a copy and a writeback statement for this type of handle
                    if arg.isStatus():
                        copy = "    %s_f2c(&%s[i], &%s[i])"  % (conv, arg.name, temp)
                        writeback = "    %s_c2f(&%s[i], &%s[i])" % (conv, temp, arg.name)
                    else:
                        copy = "    temp_%s[i] = %s_f2c(%s[i])"  % (arg.name, conv, arg.name)
                        writeback = "    %s[i] = %s_c2f(temp_%s[i])" % (arg.name, conv, arg.name)
                
                    # Generate the call surrounded by temp array allocation, copies, writebacks, and temp free
                    count = "*(%s)" % arg.countParam().name
                    call.addCopy("%s = (%s)malloc(sizeof(%s) * %s);" %
                                         (temp, temp_arr_type, arg.type, count))
                    call.addCopy("for (int i=0; i < %s; i++) %s;" % (count, copy))
                    call.addActualMPI2(temp)
                    call.addWriteback("for (int i=0; i < %s; i++) %s;" % (count, writeback))
                    call.addWriteback("free(%s);" % temp)
            
    call.write(out)
                    
    out.write("    *ierr = %s;\n" % return_val)
    out.write("}\n\n")

    # Write out various bindings that delegate to the main fortran wrapper
    write_fortran_binding(out, decl, delegate_name, decl.name.upper())
    write_fortran_binding(out, decl, delegate_name, decl.name.lower())
    write_fortran_binding(out, decl, delegate_name, decl.name.lower() + "_")
    write_fortran_binding(out, decl, delegate_name, decl.name.lower() + "__")


class Scope:
    """Maps string keys to either macros or values.
       Supports nesting: if values are not found in this scope, then 
       enclosing scopes are searched recursively.
    """
    def __init__(self, enclosing_scope=None):
        self.map = {}
        self.enclosing_scope = enclosing_scope

    def __getitem__(self, key):
        if key in self.map:
            return self.map[key]
        elif self.enclosing_scope:
            return self.enclosing_scope[key]
        else:
            raise KeyError(key + " is not in scope.")

    def __contains__(self, key):
        if key in self.map:
            return True
        elif self.enclosing_scope:
            return key in self.enclosing_scope
        else:
            return False

    def __setitem__(self, key, value):
        self.map[key] = value

    def include(self, map):
        """Add entire contents of the map (or scope) to this scope."""
        for key in map:
            self.map[key] = map[key]

    def include_decl(self, decl):
        self["retType"]     = decl.retType()
        self["argTypeList"] = decl.argTypeList()
        self["argList"]     = decl.argList()


def macro(fun):
    """Put a function in the macro table if it's annotated as a macro."""
    macros[fun.__name__] = fun
    return fun

def all_but(fn_list):
    """Return a list of all mpi functions except those in fn_list"""
    all_mpi = set(mpi_functions.keys())
    diff = all_mpi - set(fn_list)
    return [x for x in diff]

@macro
def foreachfn(out, scope, args, children):
    """Iterate over all functions listed in args."""
    fn_var = args[0]
    for fn_name in args[1:]:
        if not fn_name in mpi_functions:
            raise SyntaxError(fn_name + " is not an MPI function")

        fn = mpi_functions[fn_name]
        scope[fn_var] = fn_name
        scope.include_decl(fn)
        for child in children:
            child.execute(out, scope)

@macro
def fn(out, scope, args, children):
    """Iterate over listed functions and generate skeleton too."""
    fn_var = args[0]
    for fn_name in args[1:]:
        if not fn_name in mpi_functions:
            raise SyntaxError(fn_name + " is not an MPI function")

        fn = mpi_functions[fn_name]
        return_val = "return_val"

        scope[fn_var] = fn_name
        scope.include_decl(fn)
        scope["return_val"] = return_val

        c_call = "%s = P%s%s;" % (return_val, fn.name, fn.argList())
        scope["callfn"] = c_call

        if fn_name == "MPI_Init":
            def callfn(out, scope, args, children):
                out.write("    if (init_was_fortran) {\n")
                out.write("        pmpi_init_(&return_val);\n")
                out.write("    } else {\n")
                out.write("        %s\n" % c_call)
                out.write("    }\n")
            scope["callfn"] = callfn
        
        def write_body(out):
            for child in children:
                child.execute(out, scope)

        write_c_wrapper(out, fn, return_val, write_body)
        if output_fortran_wrappers:
            write_fortran_wrappers(out, fn, return_val)

@macro
def forallfn(out, scope, args, children):
    """Iterate over all but the functions listed in args."""
    foreachfn(out, scope, [args[0]] + all_but(args[1:]), children)

@macro
def fnall(out, scope, args, children):
    """Iterate over all but listed functions and generate skeleton too."""
    fn(out, scope, [args[0]] + all_but(args[1:]), children)


class Chunk:
    """Represents a piece of a wrapper file.  Is either a text chunk
       or a macro chunk with children to which the macro should be applied.
       macros are evaluated lazily, so the macro is just a string until 
       execute is called and it is fetched from its enclosing scope."""
    def __init__(self):
        self.macro    = None
        self.args     = []
        self.text     = None
        self.children = []

    def iwrite(self, file, level, text):
        """Write indented text."""
        for x in xrange(level): 
            file.write("  ")
        file.write(text)

    def write(self, file=sys.stdout, l=0):
        if self.macro: 
            self.iwrite(file, l, self.macro + "\n")

        if self.args: 
            self.iwrite(file, l," ".join(self.args) + "\n")

        if self.text: 
            self.iwrite(file, l, "TEXT\n")

        for child in self.children:
            child.write(file, l+1)

    def execute(self, out, scope):
        if not self.macro:
            out.write(self.text)
        else:
            if not self.macro in scope:
                raise SyntaxError("Invalid macro: " + self.macro)

            macro = scope[self.macro]
            if isinstance(macro, str):
                # raw strings in the scope will just get printed out.
                out.write(macro)
            else:
                # macros get executed inside a new scope 
                macro(out, Scope(scope), self.args, self.children)
        

def parse(tokens, macros, end_macro=None):
    """Turns a string of tokens into a list of chunks.
       Macros that have a has_body attribute will be recursively parsed
       and the result will be appended as a list of child chunks."""
    chunk_list = []

    for token in tokens:
        chunk = Chunk()

        if token.isa(TEXT):
            chunk.text = token.value

        elif token.isa(LBRACE):
            text, close = tokens.next(), tokens.next()
            if not text.isa(TEXT) or not close.isa(RBRACE):
                raise SyntaxError("Expected macro body after open brace.")

            args = text.value.split()
            name = args.pop(0)
            if name == end_macro:
                break
            else:
                chunk.macro = name
                chunk.args  = args
                if name in macros:
                    chunk.children = parse(tokens, macros, "end" + name)
        else:
            raise SyntaxError("Expected text block or macro.")

        chunk_list.append(chunk)

    return chunk_list


def usage():
    print "Usage: wrap.py [-fg] [-c mpicc_name] [-o file] wrapper.w [...]"
    print "  Python script for creating PMPI wrappers. Roughly follows the syntax of "
    print "  the Argonne PMPI wrapper generator, with some enhancements."
    print "Options:"
    print "  -f        Generate fortran wrappers in addition to C wrappers."
    print "  -g        Generate reentry guards around wrapper functions."
    print "  -c exe    Provide name of MPI compiler (for parsing mpi.h).  Default is mpicc."
    print "  -o file   Send output to a file instead of stdout."
    sys.exit(2)


# Let the user specify another mpicc to get mpi.h from
output = sys.stdout
try:
    opts, args = getopt.gnu_getopt(sys.argv[1:], "fgc:o:")
except getopt.GetoptError, err:
    usage()

if len(args) < 1:
    usage()

for opt, arg in opts:
    if opt == "-f": 
        output_fortran_wrappers = True
    if opt == "-g": 
        output_guards = True
    if opt == "-c": 
        mpicc = arg
    if opt == "-o": 
        try:
            output = open(arg, "w")
        except IOError:
            sys.stderr.write("Error: couldn't open file " + arg + " for writing.\n")
            sys.exit(1)

#
# Parse mpi.h and put declarations into a map.
#
for decl in enumerate_mpi_declarations(mpicc):
    mpi_functions[decl.name] = decl

#
# Parse each file listed on the command line and execute
# it once it's parsed.
#
fileno = 0
lexer = Lexer("{{","}}")

# Start by including mpi.h
output.write("#include <mpi.h>\n")

if output_guards:
    output.write("static int in_wrapper = 0;\n")

output.write("static int init_was_fortran = 0;\n")
output.write("#pragma weak pmpi_init=pmpi_init_\n")
output.write("#pragma weak PMPI_INIT=pmpi_init_\n")
output.write("#pragma weak pmpi_init__=pmpi_init_\n\n")

output.write("#ifdef __cplusplus\n")
output.write("extern \"C\" {\n")
output.write("#endif /* __cplusplus */\n")
output.write("    void pmpi_init(MPI_Fint *ierr);\n")
output.write("    void PMPI_INIT(MPI_Fint *ierr);\n")
output.write("    void pmpi_init_(MPI_Fint *ierr);\n")
output.write("    void pmpi_init__(MPI_Fint *ierr);\n")
output.write("#ifdef __cplusplus\n")
output.write("}\n")
output.write("#endif /* __cplusplus */\n")

for filename in args:
    file = open(filename)

    # Outer scope contains fileno and the basic macros.
    outer_scope = Scope()
    outer_scope["fileno"] = str(fileno)
    outer_scope.include(macros)

    for chunk in parse(lexer.lex(file), macros):
        chunk.execute(output, outer_scope)

    fileno += 1

output.close()
