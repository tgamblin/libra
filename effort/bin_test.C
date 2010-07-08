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
#include <sys/stat.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#include "FrameId.h"
#include "ModuleId.h"
#include "Metric.h"
#include "Timer.h"
#include "io_utils.h"
using namespace wavelet;

#include "effort_data.h"
#include "effort_params.h"
#include "parallel_compressor.h"
#include "parallel_decompressor.h"
using namespace effort;



int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int rank, size;
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  PMPI_Comm_size(MPI_COMM_WORLD, &size);

  if (!isPowerOf2(size)) {
    if (rank == 0) {
      cerr << "Error: Process count is not a power of 2!" << endl;
    }
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  if (argc != 2) {
    if (rank == 0) {
      cerr << "Usage: bin_test <dir>" << endl;
      cerr << "  Where <dir> is a directory full of effort files." << endl;
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
  }
  string input_dir(argv[1]);
    
  // load up an effort log with stored data.
  effort_data effort_log;
  parallel_decompressor decompressor;
  decompressor.set_input_dir(input_dir);
  decompressor.decompress(effort_log, MPI_COMM_WORLD);
  
  ostringstream soutput_dir;
  soutput_dir << input_dir << "/output";
  string output_dir = soutput_dir.str();

  effort_params params;
  params.rows_per_process = size / decompressor.get_blocks();
  
  parallel_compressor compressor(params);
  mkdir(output_dir.c_str(), 0750);
  compressor.set_output_dir(output_dir);
  compressor.set_file_map(decompressor.file_map());
  
  compressor.compress(effort_log, MPI_COMM_WORLD);

  
  MPI_Finalize();
}

