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
#include "effort_module.h"

#include <mpi.h>

#include <unistd.h>
#include <cerrno>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include "timing.h"

static double init_time;           /// Time MPI_Init ran.
static double finalize_time;       /// Time MPI_Finalize ran.

string working_dir;

static string get_wd() {
  vector<char> tmp_wd(1024);
  while (NULL == getcwd(&tmp_wd[0], tmp_wd.size() - 1)) {
    if (errno != ERANGE) {
      cerr << "Error: Effort module can't get working directory." << endl;
      exit(1);
    }
    tmp_wd.resize(tmp_wd.size() * 2);
  }
  return string(&tmp_wd[0]);
}


extern "C" int MPI_Init(int *argc, char ***argv) {
  working_dir = get_wd();
  init_time = get_time_ns();
  return PMPI_Init(argc, argv);
}


static void dump_timing() {
  int size;
  PMPI_Comm_size(MPI_COMM_WORLD, &size);
  
  // print out all the callpaths for each process
  ostringstream fn;
  fn << working_dir << "/times-" << size;
  ofstream times(fn.str().c_str());
  
  times << "APP:\t"   << finalize_time - init_time << endl;
  times << "TOTAL:\t" << finalize_time - init_time << endl;
}


extern "C" int MPI_Finalize() {
  int rank;
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  finalize_time = get_time_ns();

  // dump times on rank 0.
  if (rank == 0) {
    dump_timing();
  }

  return PMPI_Finalize();
}
