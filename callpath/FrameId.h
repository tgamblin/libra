#ifndef FRAME_ID_H
#define FRAME_ID_H

#include "libra-config.h"
#ifdef LIBRA_HAVE_MPI
#include <mpi.h>
#endif // LIBRA_HAVE_MPI

#include <stdint.h>
#include <string>
#include <set>
#include <iostream>

/// Comparator for string pointers
struct fid_dereference_lt {
  template <typename Tp>
  bool operator()(Tp lhs, Tp rhs) const {
    return *lhs < *rhs;
  }
};

/// Set of unique string pointers for module ids.
typedef std::set<const std::string*, fid_dereference_lt> module_set;

///
/// Element type for callpaths.  Uniquely identifies an address within 
/// a loadable module.
///
class FrameId {
public:
  const std::string *module;
  uintptr_t offset;

  ~FrameId() { }

  FrameId(const FrameId& other);
  FrameId& operator=(const FrameId& other);

  /// writes out raw values for this FrameId
  void write_out(std::ostream& out) const;

  /// reads a frame's raw values in from a file.  module will require
  /// translation before it is usable.
  static FrameId read_in(std::istream& in);

  /// Creates a FrameId with a unique module pointer.
  static FrameId create(const std::string& name, uintptr_t offset);

  /// Get full set of modules seen in frames so far.
  static module_set& modules();

#ifdef LIBRA_HAVE_MPI
  /// Gets size of a packed frame id for sending via MPI.
  size_t packed_size(MPI_Comm comm) const;

  /// Packs a frame onto an MPI buffer.  Note that this does NOT pack up
  /// the modules string, just the pointer.  This is inteded to be
  /// used from within Callpath to send frames along with a module translation
  /// table.  It is not a general-purpose send for frameids.
  void pack(void *buf, int bufsize, int *position, MPI_Comm comm) const;

  /// Unpacks a raw FrameId as packed by pack().  Expects a raw remote
  /// pointer, not a module string.  module will require translation
  /// before it is usable.
  static FrameId unpack(void *buf, int bufsize, int *position, MPI_Comm comm);
#endif // LIBRA_HAVE_MPI  
  
private:
  friend class Callpath;
  friend class CallpathRuntime;

  static const std::string *module_for(const std::string& name);

  FrameId() : module(NULL), offset(0) { }  
  FrameId(const std::string *m, uintptr_t o) : module(m), offset(o) { }
  
  friend bool operator==(const FrameId& lhs, const FrameId& rhs);
  friend bool operator<(const FrameId& lhs, const FrameId& rhs);
  friend bool operator>(const FrameId& lhs, const FrameId& rhs);
};

inline bool operator==(const FrameId& lhs, const FrameId& rhs) {
  return (lhs.module == rhs.module) && (lhs.offset == rhs.offset);
}

inline bool operator<(const FrameId& lhs, const FrameId& rhs) {
  return (lhs.module == rhs.module) 
    ? lhs.offset < rhs.offset 
    : lhs.module < rhs.module;
}

inline bool operator>(const FrameId& lhs, const FrameId& rhs) {
  return (lhs.module == rhs.module) 
    ? lhs.offset > rhs.offset 
    : lhs.module > rhs.module;
}

std::ostream& operator<<(std::ostream& out, const FrameId& fid);


/// Comparator for FrameIds that uses actual module name + offset
struct frameid_string_lt {
  bool operator()(const FrameId& lhs, const FrameId& rhs) {
    if ((*lhs.module) == (*rhs.module)) {
      return lhs.offset < rhs.offset;
    } else {
      return (*lhs.module) < (*rhs.module);
    }
  }
};


#endif //FRAME_ID_H
