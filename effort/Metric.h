#ifndef METRIC_H
#define METRIC_H

#include "UniqueId.h"

namespace effort {

  /// Metrics are string identifiers, but we unique them for fast compares by 
  /// deriving from UniqueId
  class Metric : public UniqueId<Metric> {
  public:
    Metric(const std::string& id);

    /// Time metric provided for convenience here.
    static const Metric& time() {
      static Metric time_metric("time");
      return time_metric;
    }
  };

} // namespace effort

#endif // METRIC_H
