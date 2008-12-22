#ifndef SYNCHRONIZE_EFFORT_KEYS_H
#define SYNCHRONIZE_EFFORT_KEYS_H

#include <mpi.h>
#include "effort_data.h"

///\file synchronize_keys.h
/// 
/// Contains declarations for routines involved in global reduction done before
/// compression.  These routines merge sets of effort keys across processors
/// so that all processors end up with the same set in the end.
/// 
/// This is necessary for compression so that all processors end up transforming
/// data from the same effort region at the same time.
/// 
namespace effort {

  /// Receives a set of keys from another processor via PMPI and
  /// Merges all of them into the supplied effort map.
  void receive_keys(effort_data& effort_log, int src, MPI_Comm comm);
  
  /// Sends a set of effort_keys to another processor via PMPI.
  void send_keys(effort_data& effort_log, int dest, MPI_Comm comm);
  
  /// Reduces effort keys so that all processors have the same keys.
  void synchronize_effort_keys(effort_data& effort_log, MPI_Comm comm);
  
} // namespace

#endif // SYNCHRONIZE_EFFORT_KEYS_H
