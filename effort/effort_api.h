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

  /// Inits manual effort library with a list of metric names.
  /// This needs to be called before effort initialization (before MPI_Init())
  void init_metrics(size_t metric_count, const char **metric_names);

  /// This should be called within progress steps to accumulate elapsed metrics in effort regions.
  /// Metrics are assumed to be positionally correlated with the names passed to init_effort, and
  /// metric_values is assumed to have at least as many elements as the metric_count passed to 
  /// init_effort.
  void record_effort(double *metric_values);
  
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // EFFORT_API_H
