#include "effort_data.h"

#include <algorithm>
#include <cassert>
using namespace std;

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
  

} // namespace
