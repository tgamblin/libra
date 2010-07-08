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

#include <fstream>
#include <sstream>
#include <cstdlib>
using namespace std;

#include "wavelet.h"
#include "matrix_utils.h"
#include "effort_api.h"
using namespace wavelet;

#ifndef SRC_DIR
#define SRC_DIR ""
#endif

/// Try to find bunny.dat in gnu standard location, $(srcdir).
/// Expect this to be passed in as env var or via -DSRC_DIR=
/// at compile time.  Failing those, try the working directory.
void find_bunny(string& location) {
  ostringstream bunny_path;

  if (getenv("srcdir")) {
    bunny_path << getenv("src_dir");
  } else if (strlen(SRC_DIR)) {
    bunny_path << SRC_DIR;
  } else {
    bunny_path << ".";
  }
  bunny_path << "/bunny.dat";

  location = bunny_path.str();
}


/// Loads a height-map of the Stanford bunny from a file.  Outputs 
/// synthetic, bilinear-interpolated data from this file to a trace
/// using the effort API.  If the trace looks like a bunny, you know
/// this test worked.
int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  const char* metrics[] = {"Bunny", "BunnyFlipped", "Bunny2x", "Bunny0.5x"};
  const size_t num_metrics = sizeof(metrics) / sizeof(char*);
  init_metrics(num_metrics, metrics);

  // allow -p <num> to print out a particular matrix
  int print = -1;
  if (argc > 1 && string(argv[1]) == "-p") {
    if (argc > 2) {
      print = atoi(argv[2]);
      if (print >= (int)num_metrics) {
        print = 0;
      } else if (print < 0) {
        print = -1;
      }
    }
  }

  // read in file with bunny image.
  wt_matrix bunny;

  string bunny_data;
  find_bunny(bunny_data);
  if (!read_matrix(bunny_data.c_str(), bunny)) {
    if (rank == 0) {
      cerr << "Can't find file '" << bunny_data << "'" << endl;
    }
    exit(1);
  }

  // figure out scaling factor from image size (128x128) to (size x size)
  double scale = ((double)bunny.size1() / size);
  double x = rank * scale;  
  
  if (print >= 0 && rank == 0) {
    cerr << metrics[print] << ":" << endl;
  }

  double values[num_metrics];
  for (int i = 0; i < size; i++) {
    double y = i * scale;

    // record effort every timestep so as to make a 3d bunny with
    // the heightmap values. Record transpose and scaled versions to 
    // test recording of multiple values at once.
    values[0] = interp_bilinear(bunny, x, y);
    values[1] = interp_bilinear(bunny, y, x);
    values[2] = 2.0 * values[0];
    values[3] = 0.5 * values[0];


    if (print >= 0) {
      double val = values[print];
      double vals[size];
      MPI_Gather(&val, 1, MPI_DOUBLE,
                 vals, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
      
      if (rank == 0) {
        for (size_t i=0; i < (size_t)size; i++) {
          cerr << setw(5) << vals[i] << " ";
        }
        cerr << endl;
      }
    }

    record_effort(values);
    progress_step();
  }

  MPI_Finalize();
}


