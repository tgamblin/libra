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
#include "effort_data.h"

#include <dirent.h>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <sstream>
using namespace std;

#include "ezw.h"
using namespace wavelet;

namespace effort {
  
  effort_data::effort_data() : progress_count(0) { }

  void effort_data::progress_step(size_t step_to) {
    assert(!step_to || step_to > progress_count);

    // get ready to step to a specified timestep if one was provided
    if (step_to > 0) {
      progress_count = step_to - 1;
    }

    // commit all the effort for this timestep
    for_each(emap.begin(), emap.end(), committer(progress_count));
    progress_count++;
  }

  
  void effort_data::load_keys(const string& dirname, effort_data& log, 
                              wavelet::ezw_header& header, map<effort_key, string> *filenames) {
    DIR *dirp = opendir(dirname.c_str());
    effort_key key;
    bool first = true;
    for (dirent *dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
      if (parse_filename(dp->d_name)) {
        ostringstream fullpath;
        fullpath << dirname << "/" << dp->d_name;
        ifstream file(fullpath.str().c_str());
        
        effort_key::read_in(file, key);
        log[key] = effort_record();
        
        if (filenames) {
          pair<effort_key, string> entry(key, string(dp->d_name));
          filenames->insert(entry);
        }

        if (first) {
          ezw_header::read_in(file, header);
          first = false;
        }
      }
    }
  }


  void effort_data::write_current_step(std::ostream& out) {
    ostringstream buffer;
    buffer << "STEP " << progress_count << endl;
    for_each(emap.begin(), emap.end(), write_current(buffer, "    "));
    out << buffer.str();
  }

} // namespace
