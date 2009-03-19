#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
using namespace std;

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "ModuleId.h"
#include "wavelet.h"
#include "wt_direct.h"
#include "ezw_decoder.h"
#include "matrix_utils.h"
using namespace wavelet;

#include "effort_key.h"
using namespace effort;

#include "string_utils.h"
using namespace stringutils;

#ifdef HAVE_SYMTAB
#include "Symtab.h"
#include "Symbol.h"
using namespace Dyninst::SymtabAPI;
#endif // HAVE_SYMTAB

typedef unsigned decompression_stage;

const decompression_stage none        = 0x0;
const decompression_stage metadata    = 0x1;
const decompression_stage wt_coeff    = 0x2;
const decompression_stage reconstruct = 0x4;
decompression_stage stage = none;

ModuleId UNKNOWN_MODULE("[unknown module]");

int iwt_level = -1;                    /// Deterines level of compression
bool reduce = false;                   /// Output reduced size metrix for small levels
bool translate = false;                /// Whether to translate symbol names as we find them.
bool one_line = false;                 /// Whether to translate symbol names as we find them.
string fields("mtazsrclSMTebpERZ");    /// Which fields to show. All if empty.
ModuleId executable(UNKNOWN_MODULE);   /// Optionally supplied executable to look up symbols in.

/// Usage parameters
void usage() {
  cerr << "Usage: ef [-hmwxrfo] [-e exe] [-p num] [-s fields] compresed_file [...]" << endl;
  cerr << "  By default, this tool simply prints out metadata from an effort file." << endl;
  cerr << "Other options:" << endl;
  cerr << "  -h         Show this message." << endl;
  cerr << "  -m         Output metadata to a file." << endl;
  cerr << "  -w         Output wavelet data to a file." << endl;
  cerr << "  -x         Output full reconstruction to a file." << endl;
  cerr << "  -l num     Only apply <num> levels of inverse transform." << endl;
  cerr << "  -r         Output a reduced-size matrix for small level counts." << endl;
  cerr << "  -f         Look up and print out symbol names for addrs (With SymtabAPI only)." << endl;
  cerr << "  -e exe     Use exe file to look up symbols. Use when Stackwalk can't figure" << endl;
  cerr << "             out modules." << endl;
  cerr << "  -o         Print all fields on one line, with no headings, separated by '|'" << endl;
  cerr << "  -s fields  Show only certain fields, where fields is any set of:" << endl;
  cerr << "              m    Metric      s    Size        b   Blocks"   << endl;
  cerr << "              t    Type        l    Level       p   Passes"   << endl;
  cerr << "              a    Start       S    Scale       E   EZW_Size" << endl;
  cerr << "              z    End         M    Mean        R   RLE_Size" << endl;
  cerr << "              r    Rows        T    Thresh      Z   ENC_Size" << endl;
  cerr << "              c    Cols        e    Enc"                      << endl;
  exit(1);
}


/// Uses getopt to read in arguments.
void get_args(int *argc, char ***argv) {
  int c;
  char *err;
  ifstream vdata;

  while ((c = getopt(*argc, *argv, "mwxrfhos:e:l:")) != -1) {
    switch (c) {
    case 'm':
      stage |= metadata;
      break;
    case 'w':
      stage |= wt_coeff;
      break;
    case 'x':
      stage |= reconstruct;
      break;
    case 'l':
      iwt_level = strtol(optarg, &err, 10);
      if (*err) usage();
      break;
    case 'r':
      reduce = true;
      break;
    case 's':
      fields = string(optarg);
      break;
    case 'o':
      one_line=true;
      break;
    case 'f':
#ifdef HAVE_SYMTAB
      translate = true;
#else 
      vdata.open("viewer-data/symtab");
      if (vdata.fail()) {
        cerr << "ERROR: -f requires either generated viewer-data or DynStackwalker API." << endl;
        exit(1);
      } else {
        translate = true;
      }
#endif // HAVE_SYMTAB
      break;
    case 'e':
#ifdef HAVE_SYMTAB
      translate = true;
#else 
      cerr << "ERROR: -e requires DynStackwalker API." << endl;
      exit(1);
#endif // HAVE_SYMTAB
      executable = string(optarg);
      break;
    case 'h':
    default:
      usage();
      break;
    }
  }

  // adjust params
  *argc -= optind;
  *argv += optind;
}

//
// Below routines will only compile with symtab API installed.
//
#ifdef HAVE_SYMTAB

struct symbol_addr_gt {
  bool operator()(Symbol* lhs, Symbol *rhs)   { return lhs->getAddr() > rhs->getAddr(); }
  bool operator()(uintptr_t lhs, Symbol *rhs) { return lhs > rhs->getAddr(); }
  bool operator()(Symbol* lhs, uintptr_t rhs) { return lhs->getAddr() > rhs; }
};

struct symtab_info {
  Symtab *symtab;
  vector<Symbol*> syms;
  symtab_info(Symtab *s) : symtab(s) { }

  /// This gets a symbol name from an offset the same way stackwalker does it.
  void getName(uintptr_t offset, string& name) {
    if (!syms.size()) {
      if (!symtab->getAllSymbolsByType(syms, Symbol::ST_FUNCTION)) {
        cerr << "ERROR: couldn't read symbols from " << symtab->file() << endl;
        return;
      }
      sort(syms.begin(), syms.end(), symbol_addr_gt());
    }
    
    Symbol *sym = 
      *lower_bound(syms.begin(), syms.end(), offset, symbol_addr_gt());

    name = sym->getTypedName();
    if (!name.length())
      name = sym->getPrettyName();
    if (!name.length())
      name = sym->getName();
  }
};

typedef map<ModuleId, symtab_info> symtab_cache;
static symtab_cache symtabs;


/// Reads in a symbol table for the executable file; aborts on failure.
symtab_info *getSymtabInfo(ModuleId module) {
  symtab_cache::iterator sti = symtabs.find(module);
  if (sti == symtabs.end()) {
    Symtab *symtab;
    if (!Symtab::openFile(symtab, module.str())) {
      cerr << "Error opening file: " << module.str() << endl;
      exit(1);
    }
    sti = symtabs.insert(symtab_cache::value_type(module, symtab_info(symtab))).first;
  }

  return &sti->second;
}

#endif // HAVE_SYMTAB


struct FrameInfo {
  string file;
  string line_num;
  string sym_name;
  
  FrameInfo(const string& f, int l, const string& n) : file(f), sym_name(n) { 
    ostringstream ln;
    ln << l;
    line_num = ln.str();
  }

  FrameInfo() { }

  void write(ostream& out, size_t file_line_width, size_t sym_width) {
    if (file == "" && translate) {
      out << left << setw(file_line_width) << "[unknown]";
      out << left << setw(sym_width) << "[unknown]";
    } else {
      ostringstream file_line;
      file_line << file + ":" + line_num;
      out << left << setw(file_line_width) << file_line.str();
      out << left << setw(sym_width) << sym_name;
    }
  }

};


FrameInfo  get_symtab_frame_info(const FrameId& frame) {
#ifndef HAVE_SYMTAB
  cerr << "ERROR: -f requires either generated viewer-data or SymtabAPI." << endl;
  exit(1);
#else // HAVE_SYMTAB
  vector<LineNoTuple> lines;

  ModuleId module = frame.module;
  if (module == UNKNOWN_MODULE) module = executable;

  symtab_info *stinfo = getSymtabInfo(module);
    
  // Subtract one from the offset here, to hackily
  // convert return address to callsite
  uintptr_t offset = frame.offset ? frame.offset - 1 : frame.offset;
  if (stinfo->symtab->getSourceLines(lines, offset)) {
    string name;
    stinfo->getName(offset, name);
    return FrameInfo(lines[0].first, lines[0].second, name);
    
  } else {
    return FrameInfo();
  }
#endif // HAVE_SYMTAB
}



FrameInfo get_frame_info(const FrameId& frame) {
  static map<const FrameId, FrameInfo> viewer_data;
  static bool vd_cache = true;

  if (!vd_cache) {
    return get_symtab_frame_info(frame);
  }

  if (viewer_data.size() == 0) {
    ifstream vd("viewer-data/symtab");
    if (vd.fail()) {
      vd_cache = false;
      return get_symtab_frame_info(frame);
      
    } else {
      // some module names need to be substituted.
      map<string, string> module_mappings;
      
      // if we found the viewer data file build up the map.
      string line;
      while (getline(vd, line)) {
        if (line.find("=>") != string::npos) {
          // this is a mapping from something like [unknown module] to an exe 
          // provided at symbol generation time
          vector<string> mapping;
          split_str(line, "=>", mapping);
          
          if (mapping.size() != 2) {
            cerr << "Invalid mapping in viewer-data/symbtab: " << endl;
            cerr << "    " << line << endl;
            cerr << mapping.size() << endl;
            exit(1);
          }
          
          string orig = trim(mapping[1]);
          string alias = trim(mapping[0]);
          module_mappings[orig] = alias;

        } else {
          // this is file/line info for some (module, offset)
          vector<string> parts;
          split(line, "|", parts);

          char *err;
          uintptr_t offset = strtol(parts[4].c_str(), &err, 0);
          if (*err) {
            cerr << "Invalid offset in viewer-data/symtab: " << parts[4] << endl;
            exit(1);
          }
          string module = parts[3];
          FrameId key(module, offset);          
          
          if (parts[0] == "?") {
            viewer_data[key] = FrameInfo();
            
          } else {
            int line_num = strtol(parts[1].c_str(), &err, 10);
            if (*err) {
              cerr << "Invalid line number in viewer-data/symtab: " << parts[1] << endl;
              exit(1);
            }

            FrameInfo info(parts[0], line_num, parts[2]);
            viewer_data[key] = info;
            
            if (module_mappings.find(module) != module_mappings.end()) {
              FrameId alias(module_mappings[module], offset);
              viewer_data[alias] = info;
            }
          }
        }
      }
    }
  }

  return viewer_data[frame];
}


/// Given a callpath writes out the names of all the symbols in it.
void write_names_for_path(ostream& out, const Callpath& path) {
  if (!path.size()) {
    out << "null_callpath";
    if (!one_line) {
      out << endl;
    }
    return;
  }
  
  size_t max_file = 0;
  size_t max_line = 0;
  size_t max_sym = 0;
  for (int i=path.size()-1; i >= 0; i--) {
    FrameInfo inf = get_frame_info(path[i]);
    max_file = max(max_file, inf.file.size());
    max_line = max(max_line, inf.line_num.size());
    max_sym  = max(max_sym,  inf.sym_name.size());
  }

  for (int i=path.size()-1; i >= 0; i--) {
    if (i != (int)path.size()-1) 
      out << (one_line ? " : " : "          ");

    if (translate && !one_line) {
      get_frame_info(path[i]).write(out, max_file+max_line + 3, max_sym + 2);
    }

    ModuleId module = path[i].module;
    if (module == UNKNOWN_MODULE) module = executable;

    out << module << "(0x" << hex << path[i].offset << ")";
    if (!one_line) out << endl;
    out << dec; // revert output.
  }
}



struct row_header {
  string name;
  row_header(string n) : name(n) { }
};

ostream& operator<<(ostream& out, const row_header& h) {
  if (!one_line) out << setw(10) << left << h.name;
  return out;
}

void write_field(char fid, ostream& out, const effort_key& key, const ezw_header& header) {
  switch (fid) {
  case 'm': out << row_header("Metric")   << key.metric;      break;
  case 't': out << row_header("Type")     << key.type;        break;  
  case 'r': out << row_header("Rows")     << header.rows;     break;
  case 'c': out << row_header("Cols")     << header.cols;     break;
  case 'l': out << row_header("Level")    << header.level;    break;
  case 'S': out << row_header("Scale")    << header.scale;    break;
  case 'M': out << row_header("Mean")     << header.mean;     break;
  case 'e': out << row_header("Enc")      << header.enc_type; break;
  case 'b': out << row_header("Blocks")   << header.blocks;   break;
  case 'p': out << row_header("Passes")   << header.passes;   break;
  case 'E': out << row_header("EZW_Size") << header.ezw_size; break;
  case 'R': out << row_header("RLE_Size") << header.rle_size; break;
  case 'Z': out << row_header("ENC_Size") << header.enc_size; break;
  case 's': out << row_header("Size")     << header.rows * header.cols; break;
  case 'T': out << row_header("Thresh")   << hex << "0x"     << header.threshold << dec; break;
  case 'a': 
    out << row_header("Start");
    write_names_for_path(out, key.start_path);
    break;
  case 'z': 
    out << row_header("End");
    write_names_for_path(out, key.end_path);
    break;

  default: // ignore
    break;
  }
}



void write_metadata(ostream& out, const effort_key& key, const ezw_header& header) {
  for (size_t i=0; i < fields.size(); i++) {
    write_field(fields[i], out, key, header);

    if (one_line && (i < fields.size()-1)) {
      out << " | ";
    } else {
      out << endl;
    }
  }
}


int main(int argc, char **argv) {
  if (argc < 2) {
    usage();
  }

  get_args(&argc, &argv);

  for (int i=0; i < argc; i++) {
    ifstream comp_file(argv[i]);
    if (comp_file.fail()) {
      cerr << "Unable to open file: '" << argv[i] << "'" << endl;
      exit(1);
    }
    
    effort_key key;
    ezw_header header;

    effort_key::read_in(comp_file, key);    
    ezw_header::read_in(comp_file, header);

    if (stage == none) {      // no parameters
      write_metadata(cout, key, header);
      continue;
    }

    string metric;
    int type, number;
    if (!parse_filename(argv[i], &metric, &type, &number)) {
      cerr << "Invalid effort file: " << argv[i] << endl;
      exit(1);
    }
    
    ostringstream sufstr;
    sufstr << "-" << metric << "-" << type << "-" << number;
    string suffix = sufstr.str();

    if (stage & metadata) {      // output metadata to a file.
      ostringstream mdname;
      mdname <<  "md" << suffix;
      ofstream mdfile(mdname.str().c_str());
      write_metadata(mdfile, key, header);
    }

    if (stage < wt_coeff) return 0;
    // Do EZW decoding to get wavelet coefficients
    wavelet::wt_matrix reconstruction;
    ezw_decoder decoder;

    // Use the decode level in the header by default for both ezw and iwt.
    if (iwt_level < 0) iwt_level = header.level;

    if (reduce) {
      // if we're reducing the size of the output, we need to tell the decoder
      // to create a matrix to hold only the inverse-transformed levels.
      iwt_level = decoder.decode(comp_file, reconstruction, iwt_level, &header);

    } else {
      // if not, then we do a full ezw decode with the level of the forward transform
      // Don't set the IWT level, though.  The user may still want fewer inverse
      // transforms but full size data.
      decoder.decode(comp_file, reconstruction, header.level, &header);
    }




    if (stage & wt_coeff) {
      ostringstream matname;
      matname <<  "wt" << suffix;
      ofstream wtfile(matname.str().c_str());
      output(reconstruction, wtfile);
    }

    if (stage < reconstruct) return 0;

    // Do iwt for full reconstruction.
    wt_direct dwt;
    dwt.iwt_2d(reconstruction, iwt_level);
    
    if (stage & reconstruct) {
      ostringstream matname;
      matname <<  "recon" << suffix;
      ofstream reconfile(matname.str().c_str());
      output(reconstruction, reconfile);
    }
  }

  return 0;
}
