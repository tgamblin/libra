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
  
  compressor.compress(effort_log, MPI_COMM_WORLD);

  
  MPI_Finalize();
}

