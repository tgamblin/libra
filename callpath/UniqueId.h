#ifndef UNIQUE_ID_H
#define UNIQUE_ID_H

#include <string>
#include <set>
#include <ostream>

#include "io_utils.h"

#include "libra-config.h"
#ifdef LIBRA_HAVE_MPI
#include <mpi.h>
#include "mpi_utils.h"
#endif // LIBRA_HAVE_MPI

/// Compares by targets of pointer types.
struct dereference_lt {
  template <typename Tp>
  bool operator()(Tp lhs, Tp rhs) const {
    return *lhs < *rhs;
  }
};

/// Class to represent internally-uniqued strings.  Much like symbols in ruby or lisp,
/// this keeps an internal set of pointers to unique std::strings.
/// Instances of the class contain unique pointers into this set, so they can be
/// compared fast for equality, less than, etc.  Lookups into the map are done
/// at construction.  Methods for serialization, etc are also provided here.
///
/// Note: std::string class must be copyable (support copy constructor) and less-than comparable.
///
/// To make your own unique id class out of this:
/// 
///    class MyUniqueIdClass : public UniqueId<MyUniqueIdClass> {
///        MyUniqueIdClass(const std::string& id) : UniqueId<MyUniqueIdClass>(id) { }
///    };
///
/// That's all!
///
template <class Derived>
class UniqueId {
protected:
  /// Type for set of unique uid values.
  typedef std::set<const std::string*, dereference_lt> id_set;
  typedef typename id_set::iterator id_set_iterator;

  /// Unique identifier for this instance of the UniqueId
  const std::string *identifier;
  
  static id_set& get_identifiers() {
    static id_set ids;
    return ids;
  }
  
  /// Constructor takes a const std::string reference, gets a unique pointer to its value,
  /// and inits this uid with the pointer.
  UniqueId(const std::string& id) { 
    id_set& ids = get_identifiers();
    id_set_iterator i = ids.find(&id);
    if (i == ids.end()) {
      i = ids.insert(new std::string(id)).first;
    }
    this->identifier = *i;
  }
  
public:
  Derived& operator=(const Derived& other) {
    identifier = other->identifier;
  }
  
  bool operator<(const Derived& other) const {
    return identifier < other.identifier;
  }

  bool operator==(const Derived& other) const {
    return identifier == other.identifier;
  }
  
  bool operator>(const Derived& other) const {
    return identifier > other.identifier;
  }
  
  const char *c_str() const {
    return identifier->c_str();
  }

  const std::string& str() const {
    return *identifier;
  }

  void write_out(std::ostream& out) const {
    wavelet::vl_write(out, identifier->size());
    out.write(identifier->c_str(), identifier->size());
  }

  static Derived read_in(std::istream& in) {
    size_t id_size = vl_read(in);
    char buf[id_size+1];
    in.read(buf, id_size);
    buf[id_size] = '\0';
    return Derived(buf);
  }

  template<class Derived>
  friend std::ostream& operator<<(std::ostream& out, UniqueId<Derived> uid);

#ifdef LIBRA_HAVE_MPI
  int packed_size(MPI_Comm comm) const {
    int size = 0;
    size += mpi_packed_size(1, MPI_INT, comm);  // identifier size
    size += mpi_packed_size(identifier->size(), MPI_CHAR, comm);
    return size;
  }


  void pack(void *buf, int bufsize, int *position, MPI_Comm comm) const {
    int size = identifier->size();
    PMPI_Pack(&size, 1, MPI_INT, buf, bufsize, position, comm);
    PMPI_Pack(const_cast<char*>(identifier->c_str()), identifier->size(), MPI_CHAR, buf, bufsize, position, comm);
  }


  static Derived unpack(void *buf, int bufsize, int *position, MPI_Comm comm) {
    int size;
    PMPI_Unpack(buf, bufsize, position, &size, 1, MPI_INT,  comm);
    
    char idstring[size+1];
    PMPI_Unpack(buf, bufsize, position, idstring, size, MPI_CHAR,  comm);
    idstring[size] = '\0';

    return Derived(idstring);
  }
#endif // LIBRA_HAVE_MPI
};


template<class Derived>
std::ostream& operator<<(std::ostream& out, UniqueId<Derived> uid) {
  out << uid.identifier;
  return out;
}



#endif //UNIQUE_ID_H
