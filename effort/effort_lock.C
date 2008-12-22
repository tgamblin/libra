#include "effort_lock.h"

namespace effort {

  // Static variable to determine when we're in an effort region.
  bool effort_lock::lock = false;

} // namespace
