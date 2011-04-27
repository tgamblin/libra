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

  ///////////////////////////////////////////////////////////////////////////
  // Application API -- can be called by applications measured using Libra
  ///////////////////////////////////////////////////////////////////////////

  /// Advances the trace one progress step.  User must ensure that this is
  /// called the same number of times on each process.
  void progress_step();

  /// Inits manual effort library with a list of metric names.
  void init_metrics(size_t metric_count, const char **metric_names);

  /// This should be called within progress steps to accumulate user metrics.
  /// Metrics should correspond to the names passed to init_metrics().
  void record_effort(const double *metric_values);
  
#ifdef __cplusplus
}
#endif // __cplusplus


///////////////////////////////////////////////////////////////////////////
// Tool API -- to be called by other tools working with Libra at runtime.
// Note that these have C lnkage but take a C++ data type.
///////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
#include "effort_data.h"
namespace effort {
  ///
  /// Handler type for functions that want to listen to progress events.
  ///
  typedef void (*progress_listener_t)(effort::effort_data& data);
	typedef void (*register_progress_listener_t)(effort::progress_listener_t listener, int frequency);
 
} // namespace effort

extern "C" {
  ///
  /// Register a listener function to be called every <frequency> progress steps.
  /// Subsequent registrations of the same functions will NOT add new handlers.  
  /// Rather, they will change (and reset) the frequency of the existing registration.
  ///
  /// Note that the effort_data passed in should *not* be modified by the listener,
  /// and it may be converted to a const reference in future versions of this API.
  /// 
  /// Note also that there are no guarantees about the order that registered 
  /// listeners will be called in each timestep, so do not depend on any particular
  /// order when you register listeners!
  ///
  void effort_register_progress_listener(effort::progress_listener_t listener, int frequency = 1);
  
  ///
  /// Remove a listener function from the list of registered listeners.
  /// After this call, the listener will no longer be notified of progress steps.
  /// 
  void effort_remove_progress_listener(effort::progress_listener_t listener);
}
#endif // __cplusplus
#endif // EFFORT_API_H
