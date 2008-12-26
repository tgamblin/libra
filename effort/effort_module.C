#include "effort_module.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

#ifdef HAVE_LIBPAPI
#include <papi.h>
#endif //HAVE_LIBPAPI

#ifndef PMPI_EFFORT
#include "pnmpimod.h"
#endif // PMPI_EFFORT

#include "env_config.h"

#include "timing.h"
#include "io_utils.h"
#include "mpi_utils.h"
using namespace wavelet;

#include "Callpath.h"
#include "CallpathRuntime.h"
#include "callpath_module.h"

#include "effort_data.h"
#include "effort_lock.h"
#include "effort_params.h"
#include "parallel_compressor.h"
using namespace effort;

#include "string_utils.h"
using namespace stringutils;

const char *const PNMPI_MODULE_EFFORT = "pnmpi-module-effort";

/// Contains all our stakcwalking support
CallpathRuntime runtime;

/// Defaults are overridden (if provided by config) in PNMPI_RegistrationPoint.
effort_params params;

/// Arguments for PNMPI configuration.
config_desc config_arguments[] = {
  config_desc("rows_per_process",  &params.rows_per_process),
  config_desc("verify",            &params.verify),
  config_desc("pass_limit",        &params.pass_limit),
  config_desc("scale",             &params.scale),
  config_desc("sequential",        &params.sequential),
  config_desc("encoding",          &params.encoding),
  config_desc("metrics",           &params.metrics),
  config_desc("chop_libc",         &params.chop_libc),
  config_desc("regions",           &params.regions),
  config_desc("sampling",          &params.sampling),
  config_desc()
};

typedef enum {
  REGIONS_EFFORT, REGIONS_COMM, REGIONS_BOTH, REGIONS_INVALID
} regions_t;

/// Running records of effort values per progress step, keyed by effort region.
static effort_data effort_log;       /// Cumulative effort data through entire run.
static int cur_effort_type = 0;

static Callpath start_callpath;      /// Effort region start callpath -- initially empty.
static Callpath *pnmpi_callpath;     /// Global callpath pointer from PnMPI.
static Callpath pmpi_only_callpath;  /// Storage for callpath if no PnMPI -- initially empty.

static string working_dir;           /// Application's working directory, assessed at registration.
static string effort_dir;            /// Location of effort data output, inited in MPI_Finalize().
static string exact_dir;             /// Location of exact (verification) data, inited in MPI_Finalize().

static double init_time;             /// Time MPI_Init ran.
static double finalize_time;         /// Time MPI_Finalize ran.

// Storage and metadata for PAPI counters
static bool keep_time = true;        /// Whether to record wall-clock timing as a metric
static double start_time = -1;       /// Start time for system clock.

static size_t sample_count;          /// Sample count for progress steps
static regions_t regions;            /// region collection mode (effort, comm, or both)

static vector<string> metric_names;  /// Mapping from id to metric name (PAPI or otherwise)
static vector<long long> counters;   /// HW counter value storage for PAPI

#ifdef HAVE_LIBPAPI
static int event_set = PAPI_NULL;    /// PAPI event set.
#endif //HAVE_LIBPAPI


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


string id_to_metric_name(int id) {
  if (id < 0) {
    return METRIC_TIME;
  } else if (id < metric_names.size()) {
    return metric_names[id];
  } else {
    ostringstream idstr;
    idstr << id;
    return idstr.str();
  }
}


regions_t str_to_regions(const char *str) {
  if (strcasecmp(str, "effort") == 0) {
    return REGIONS_EFFORT;
  } else if (strcasecmp(str, "comm") == 0) {
    return REGIONS_COMM;
  } else if (strcasecmp(str, "both") == 0) {
    return REGIONS_BOTH;
  } else {
    return REGIONS_INVALID;
  }
}


#ifndef HAVE_LIBPAPI

static void metric_setup() { }

#else // HAVE_LIBPAPI
static void error(const char *msg) {
  int rank;
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    cerr << msg << endl;
  }
  exit(1);
}

#define ERROR(x) do { \
  ostringstream estream; \
  estream << x << endl; \
  error(estream.str().c_str()); \
} while (0)

///
/// Initializes PAPI library and sets up performance metrics.
///
static void metric_setup() {
  // First determine if we need PAPI or not by looking through 
  // the metrics the user wants.
  split(params.metrics, " ,", metric_names);

  // move time first if it's there 
  vector<string>::iterator todel = remove_if(metric_names.begin(), metric_names.end(), 
                                             bind2nd(equal_to<string>(), METRIC_TIME));
  if (todel == metric_names.end()) {
    keep_time = false;
  } else {
    keep_time = true;
    metric_names.erase(todel, metric_names.end());
  }
  counters.resize(metric_names.size());

  // if there are no hardware metrics, then just skip this.
  if (counters.size() == 0) return;

  // Initialize the PAPI library 
  int retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT && retval > 0) {
    ERROR("PAPI library version mismatch!");
  }

  if (retval < 0) {
    ERROR("PAPI Initialization error!");
  }
  
  // Create an EventSet
  if (PAPI_create_eventset(&event_set) != PAPI_OK) {
    ERROR("Couldn't create event set!");
  }

  for (size_t i=0; i < metric_names.size(); i++) {
    int event;
    char *name = const_cast<char*>(metric_names[i].c_str());

    if (PAPI_event_name_to_code(name, &event) != PAPI_OK) {
      ERROR("Error adding event: " << name);
    }
    
    // Add event to the event set.
    if (PAPI_add_event(event_set, event) != PAPI_OK) {
      ERROR("Failed to add event to set: " << name);
    }
  }

  if(PAPI_start(event_set) != PAPI_OK) {
    ERROR("Coudln't start event set!");
  }
}
#endif //HAVE_LIBPAPI


void effort_do_stackwalk() {
  pmpi_only_callpath = runtime.doStackwalk(2);
}


/// Creates an effort key and adds the provided delta to the key's entry in the effort log.
static inline void record_metric(const Callpath& start, const Callpath& end, int metric_id, double delta) {
  effort_key key(metric_id, cur_effort_type, start, end);
  effort_log[key] += delta;
}

/// Does the work of inserting the effort key for the current region into the map.
/// Records time, current effort type, and whatever counters are enabled.
static inline void record_region(const Callpath& start, const Callpath& end) {
  if (keep_time) {
    double cur_time = get_time_ns();
    record_metric(start, end, METRIC_TIME_ID, cur_time - start_time);
    start_time = cur_time;
  }

#ifdef HAVE_LIBPAPI
  if (counters.size() != 0) {
    PAPI_accum(event_set, &counters[0]);  // get deltas
    for (size_t i=0; i < metric_names.size(); i++) {
      record_metric(start, end, i, counters[i]);
      counters[i] = 0;
    }
  }
#endif // HAVE_LIBPAPI
}

/// Just resets counters and timers; doesn't record anything.
static inline void reset_counters() {
  if (keep_time) {
    start_time = get_time_ns();
  }
  
#ifdef HAVE_LIBPAPI
  if (counters.size() != 0) {
    PAPI_accum(event_set, &counters[0]);  // get deltas
    for (size_t i=0; i < metric_names.size(); i++) {
      counters[i] = 0;
    }
  }
#endif // HAVE_LIBPAPI
}


void effort_enter_comm() {
  if (regions == REGIONS_EFFORT || regions == REGIONS_BOTH) {
    record_region(start_callpath, *pnmpi_callpath);
  } else {
    reset_counters();
  }
  start_callpath = *pnmpi_callpath;
}


void effort_exit_comm() {
  if (regions == REGIONS_COMM || regions == REGIONS_BOTH) {
    // record comm regions if asked for.
    record_region(start_callpath, start_callpath);
  } else {
    // lump comm into one region if comm is off.
    record_region(Callpath(), Callpath());
  }
}


void progress_step() {
  sample_count--;
  if (sample_count == 0) {
    // commit all effort recorded this iteration and advance to next iteration.
    effort_log.progress_step();
    sample_count = params.sampling;
  }
}


void record_effort(size_t count, double *counter_values) {
  for (size_t i=0; i < count; i++) {
    record_metric(Callpath(), Callpath(), i, counter_values[i]);
  }
}


int MPI_Pcontrol(const int type, ...) {
  // Just in case, avoid interpositioning in MPI implementations that make 
  // MPI_xxx calls.  But if they're calling Pcontrol that's REALLY weird.
  if (effort_lock::in_effort()) return 0;
  effort_lock lock;

  effort_do_stackwalk();
  if (regions == REGIONS_EFFORT || regions == REGIONS_BOTH) {
    record_region(start_callpath, *pnmpi_callpath);
    start_callpath = *pnmpi_callpath;
  }

  cur_effort_type = type;
  if (type == PROGRESS_TYPE) {
    progress_step();
  }

  // don't bother delegating this call down the stack.
  return 0;
}


///
/// Creates directories for effort and exact data.
///
void setup_effort_directories() {
  int rank, size;
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  PMPI_Comm_size(MPI_COMM_WORLD, &size);

  // first make necessary directories.
  ostringstream effort_dir_name;
  effort_dir_name << working_dir << "/effort-" << size;
  effort_dir = effort_dir_name.str();
  mkdir(effort_dir.c_str(), 0750);  // create effort dir if it doesn't exist.

  if (params.verify) {
    ostringstream exact_dir_name;
    exact_dir_name << effort_dir << "/exact";
    exact_dir = exact_dir_name.str();  
    mkdir(exact_dir.c_str(), 0750);
  }
}



#ifndef PMPI_EFFORT
/// This macro checks PnMPI calls and returns error on failure.
#define CHECK_CALL(fn_call) do {\
   if ((err = (fn_call)) != PNMPI_SUCCESS) { \
     cerr << __FILE__ << ":" << __LINE__ << endl; \
     return MPI_ERROR_PNMPI; \
   } \
 } while (0)


/// Contains setup for PnMPI module and registration of services.
int PNMPI_RegistrationPoint() {
  int err = PNMPI_SUCCESS;

  CHECK_CALL(PNMPI_Service_RegisterModule(PNMPI_MODULE_EFFORT));

  PNMPI_modHandle_t my_handle;
  CHECK_CALL(PNMPI_Service_GetModuleByName(PNMPI_MODULE_CALLPATH, &my_handle));

  CHECK_CALL(PNMPI_Service_GetConfiguration(my_handle, config_arguments));

  // using PnMPI stackwalk module.
  PNMPI_modHandle_t sw_handle;
  CHECK_CALL(PNMPI_Service_GetModuleByName(PNMPI_MODULE_CALLPATH, &sw_handle));

  PNMPI_Global_descriptor_t gd;
  CHECK_CALL(PNMPI_Service_GetGlobalByName(sw_handle, PNMPI_GLOBAL_CALLPATH, 'p', &gd));
  pnmpi_callpath = reinterpret_cast<Callpath*>(gd.addr.p);
  
  CHECK_CALL(PNMPI_Service_GetModuleByName(PNMPI_MODULE_CALLPATH, &sw_handle));

  return err;
}
#endif // PMPI_EFFORT


///
/// MPI_Init does non PnMPI-related configuration of the module
/// Does everything we need for a PMPI-only module, too.
///
int MPI_Init(int *argc, char ***argv) {
  effort_lock lock;

#ifdef PMPI_EFFORT
  env_get_configuration(config_arguments);
  pnmpi_callpath = &pmpi_only_callpath;
#endif // PMPI_EFFORT
  runtime.set_chop_libc(params.chop_libc);
  regions = str_to_regions(params.regions);
  sample_count = params.sampling;
  
  working_dir = get_wd();
  init_time = get_time_ns();

  int ret = PMPI_Init(argc, argv);

  int size, rank;
  PMPI_Comm_size(MPI_COMM_WORLD, &size);
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (!isPowerOf2(size) && rank == 0) {
    cerr << "WARNING: Effort module will not currently work with "
         << "non-power-of-2 process count: " << size << endl;
  }

  if (regions == REGIONS_INVALID) {
    regions = REGIONS_EFFORT;
    if (rank == 0) {
      cerr << "WARNING: Invalid value for regions: '" << params.regions << "'. Defaulting to EFFORT."<< endl;
    }
  }

  metric_setup();  
  if (keep_time) start_time = get_time_ns();

  if (rank == 0) {
    cerr << "========================================================" << endl;
    cerr << "   .      Running with Effort Module            " << endl;
    cerr << " _/ \\  _  by Todd Gamblin, tgamblin@cs.unc.edu  " << endl;
    cerr << "     \\/                                         " << endl;
    cerr << endl;
    cerr << params;
    cerr << "========================================================" << endl;
  }
  return ret;
}


/// Prints out all callpaths observed to a file in the effort directory.
void dump_paths(int rank, string name="paths") {
  ostringstream fn;
  fn << effort_dir << "/" << name << "-" << rank;

  ofstream pathfile(fn.str().c_str());
  Callpath::dump(pathfile);
}

/// Prints out all callpaths observed to a file in the effort directory.
void dump_keys(int rank, string name="keys") {
  ostringstream fn;
  fn << effort_dir << "/" << name << "-" << rank;

  ofstream pathfile(fn.str().c_str());
  for (effort_map::iterator i=effort_log.begin(); i != effort_log.end(); i++) {
    pathfile << i->first << endl;
  }
}

/// Prints out module-related timing data to a file in the effort directory.
void dump_timing() {
  // print out all the callpaths for each process
  ostringstream fn;
  fn << effort_dir << "/times";
  ofstream times(fn.str().c_str());
  
  double now = get_time_ns();
  times << "APP:\t"   << (finalize_time - init_time)/1e9 << endl;
  times << "COMP:\t"  << (now - finalize_time)/1e9 << endl;
  times << "TOTAL:\t" << (now - init_time)/1e9 << endl;
}


///
/// MPI_Finalize prints out timings and stackwalk counts, then initiates
/// parallel compression on observed effort data.
///
int MPI_Finalize() {
  effort_lock lock;
  
  int rank, size;
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  PMPI_Comm_size(MPI_COMM_WORLD, &size);

  finalize_time = get_time_ns();

  // this aggregates walks from all nodes
  size_t walks = runtime.numWalks();
  size_t bad_walks = runtime.badWalks();

  size_t total_walks;
  size_t total_bad_walks;
  PMPI_Reduce(&walks, &total_walks, 1, MPI_SIZE_T, MPI_SUM, 0, MPI_COMM_WORLD);
  PMPI_Reduce(&bad_walks, &total_bad_walks, 1, MPI_SIZE_T, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    // Print out percent bad stackwalks here.
    cerr << total_bad_walks << " errors out of " << total_walks << " stackwalks." << endl;
    cerr << (100.0 * total_bad_walks / total_walks) << "% bad walks" << endl;
  }

  // this sets up the directory where we'll do the work.
  setup_effort_directories();

  // distribute and do compression.
  parallel_compressor compressor(params);
  compressor.set_output_dir(effort_dir);
  compressor.set_exact_dir(exact_dir);
  compressor.compress(effort_log, MPI_COMM_WORLD);

  // dump times on rank 0.
  if (rank == 0) {
    dump_timing();
  }

  return PMPI_Finalize();
}
