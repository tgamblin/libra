#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <libgen.h>
#include <sys/stat.h>
using namespace std;

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "ModuleId.h"
#include "FrameInfo.h"
#include "Translator.h"

#include "wavelet.h"
#include "wt_direct.h"
#include "io_utils.h"
#include "ezw_decoder.h"
#include "matrix_utils.h"
using namespace wavelet;

#include "effort_key.h"
#include "FrameDB.h"
using namespace effort;


typedef unsigned decompression_stage;

const decompression_stage none        = 0x0;
const decompression_stage metadata    = 0x1;
const decompression_stage wt_coeff    = 0x2;
const decompression_stage reconstruct = 0x4;
decompression_stage stage = none;

int iwt_level = -1;                    /// Deterines level of compression
bool reduce = false;                   /// Output reduced size metrix for small levels
bool translate = false;                /// Whether to translate symbol names as we find them.
bool one_line = false;                 /// Whether to translate symbol names as we find them.
string fields("mtazsrclSMTebpERZ");    /// Which fields to show. All if empty.

auto_ptr<FrameDB> frames;              /// Cache of data from pre-generated symtab data file.
Translator translator;

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


bool exists(string filename) {
  struct stat st;
  return !stat(filename.c_str(), &st);
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
      translate = true;
      break;
    case 'e':
      {
        translate = true;
        string exe(optarg);
        if (!exists(exe)) {
          cerr << exe << ": No such file or directory." << endl;
          exit(1);
        }
        translator.set_executable(exe);
      }
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



FrameInfo get_frame_info(const FrameId& frame) {
  if (frames.get()) {
    return frames->info_for(frame);
  } else if (translate) {
    return translator.translate(frame);
  } else {
    return FrameInfo(frame.module, frame.offset);
  }
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
  
  FrameInfo infos[path.size()];
  
  // find max field widths for the output.
  size_t max_file = 0;
  size_t max_line = 0;
  size_t max_sym = 0;
  
  for (int i=path.size()-1; i >= 0; i--) {
    infos[i] = get_frame_info(path[i]);
    max_file = max(max_file, infos[i].file.size());
    max_line = max(max_line, infos[i].line_num.size());
    max_sym  = max(max_sym,  infos[i].sym_name.size());
  }

  if (one_line) {
    if (path.size()) {
      out << infos[path.size()-1];
    }

    for (int i=path.size()-2; i >= 0; i--) {
      out << " : " << infos[i];
    }

  } else {
    size_t file_line_width = max_file+max_line + 3;
    size_t max_sym_width = max_sym + 2;

    if (path.size()) {
      infos[path.size()-1].write(out, file_line_width, max_sym_width);
    }

    for (int i=path.size()-2; i >= 0; i--) {
      out << endl << "          ";
      infos[i].write(out, file_line_width, max_sym_width);      
    }
    out << endl;
  }
}



struct md_name {
  const string& name;
  md_name(const string& n) : name(n) { }
};

ostream& operator<<(ostream& out, const md_name& h) {
  if (!one_line) out << setw(10) << left << h.name;
  return out;
}

void write_field(char fid, ostream& out, const effort_key& key, const ezw_header& header) {
  switch (fid) {
  case 'm': out << md_name("Metric")   << key.metric;      break;
  case 't': out << md_name("Type")     << key.type;        break;  
  case 'r': out << md_name("Rows")     << header.rows;     break;
  case 'c': out << md_name("Cols")     << header.cols;     break;
  case 'l': out << md_name("Level")    << header.level;    break;
  case 'S': out << md_name("Scale")    << header.scale;    break;
  case 'M': out << md_name("Mean")     << header.mean;     break;
  case 'e': out << md_name("Enc")      << header.enc_type; break;
  case 'b': out << md_name("Blocks")   << header.blocks;   break;
  case 'p': out << md_name("Passes")   << header.passes;   break;
  case 'E': out << md_name("EZW_Size") << header.ezw_size; break;
  case 'R': out << md_name("RLE_Size") << header.rle_size; break;
  case 'Z': out << md_name("ENC_Size") << header.enc_size; break;
  case 's': out << md_name("Size")     << header.rows * header.cols; break;
  case 'T': out << md_name("Thresh")   << hex << "0x"     << header.threshold << dec; break;
  case 'a': 
    out << md_name("Start");
    write_names_for_path(out, key.start_path);
    break;
  case 'z': 
    out << md_name("End");
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
  translator.set_callsite_mode(true);  // translate callsites, not raw addrs.

  for (int i=0; i < argc; i++) {
    ifstream comp_file(argv[i]);
    if (comp_file.fail()) {
      cerr << "Unable to open file: '" << argv[i] << "'" << endl;
      exit(1);
    }

    // try to find frame info database based on location of first effort file
    // fail if it's not found and we can't look up the symbols with SymtabAPI
    if (translate) {
      string dir(dirname(argv[i]));
      ostringstream path;
      path << dir << "/viewer-data/symtab";
      frames.reset(FrameDB::load_from_file(path.str()));
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
