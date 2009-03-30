#include "parallel_compressor.h"

#include <sstream>
#include <algorithm>
#include <fstream>
#include <cmath>
using namespace std;

#include "wt_parallel.h"
#include "par_ezw_encoder.h"
#include "io_utils.h"
using namespace wavelet;

#include "synchronize_keys.h"
#include "timing.h"
#include "ltqnorm.h"
#include "random.h"

namespace effort {

  parallel_compressor::parallel_compressor(const effort_params& p) 
    : params(p), file_map(NULL), confidence(0.9), error(0.3) { }

  void parallel_compressor::do_compression(wavelet::wt_matrix& mat, effort_key key, int id, MPI_Comm comm) {
    int rank, size;
    PMPI_Comm_rank(comm, &rank);
    PMPI_Comm_size(comm, &size);

    ostringstream sfilename;
    if (file_map) {
      map<effort_key, string>::const_iterator i = file_map->find(key);
      if (i == file_map->end()) {
        sfilename << "unknown-" << key.metric << "-" << key.type << "-" << id;
      } else {
        sfilename << i->second;
      }
    } else {
      sfilename << "effort-" << key.metric << "-" << key.type << "-" << id;
    }
    string effort_filename = sfilename.str();

    // if verify is on, then output exact data in a separate directory.
    if (params.verify) {
      // write out exact data to a file for verification later.
      ostringstream exact_file_name;
      exact_file_name << exact_dir << "/exact-" << effort_filename << "-" << rank;
      ofstream exact_file(exact_file_name.str().c_str());
      output(mat, exact_file);
      exact_file.close();
      timer.record("ExactData");
    }
  
    // Do wavelet transform in parallel
    wt_parallel pwt;
    int level = pwt.fwt_2d(mat, -1, comm);
    timer.record("WaveletTransform");
  
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
      filename << output_dir << "/" << effort_filename;
      encoded_stream.open(filename.str().c_str());

      // output the effort id (type, callpaths) first
      key.write_out(encoded_stream);
    }

    timer.record("OpenOutputFiles");

    encoder.encode(mat, encoded_stream, level, comm);
    timer += encoder.get_timer();  // include ezw timings.
  }


  /// sets confidence interval for sample
  /// confidence should be in (0, 1]
  void set_confidence(double confidence);
  
  /// sets confidence interval for sample
  /// error should be in [0. 1)
  void set_error(double error);


  struct sample_elt {
    int rank;
    double value;

    sample_elt(int r, double v) : rank(r), value(v) { }
    sample_elt(const sample_elt& other) : rank(other.rank), value(other.value) { }

    sample_elt& operator=(const sample_elt& other) {
      rank = other.rank;
      value = other.value;
      return *this;
    }
  };

  bool operator<(const sample_elt& lhs, const sample_elt& rhs) {
    return lhs.value < rhs.value;
  }

  /*
  MPI_Comm parallel_compressor::reorder_ranks_in_bins(effort_record& record, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    // calculate local trace mean.
    double total = 0;
    for (size_t i=0; i < record.values.size(); i++) {
      total += record.values[i];
    }
    double val = total / record.values.size();
    double val2 = val * val;
    
    // calculate variance of the local trace means
    double sum, sum2;
    MPI_Reduce(&val, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, comm);
    MPI_Reduce(&val2, &sum2, 1, MPI_DOUBLE, MPI_SUM, 0, comm);

    // take random sample according to min size then bcast sample.
    vector<int> sample_ranks;
    vector<double> sample;
    size_t min_sample_size;
    if (rank == 0) {
      double mean = sum / size;
      double variance = (sum2 - (sum * sum)/size) / size;

      // calculate min sample size
      double Za = computeConfidenceInterval(confidence);   // double-tailed norm conf. interval
      double d = mean * error;                              // real error bound (error var is a %)
      double stdDev = sqrt(variance);
      
      double V = (d/(Za*stdDev));
      min_sample_size = llround(size * 1/(1 + size * V*V));

      // take a random sample and sort it so it's the sample across all nodes.
      randomSubset(size, min_sample_size, inserter(sample_ranks, sample_ranks.begin()));
      
      ostringstream strm;
      strm << "comm: " << hex << comm 
           << "   numvalues: " << record.values.size() 
           << "   sum: " << sum
           << "   sum2: " << sum2
           << "   mean: " << mean
           << "   variance: " << variance
           << "   size: " << size
           << "   stdDev: " << stdDev 
           << "   min_sample_size: " << min_sample_size << endl
           << "   sample.size(): " << sample.size();
      cerr << strm.str();
    }

    // bcast sample vector to nodes to get new order    
    MPI_Bcast(&min_sample_size, 1, MPI_SIZE_T, 0, comm);
    MPI_Bcast(&sample_ranks[0], min_sample_size, MPI_DOUBLE, 0, comm);
    
    // bcast each sample value to sync the sample vector
    for (size_t i=0; i < min_sample_size; i++) {
      if (rank == sample_ranks[i]) sample[i] = val;
      MPI_Bcast(&sample[i], 1, MPI_DOUBLE, sample_ranks[i], comm);
    }

    // sort bins by value
    vector<sample_elt> sorted_sample(min_sample_size);
    for (size_t i=0; i < min_sample_size; i++) {
      sorted_sample[i] = sample_elt(sample_ranks[i], sample[i]);
    }
    sort(sorted_sample.begin(), sorted_sample.end());

    // find closest bin to local value.
    sample_elt me(rank, val);
    vector<sample_elt>::iterator binit = lower_bound(sorted_sample.begin(), sorted_sample.end(), me);
    int mybin;
    if (binit == sorted_sample.end()) {
      mybin = sorted_sample.back().rank;
    } else {
      mybin = binit->rank;
    }
    
    size_t myindex = find(mybin, sample_ranks.begin(), sample_ranks.end()) - sample_ranks.begin();
    
    // do another comm_split with color same for everyone but 
    // rank = indexof(closest sample vector elt) * size + rank
    // this should put things in bin order
    MPI_Comm_split();
  }
  */

  void parallel_compressor::compress(effort_data& effort_log, MPI_Comm comm_world) {
    timer.clear();

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

    timer.record("LogCheck");
    
    // first need to synchronize effort keys across all processors so that everyone
    // participates in the right transforms.  Otherwise traversals of the effort
    // map will be different depending on the node.
    size_t unsynced = effort_log.size();
    synchronize_effort_keys(effort_log, comm_world);
    if (rank == 0) {
      size_t synced = effort_log.size();
      cerr << synced << " total effort regions (" << (synced - unsynced) << " new from sync)." << endl;
    }

    timer.record("SyncKeys");
  
    // now we traverse the effort map and do a transform for each type of effort 
    // we encountered.  We farm these out to different modulo sets of the cluster.
    // We wait on communication and do all the transforms when all sets are full,
    // and we continue when all the effort has been transformed.
    const int m = min(params.rows_per_process, size);

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
    timer.record("SplitAndDumpKeys");

    // Sort vector using heavy key comparison (cmpares by all frames, full module names, offsets)
    sort(sorted_keys.begin(), sorted_keys.end(), effort_key_full_lt());
    timer.record("SortKeys");



    // reorder bins
    /*
    for (size_t i=0; i < sorted_keys.size(); i++) {
      reorder_ranks_in_bins(effort_log[sorted_keys[i]], comm_world);
    }
    timer.record("ReorderBins");
    */  

    // create separate wavelet transform communicators
    MPI_Comm comm;
    PMPI_Comm_split(comm_world, rank % m, 0, &comm);

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
      timer.record("Aggregate");

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
