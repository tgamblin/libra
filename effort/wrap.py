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
    "\w+\s*" +                     # Type name w/optional space (doesn't handle long long right now)
    "(?:\s*\*\s*(?:const)?)*\s*" + # Optional, potentially const pointers
    ")\s*" +                       # End type
    "(?:(\w+)\s*)?" +              # Optional argument name
    "(\[.*\])?\s*$"                # Array type (works for multidimensions b/c it's greedy)
    )

# Fortran wrapper suffix
f_wrap_suffix = "_fortran_wrapper"

# Map from function name to declaration created from mpi.h.
mpi_functions = {}

# Global table of macro functions, keyed by name.
macros = {}

class Formal:
    """Descriptor for formal parameters of MPI functions.  
       Doesn't represent a full parse, only the initial type information, 
       name, and array info of the argument split up into strings.
    """
    def __init__(self, type, name, array_type):
        self.type = type
        self.name = name
        self.array_type = array_type

    def forFortran(self):
        return "MPI_Fint *%s" % self.name

    def forC(self):
        if not self.type:
            return self.name  # special case for '...'
        else:
            arr = self.array_type or ''
            type = self.type
            if not type.endswith("*"): 
                type += ' '
            return "%s%s%s" % (type, self.name, arr)

    def __str__(self):
        return self.forC()


class Declaration:
    """ Descriptor for simple MPI function declarations.  
        Contains return type, name of function, and a list of args.
    """
    def __init__(self, type, name, arg_list):
        self.type = type
        self.name = name
        self.arg_list = arg_list

    def __iter__(self):
        for arg in self.arg_list: yield arg

    def __str__(self):
        return self.prototype()

    def retType(self):
        return self.type

    def argTypeList(self):
        return "(" + ", ".join(map(Formal.forC, self.arg_list)) + ")"

    def fortranArgTypeList(self):
        return "(" + ", ".join(map(Formal.forFortran, self.arg_list)) + ", MPI_Fint *ierr)"

    def argList(self):
        return "(" + ", ".join([arg.name for arg in self]) + ")"

    def fortranArgList(self):
        return "(" + ", ".join([arg.name for arg in self]) + ", ierr)"

    def prototype(self):
        return "%s %s%s" % (self.retType(), self.name, self.argTypeList())
    
    def fortranPrototype(self, name=None):
        if not name: name = self.name
        return "void %s%s" % (name, self.fortranArgTypeList())
    

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
            formals = []
            arg_num = 0
            for arg in arg_list:
                if arg == '...':   # Special case for Pcontrol.
                    formals.append(Formal(None, '...', None))
                else:
                    match = formal_re.match(arg)
                    if not match:
                        print "MATCH FAILED FOR: '" + arg + "' in " + fn_name
                        sys.exit(1)

                    type, name, array_type = match.groups()
                    # If there's no name, make one up.
                    if not name: name = "arg_" + str(arg_num)
                        
                    formals.append(Formal(type.strip(), name, array_type))
                arg_num += 1
                    
            yield Declaration(return_type, fn_name, formals)

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


def write_c_wrapper(out, decl, write_body):
    out.write(decl.prototype())
    out.write("\n { \n")
    out.write("    int return_val = 1;\n")

    write_body(out)

    out.write("  return return_val;\n")
    out.write(" }\n\n")


def write_fortran_delegation(out, decl, binding):
    """Outputs a wrapper for a particular fortran binding that delegates to the
       primary Fortran wrapper.
    """
    out.write(decl.fortranPrototype(binding))
    out.write("\n { \n")
    out.write("    %s%s;\n" % (decl.name + f_wrap_suffix, decl.fortranArgList()))
    out.write(" }\n\n")
    

def write_fortran_wrappers(out, decl):
    """Writes primary fortran wrapper that handles arg translation.
       Also outputs bindings for this wrapper for different types of fortran compilers.
    """
    
    out.write(decl.fortranPrototype(decl.name + f_wrap_suffix))
    out.write("\n { \n")
    out.write("    // body\n")
    out.write(" }\n\n")
    
    write_fortran_delegation(out, decl, decl.name.upper())
    write_fortran_delegation(out, decl, decl.name.lower())
    write_fortran_delegation(out, decl, decl.name.lower() + "_")
    write_fortran_delegation(out, decl, decl.name.lower() + "__")


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
        scope["callfn"] = "%s = P%s%s;" % (return_val, fn.name, fn.argList())
        
        def write_body(out):
            for child in children:
                child.execute(out, scope)

        write_c_wrapper(out, fn, write_body)
        write_fortran_wrappers(out, fn)
        

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
    print "Usage: wrap.py [-c mpicc_name] wrapper.w [...]"
    print "  Python script for creating PMPI wrappers. Roughly follows the syntax of "
    print "  the Argonne PMPI wrapper generator, with some enhancements."
    sys.exit(2)


# Let the user specify another mpicc to get mpi.h from
output = sys.stdout
try:
    opts, args = getopt.gnu_getopt(sys.argv[1:], "c:o:")
except getopt.GetoptError, err:
    usage()

if len(args) < 1:
    usage()

for opt, arg in opts:
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
