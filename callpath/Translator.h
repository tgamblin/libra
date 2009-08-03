#ifndef CALLPATH_TRANSLATOR_H
#define CALLPATH_TRANSLATOR_H

#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "FrameInfo.h"
#include "Callpath.h"
#include "ModuleId.h"

class symtab_info;

///
/// Class to turn FrameIds into more useful FrameInfo (with symbol data).
///
class Translator { 
private:
  ModuleId executable;
public:
  /// Construct a translator; optionally provide the location of the executable 
  /// for translating frames with unknown modules.
  Translator(const std::string& exe = "");
  ~Translator();

  /// Given a module/offste frame, get FrameInfo with symbol information.
  FrameInfo translate(const FrameId& frame);

  /// Given a callpath writes out the names of all the symbols in it, nicely formatted.
  void write_path(std::ostream& out, const Callpath& path, bool one_line=false, std::string indent="");

  /// Given a callpath writes out the names of all the symbols in it, nicely formatted.
  void write_path(std::ostream& out, const Callpath& path, std::string indent);

  /// Set the executable.
  void set_executable(const std::string& exe);
  
private:
  /// Lookup table for symtabs we've already parsed.
  typedef std::map<ModuleId, symtab_info*> cache;

  /// Cache of all symbtabs seen so far.
  cache symtabs;

  /// Reads in a symbol table for the module specified.  Aborts on failure.
  symtab_info *get_symtab_info(ModuleId module);

  /// Cleans up symtab info
  void cleanup_symtab_info();
}; // Translator

#endif // CALLPATH_TRANSLATOR_H


