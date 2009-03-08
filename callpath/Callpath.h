#ifndef CALLPATH_H
#define CALLPATH_H

#include "libra-config.h"
#ifdef LIBRA_HAVE_MPI
#include <mpi.h>
#endif // LIBRA_HAVE_MPI

#include <stdint.h>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include "FrameId.h"
#include "Module.h"

/// This class represents a unique callpath, along with some extra info 
/// for easy callpath lookup.
class Callpath {
public:

  Callpath() : path(NULL) { } /// Construct a null callpath.

  Callpath(const Callpath& other);  /// Copy constructor

  ~Callpath() { }
   
  /// Returns the null callpath.
  static inline Callpath null() {
    return Callpath(NULL);
  }

  /// Assignment
  Callpath& operator=(const Callpath& other);


  /// Dumps all known paths to a file
  static void dump(std::ostream& out);
    

  static Callpath create(const std::vector<FrameId>& path);


  /// Gets the ith element in the callpath.
  const FrameId& operator[](size_t i) const {
    return (*path)[i]; 
  }
  
  /// Synonym for operator[], but with bounds checking.
  const FrameId& get(size_t i) const;

  /// Number of elements in the callpath.
  size_t size() const;


  /// Writes this callpath out to a stream.
  void write_out(std::ostream& out);

  /// Reads a callpath in from a stream.
  static Callpath read_in(std::istream& in);

#ifdef LIBRA_HAVE_MPI
  /// Gets upper bound on size of this callpath, if it were packed into an MPI buffer.
  size_t packed_size(MPI_Comm comm) const;

  /// Packs this callpath into an MPI transport buffer.
  void pack(void *buf, int bufsize, int *position, MPI_Comm comm) const;

  /// Unpacks a callpath packed by pack().  Ensures path pointer uniqueness 
  /// within processes, but not across nodes.  Requires module translation via
  /// a map from send_modules().
  static Callpath unpack(const Module::id_map& modules, void *buf, int bufsize, int *position, MPI_Comm comm);
  
#endif // LIBRA_HAVE_MPI
  
private:
  /// Unique, null-terminated array of symbol_ids for this callpath
  const std::vector<FrameId> *path;

  /// Private value constructor: used only by this class.
  Callpath(const std::vector<FrameId> *path);

  // Declare operators as friends so they can get at the internals.
  friend std::ostream& operator<<(std::ostream& out, const Callpath& path);
  friend bool operator==(const Callpath& lhs, const Callpath& rhs);
  friend bool operator<(const Callpath& lhs, const Callpath& rhs);
  friend bool operator>(const Callpath& lhs, const Callpath& rhs);
  friend struct callpath_path_lt;
}; // Callpath


/// Uses fast pointer comparison b/c path is guaranteed unique per callpath.
inline bool operator==(const Callpath& lhs, const Callpath& rhs) { 
  return lhs.path == rhs.path;
} 
  
/// Uses fast pointer comparison b/c path is guaranteed unique per callpath.
inline bool operator>(const Callpath& lhs, const Callpath& rhs) {
  return lhs.path > rhs.path;
}

/// Uses fast pointer comparison b/c path is guaranteed unique per callpath.
inline bool operator<(const Callpath& lhs, const Callpath& rhs) {
  return lhs.path < rhs.path;
}

/// Uses fast pointer comparison b/c path is guaranteed unique per callpath.
inline bool operator!=(const Callpath& lhs, const Callpath& rhs) { 
  return !(lhs == rhs);
} 

/// Outputs a simple string representation of this callpath.
std::ostream& operator<<(std::ostream& out, const Callpath& path);

/// Less-than comparator for arrays of frame ids.
template <class LessThan>
struct pathvector_lt {
  LessThan lt;
  bool operator()(const std::vector<FrameId> *lhs, const std::vector<FrameId> *rhs) {
    if (lhs == rhs)  return false;
    if (lhs == NULL) return true;
    if (rhs == NULL) return false;
    
    for (size_t i=0; i < lhs->size() && i < rhs->size(); i++) {
      if (lt((*lhs)[i], (*rhs)[i])) {
        return true;
      } else if (lt((*rhs)[i], (*lhs)[i])) {
        return false;
      }
    }
    return lhs->size() < rhs->size();
  }
};

/// Functor for path_lt.
struct callpath_path_lt {
  pathvector_lt<frameid_string_lt> lt;
  bool operator()(const Callpath& lhs, const Callpath& rhs) {
    return lt(lhs.path, rhs.path);
  }
};

#endif //CALLPATH_H
