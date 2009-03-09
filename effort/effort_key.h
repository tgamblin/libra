#ifndef EFFORT_KEY_H
#define EFFORT_KEY_H

#include "Callpath.h"
#include "Metric.h"
#include "ModuleId.h"

#include "libra-config.h"
#ifdef LIBRA_HAVE_MPI
#include <mpi.h>
#endif // LIBRA_HAVE_MPI

/// integer type for progress to pass to MPI_Pcontrol()
#define PROGRESS_TYPE 0

namespace effort {
  
  /// An effort key describes an effort region in the code.  Its start_signature
  /// and end_signature are dynamic callpaths indicating what events started and
  /// ended the effort region.  Type is a user-defined value for phase id, which
  /// can be used to further differentiate effort.
  struct effort_key {
    Metric metric;
    int type;
    Callpath start_path;
    Callpath end_path;

    /// Constructs an effort key with two null callpaths.
    effort_key(Metric m = Metric::time(), int t = PROGRESS_TYPE) : metric(m), type(t) { }
    
    /// Value constructor.
    effort_key(Metric m, int t, Callpath start, Callpath end);

    /// Copy constructor.
    effort_key(const effort_key& other);

    // Assignment.
    effort_key& operator=(const effort_key& other);

    /// True if the effort key represents communciation.
    bool isComm() const;

    /// Writes out this effort id to a stream
    void write_out(std::ostream& out);
    
    /// Reads an effort id out of a stream.
    static void read_in(std::istream& in, effort_key& md);

#ifdef LIBRA_HAVE_MPI
    // For MPI packing -- you'll need Callpath::pack_modules() to get a 
    // module_map and unpack properly.
    int packed_size(MPI_Comm comm) const;
    void pack(void *buf, int bufsize, int *position, MPI_Comm comm) const;
    static effort_key unpack(const ModuleId::id_map& modules, void *buf, int bufsize, int *position, MPI_Comm comm);
#endif // LIBRA_HAVE_MPI
  };

  /// compares type and signatures to make sure they're the same.
  bool operator==(const effort_key& lhs, const effort_key& rhs);

  /// Less than comparator for effort keys.  Used in effort_map below.
  bool operator<(const effort_key& lhs, const effort_key& rhs);

  // other comparators derived from above for convenience
  inline bool operator!=(const effort_key& lhs, const effort_key& rhs) { 
    return !(lhs == rhs); 
  }
  
  inline bool operator>(const effort_key& lhs, const effort_key& rhs) { 
    return !(lhs == rhs) && !(lhs < rhs); 
  }

  /// Outputs effort key as as tring
  std::ostream& operator<<(std::ostream& out, const effort_key& key);
  
  /// Full (string, frame by frame) compare of effort keys.  Use this to make
  /// Order consistent across nodes.
  struct effort_key_full_lt {
    callpath_path_lt lt;
    bool operator()(const effort_key& lhs, const effort_key& rhs);
  };
  

  /// Utility function to identify valid filenames output by the tool
  /// TODO : provide something to make a filename here too.
  bool parse_filename(const std::string& filename, std::string *metric = NULL, int *type = NULL, int *number = NULL);

} // namespace

#endif // EFFORT_KEY_H
