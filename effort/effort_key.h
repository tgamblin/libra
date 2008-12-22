#ifndef EFFORT_KEY_H
#define EFFORT_KEY_H

#include "Callpath.h"

/// integer type for progress to pass to MPI_Pcontrol()
#define PROGRESS_TYPE 0

#define METRIC_TIME "time"
#define METRIC_TIME_ID -1

namespace effort {
  
  /// An effort key describes an effort region in the code.  Its start_signature
  /// and end_signature are dynamic callpaths indicating what events started and
  /// ended the effort region.  Type is a user-defined value for phase id, which
  /// can be used to further differentiate effort.
  struct effort_key {
    int metric;
    int type;
    Callpath start_path;
    Callpath end_path;

    /// Constructs an effort key with two null callpaths.
    effort_key(int m = METRIC_TIME_ID, int t = PROGRESS_TYPE) : metric(m), type(t) { }
    
    /// Value constructor.
    effort_key(int m, int t, Callpath start, Callpath end);

    /// Copy constructor.
    effort_key(const effort_key& other);

    // Assignment.
    effort_key& operator=(const effort_key& other);

    /// True if the effort key represents communciation.
    bool isComm() const;

    // For MPI packing -- you'll need Callpath::pack_modules() to get a 
    // module_map and unpack properly.
    int packed_size(MPI_Comm comm) const;
    void pack(void *buf, int bufsize, int *position, MPI_Comm comm) const;
    static effort_key unpack(module_map& modules, void *buf, int bufsize, int *position, MPI_Comm comm);
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
  
} // namespace

#endif // EFFORT_KEY_H
