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
#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <map>
using namespace std;

typedef map<int, size_t> histogram_t;
histogram_t counts;

int MPI_Init(int *argc, char ***argv) {
  int return_val = PMPI_Init(argc, argv);

  int rank;
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    printf("=================================================\n");
    printf("=  Running with MPI_Pcontrol() counter module.  =\n");
    printf("=================================================\n");
  }

  return return_val;
}

int MPI_Pcontrol(const int type, ...) {
  counts[type]++;
  return 0;
}


int MPI_Finalize() {
  int rank;
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  if (rank == 0) {
    cout << "Calls to MPI_Pcontrol(int)\n";
    cout << setw(5) << "TYPE" << setw(12) << "COUNT" << endl;
    for (histogram_t::iterator i=counts.begin(); i != counts.end(); i++) {
      cout << setw(5) << i->first << setw(12) << i->second << endl;
    }
  }
  
  return PMPI_Finalize();
}
