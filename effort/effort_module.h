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
#ifndef EFFORT_MODULE_H
#define EFFORT_MODULE_H

#include <mpi.h>
#include "effort_api.h"

/// Name of this module to be requested by users.
extern const char *const PNMPI_MODULE_EFFORT;

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

  /// Registration point for PNMPI module -- inits everything
  /// and requests service modules that this one uses.
  int PNMPI_RegistrationPoint();
  
  
  /// Called by wrappers to enter comm region
  void effort_enter_comm();


  /// Called by wrappers to leave comm region
  void effort_exit_comm();
  

  // --- below are MPI-related routines --- //

  /// To be called before MPI is inited
  void effort_preinit();

  /// To be called after MPI is inited but before other MPI calls
  void effort_postinit();

  /// To be called from MPI_Pcontrol.
  void effort_pcontrol(int level);

  /// To be called just before MPI is finalized.
  void effort_finalize();
  
  /// This is defined if we are using PNMPI_EFFORT (no PnMPI)
  void effort_do_stackwalk();

  /// temporary addition for topo experiment
  void effort_set_dims(size_t x, size_t y, size_t z);
  

#ifdef __cplusplus
}
#endif //__cplusplus


#endif // EFFORT_MODULE_H
