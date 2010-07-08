/////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010, Lawrence Livermore National Security, LLC.  
// Produced at the Lawrence Livermore National Laboratory  
// Written by Todd Gamblin, tgamblin@llnl.gov.
// LLNL-CODE-417602
// All rights reserved.  
// 
// This file is part of Libra. For details, see http://github.com/tgamblin/libra.
// Please also read the LICENSE file for further information.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the disclaimer below.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the disclaimer (as noted below) in the documentation and/or other materials
//    provided with the distribution.
//  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
//    or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
/////////////////////////////////////////////////////////////////////////////////////////////////
#include "FrameDB.h"
#include <memory>
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
    
    return db.release();
  }

} // namespace effort
