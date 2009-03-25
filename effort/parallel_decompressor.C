#include "parallel_decompressor.h"

#include <sstream>
#include <map>
#include <algorithm>
#include <fstream>
using namespace std;

#include "wt_parallel.h"
#include "ezw_decoder.h"
#include "io_utils.h"
#include "wt_utils.h"
using namespace wavelet;

#include "synchronize_keys.h"
#include "timing.h"

namespace effort {

  /// broadcasts a vector of strings from root to other nodes.
  static void bcast_strings(vector<string>& strings, MPI_Comm comm) {
    int rank, size;
    PMPI_Comm_rank(comm, &rank);
    PMPI_Comm_size(comm, &size);
    
    size_t num_strings = strings.size();
    MPI_Bcast(&num_strings, 1, MPI_SIZE_T, 0, comm);
    
    relatives rels = get_radix_relatives(rank, size);
    if (rels.parent >= 0) strings.clear();  // empty vector on non-root
    
    vector<char> buf;    // tmp storage    
    for (size_t i=0; i < num_strings; i++) {
      if (rels.parent >= 0) {
        size_t size;
        MPI_Recv(&size, 1, MPI_SIZE_T, rels.parent, 0, comm, MPI_STATUS_IGNORE);
        
        if (buf.size() < size + 1) buf.resize(size + 1);

        MPI_Recv(&buf[0], size, MPI_CHAR, rels.parent, 0, comm, MPI_STATUS_IGNORE);
        buf[size+1] = '\0';
        strings.push_back(string(&buf[0]));
      }

      if (rels.left >= 0)  {
        size_t size = strings[i].size();
        MPI_Send(&size,             1, MPI_SIZE_T, rels.left, 0, comm);
        MPI_Send(&(strings[i][0]), size, MPI_CHAR,   rels.left, 0, comm);
      }

      if (rels.right >= 0) {
        size_t size = strings[i].size();
        MPI_Send(&size,             1, MPI_SIZE_T, rels.right, 0, comm);
        MPI_Send(&(strings[i][0]), size, MPI_CHAR,   rels.right, 0, comm);
      }
    }
  }
  


  parallel_decompressor::parallel_decompressor() : input_dir(".") { }


  void parallel_decompressor::do_decompression(wavelet::wt_matrix& mat, effort_key key, 
                                               const string& filename, MPI_Comm comm) {
    int rank, size;
    PMPI_Comm_rank(comm, &rank);
    PMPI_Comm_size(comm, &size);

    int level = -1;
    if (rank == 0) {
      ifstream file(filename.c_str());
      ezw_decoder decoder;
      
      effort_key key;
      effort_key::read_in(file, key); // todo verify here.x
      
      level = decoder.decode(file, mat);
    }

    // Do wavelet transform in parallel
    wt_parallel wt;
    wt.iwt_2d(mat, level, comm);
  }


  void parallel_decompressor::decompress(effort_data& effort_log, MPI_Comm comm_world) {
    int rank, size;
    PMPI_Comm_rank(comm_world, &rank);
    PMPI_Comm_size(comm_world, &size);

    // Skip this if we don't have a power of 2 process count.
    if (!isPowerOf2(size)) {
      if (rank == 0) {
        cerr << "WARNING: Skipping decompression due to non-power-of-2 process count." << endl;
      }
      return;
    }

    size_t progress_steps = 0;
    blocks = 0;
    map<effort_key, string> file_for_key;

    if (rank == 0) {
      ezw_header header;
      effort_data::load_keys(input_dir, effort_log, header, &file_for_key);

      if (header.rows != size) {
        cerr << "Error: attempted to load " << header.rows << " rows to " << size << " processors." << endl;
        exit(1);
      }

      progress_steps = header.cols;
      blocks = header.blocks;
    }

    /// synch up the relevant parts of the header.
    synchronize_effort_keys(effort_log, comm_world);
    MPI_Bcast(&progress_steps, 1, MPI_SIZE_T, 0, comm_world);
    MPI_Bcast(&blocks,         1, MPI_SIZE_T, 0, comm_world);

    // step to the next power of 2 timestep and fill with zeros.
    effort_log.progress_step(gePowerOf2(progress_steps));

    const int m = size / blocks;
  
    // create separate wavelet transform communicators
    MPI_Comm comm;
    PMPI_Comm_split(comm_world, rank % m, 0, &comm);

    wavelet::wt_matrix mat;    // local WT storage
    vector<MPI_Request> reqs;  // outstanding request storage.

    // Vector to hold keys in identical order across processes
    vector<effort_key> sorted_keys;
    
    // Dump keys into the vector.
    sorted_keys.reserve(effort_log.size());
    for (effort_map::iterator e = effort_log.begin(); e != effort_log.end(); e++) {
      sorted_keys.push_back(e->first);
    }
    sort(sorted_keys.begin(), sorted_keys.end(), effort_key_full_lt());

    // Mapping from keys to filenames.  Need this in decompressor because user might've 
    // renamed/moved some files.  The id appended to files by the compressor is arbitrary anyway.
    vector<string> filenames;
    if (rank == 0) {
      for (size_t i=0; i < sorted_keys.size(); i++) {
        filenames.push_back(file_for_key[sorted_keys[i]]);
      }
    }
    bcast_strings(filenames, comm_world);
    
    // Sort vector using heavy key comparison (cmpares by all frames, full module names, offsets)
    sort(sorted_keys.begin(), sorted_keys.end(), effort_key_full_lt());
    for (size_t id=0; id < sorted_keys.size(); /* id is incremented in inner loop */) {
      // this loop farms out work to sets of processors
      int set;
      for (set=0; set < m && id < sorted_keys.size(); set++, id++) {
        effort_key& key = sorted_keys[id];
        effort_record& record = effort_log[key];

        if (rank % m == set) {
          do_decompression(mat, key, filenames[id], comm);
        }

        // consolidate all data for the set onto its processors
        wt_parallel::distribute(mat, record.values, m, set, reqs, comm_world);      
      }

      // we've farmed out enough work for all procs; wait for distributes to finish. 
      if (reqs.size()) {
        MPI_Status statuses[reqs.size()];
        PMPI_Waitall(reqs.size(), &reqs[0], statuses);
        reqs.clear();
      }
    }
  }

} //namespace
