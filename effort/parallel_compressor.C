#include "parallel_compressor.h"

#include <sstream>
#include <algorithm>
#include <fstream>
using namespace std;

#include "wt_parallel.h"
#include "par_ezw_encoder.h"
#include "io_utils.h"
using namespace wavelet;

#include "synchronize_keys.h"
#include "effort_metadata.h"
#include "effort_module.h"

namespace effort {

  parallel_compressor::parallel_compressor(const effort_params& p) : params(p) { }

  void parallel_compressor::do_compression(wavelet::wt_matrix& mat, effort_key key, int id, MPI_Comm comm) {
    int rank, size;
    PMPI_Comm_rank(comm, &rank);
    PMPI_Comm_size(comm, &size);
  
    // if verify is on, then output exact data in a separate directory.
    if (params.verify) {
      // write out exact data to a file for verification later.
      ostringstream exact_file_name;
      exact_file_name << exact_dir 
                      << "/exact" 
                      << "-" << id_to_metric_name(key.metric) 
                      << "-" << key.type 
                      << "-" << id 
                      << "-" << rank;
      ofstream exact_file(exact_file_name.str().c_str());
      output(mat, exact_file);
      exact_file.close();
    }
  
    // Do wavelet transform in parallel
    wt_parallel pwt;
    int level = pwt.fwt_2d(mat, -1, comm);
  
    // Encode transformed data in parallel
    par_ezw_encoder encoder;
    encoder.set_pass_limit(params.pass_limit);
    encoder.set_use_sequential_order(params.sequential);
    encoder.set_scale(params.scale);
    encoder.set_encoding_type(str_to_encoding(params.encoding));

    ofstream encoded_stream;
    if (rank == encoder.get_root(comm)) {
      // open the encoded file stream on the root process
      ostringstream filename;
      filename << output_dir 
               << "/effort" 
               << "-" << id_to_metric_name(key.metric)
               << "-" << key.type 
               << "-" << id;
      encoded_stream.open(filename.str().c_str());

      // output the effort metadata (type, callpaths) first
      effort_metadata md(id_to_metric_name(key.metric), 
                         key.type, key.start_path, key.end_path);
      md.write_out(encoded_stream);
    }

    encoder.encode(mat, encoded_stream, level, comm);
  }



  void parallel_compressor::compress(effort_data& effort_log, MPI_Comm comm_world) {
    int rank, size;
    PMPI_Comm_rank(comm_world, &rank);
    PMPI_Comm_size(comm_world, &size);

    // Skip this if we don't have a power of 2 process count.
    if (!isPowerOf2(size)) {
      if (rank == 0) {
        cerr << "WARNING: Skipping compression due to non-power-of-2 process count." << endl;
      }
      return;
    }

    // Filter out everything that lacks all the progress iterations.
    // TODO: figure out why these pop up at the end of a trace.
    for (effort_map::iterator entry = effort_log.begin(); entry != effort_log.end(); ) {
      effort_map::iterator old = entry++;
      if (old->second.values.size() == 0) {
        effort_log.emap.erase(old);
      }
    }

    if (rank == 0) {
      cerr << effort_log.progress_count << " progress steps." << endl;
    }

    // Make sure progress count is a power of two.
    // TODO: if we have to pad, make a note of it in the metadata and update 
    // TODO: decoder, iwt, etc. to take out padding on expansion.
    if (!isPowerOf2(effort_log.progress_count)) {
      // step to the next power of 2 timestep and fill with zeros.
      effort_log.progress_step(gePowerOf2(effort_log.progress_count));
    }

    // first need to synchronize effort keys across all processors so that everyone
    // participates in the right transforms.  Otherwise traversals of the effort
    // map will be different depending on the node.
    size_t unsynced = effort_log.size();
    synchronize_effort_keys(effort_log, comm_world);
    if (rank == 0) {
      size_t synced = effort_log.size();
      cerr << synced << " total effort regions (" << (synced - unsynced) << " new from sync)." << endl;
    }
  
    // now we traverse the effort map and do a transform for each type of effort 
    // we encountered.  We farm these out to different modulo sets of the cluster.
    // We wait on communication and do all the transforms when all sets are full,
    // and we continue when all the effort has been transformed.
    const int m = min(params.rows_per_process, size);
  
    // create separate wavelet transform communicators
    MPI_Comm comm;
    PMPI_Comm_split(comm_world, rank % m, 0, &comm);

    wavelet::wt_matrix mat;    // local WT storage
    vector<MPI_Request> reqs;  // outstanding request storage.
    effort_key set_to_key[m];  // mapping from sets to their effort keys.
    size_t set_to_id[m];       // mapping from sets to their ids.

    // Vector to hold keys in identical order across processes
    vector<effort_key> sorted_keys;

    // Dump keys into the vector.
    sorted_keys.reserve(effort_log.size());
    for (effort_map::iterator e = effort_log.begin(); e != effort_log.end(); e++) {
      sorted_keys.push_back(e->first);
    }

    // Sort vector using heavy key comparison (cmpares by all frames, full module names, offsets)
    sort(sorted_keys.begin(), sorted_keys.end(), effort_key_full_lt());
    for (size_t id=0; id < sorted_keys.size(); /* id is incremented in inner loop */) {
      // this loop farms out work to sets of processors
      int set;
      for (set=0; set < m && id < sorted_keys.size(); set++, id++) {
        effort_key& key = sorted_keys[id];
        effort_record& record = effort_log[key];

        // sanity check for values from all timesteps
        if (record.values.size() != effort_log.progress_count) {
          cerr << "ERROR: vector size was " << record.values.size()
               << "   expected: " << effort_log.progress_count
               << "   for key   " << key << endl;
          exit(1);
        }

        // record the key for this set
        set_to_key[set] = key;
        set_to_id[set] = id;

        // consolidate all data for the set onto its processors
        wt_parallel::aggregate(mat, record.values, m, set, reqs, comm_world);      
      }

      // we've farmed out enough work for all procs; need to do transforms
      if (reqs.size() || size == 1) { 
        if (reqs.size()) {
          MPI_Status statuses[reqs.size()];
          PMPI_Waitall(reqs.size(), &reqs[0], statuses);
          reqs.clear();
        }
      
        if (rank % m < set) {
          do_compression(mat, set_to_key[rank % m], set_to_id[rank % m], comm);
        }
      }
    }
  }

} //namespace
