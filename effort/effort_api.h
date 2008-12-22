#ifndef EFFORT_API_H
#define EFFORT_API_H

#include <stdlib.h>

///\file effort_api.h
///
/// This file defines the external API for the effort library.
/// This provides two functions: one for incrementing a progress step
/// and another for manually providing effort values on each 
/// progress step.
///

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

  /// Advances the trace one progress step.  User must ensure that this is
  /// called the same number of times on each process.
  void progress_step();

  /// This should be called within progress steps to accumulate time (or some other
  /// metric) spent in effort regions.  Metrics are kept track of by id (position
  /// in the vector), and the runtime keeps per-id traces.  At the end of each timestep,
  /// the sum of all counter values within that timestep is appended to the trace for
  /// each metric.
  void record_effort(size_t count, double *counter_values);
  
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // EFFORT_API_H
