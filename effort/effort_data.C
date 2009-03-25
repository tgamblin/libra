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
        
        if (filenames) (*filenames)[key] = fullpath.str();

        if (first) {
          ezw_header::read_in(file, header);
          cerr << header.cols << endl;
          first = false;
        }
      }
    }
  }
  

} // namespace
