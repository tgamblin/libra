#include "effort_record.h"

namespace effort {
  
  void effort_record::commit(size_t progress_count) {
    // should be one less than timestep about to be recorded
    if (values.size() < progress_count) values.resize(progress_count);
    values.push_back(current);
    current = 0;
  }

} // namespace

