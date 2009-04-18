#include "FrameDB.h"

#include <vector>
#include <fstream>
using namespace std;

#include "string_utils.h"
using namespace stringutils;


namespace effort {

  FrameDB::FrameDB() { }
  
  FrameDB::~FrameDB() { }
  
  size_t FrameDB::size() {
    return frames.size();
  }

  FrameInfo FrameDB::info_for(FrameId key) {
    frame_map::iterator i = frames.find(unalias(key));
    if (i != frames.end()) {
      return i->second;
    } else {
      return FrameInfo();
    }
  }


  FrameId FrameDB::unalias(FrameId key) {
    alias_map::iterator i = aliases.find(key.module);
    if (i != aliases.end()) {
      FrameId foo = FrameId(i->second, key.offset);
      key = foo;
    }
    return key;
  }


  void FrameDB::add_alias(const string& line) {
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
    
    ModuleId orig(trim(mapping[1]));
    ModuleId alias(trim(mapping[0]));
    aliases[alias] = orig;
  }

  
  void FrameDB::add_info(const string& line) {
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
      frames[key] = FrameInfo();
            
    } else {
      int line_num = strtol(parts[1].c_str(), &err, 10);
      if (*err) {
        cerr << "Invalid line number in viewer-data/symtab: " << parts[1] << endl;
        exit(1);
      }
      
      FrameInfo info(module, offset, parts[0], line_num, parts[2]);
      frames[key] = info;
    }
  }


  FrameDB *FrameDB::load_from_file(const string& filename) {
    ifstream vd(filename.c_str());
    if (vd.fail()) {
      return NULL;
    }

    auto_ptr<FrameDB> db(new FrameDB());

    // if we found the viewer data file build up the map.
    string line;
    while (getline(vd, line)) {
      if (line.find("=>") != string::npos) {
        db->add_alias(line);
      } else {
        db->add_info(line);
      }
    }

    cerr << "loaded " << db->size() << " frames/aliases" << endl;
    
    return db.release();
  }

} // namespace effort
