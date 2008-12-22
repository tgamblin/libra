#include "effort_key.h"
#include "Callpath.h"
#include "mpi_utils.h"
using namespace std;

namespace effort {

  effort_key::effort_key(int met, int t, Callpath start, Callpath end) 
    : metric(met), type(t), start_path(start), end_path(end) 
  { }


  bool effort_key::isComm() const {
    return (start_path == Callpath::null()) && (end_path == Callpath::null());
  }


  effort_key::effort_key(const effort_key& other) 
    : metric(other.metric), 
      type(other.type), 
      start_path(other.start_path), 
      end_path(other.end_path) { }


  effort_key& effort_key::operator=(const effort_key& other) {
    metric = other.metric;
    type = other.type;
    start_path = other.start_path;
    end_path = other.end_path;
    return *this;
  }
  

  bool operator==(const effort_key& lhs, const effort_key& rhs) {
    return lhs.metric   == rhs.metric 
      && lhs.type       == rhs.type 
      && lhs.start_path == rhs.start_path
      && lhs.end_path   == rhs.end_path;
  }

  bool operator<(const effort_key& lhs, const effort_key& rhs) {
    if (lhs.metric != rhs.metric) {
      return lhs.metric < rhs.metric;

    } else if (lhs.type != rhs.type) {
      return lhs.type < rhs.type;

    } else if (lhs.start_path != rhs.start_path) {
      return lhs.start_path < rhs.start_path;

    } else {
      return lhs.end_path < rhs.end_path;
    }
  }

  std::ostream& operator<<(std::ostream& out, const effort_key& key) {
    out << "[" 
        << key.metric << " "
        << key.type << " " 
        << key.start_path << " => " << key.end_path 
        << "]";

    return out;
  }

  int effort_key::packed_size(MPI_Comm comm) const {
    int size = 0;
    size += mpi_packed_size(1, MPI_INT, comm);               // metric 
    size += mpi_packed_size(1, MPI_INT, comm);  // type
    size += start_path.packed_size(comm);       // start signature
    size += end_path.packed_size(comm);         // end signature 
    return size;
  }


  void effort_key::pack(void *buf, int bufsize, int *position, MPI_Comm comm) const {
    PMPI_Pack(const_cast<int*>(&metric), 1, MPI_INT, buf, bufsize, position, comm);
    PMPI_Pack(const_cast<int*>(&type), 1, MPI_INT, buf, bufsize, position, comm);
    start_path.pack(buf, bufsize, position, comm);
    end_path.pack(buf, bufsize, position, comm);
  }


  effort_key effort_key::unpack(module_map& modules, void *buf, int bufsize, int *position, MPI_Comm comm) {
    effort_key key;
    PMPI_Unpack(buf, bufsize, position, &key.metric, 1, MPI_INT,  comm);
    PMPI_Unpack(buf, bufsize, position, &key.type, 1, MPI_INT,  comm);
    key.start_path = Callpath::unpack(modules, buf, bufsize, position, comm);
    key.end_path = Callpath::unpack(modules, buf, bufsize, position, comm);
    return key;
  }


  bool effort_key_full_lt::operator()(const effort_key& lhs, const effort_key& rhs) {
    if (lhs.metric != rhs.metric) {
      return lhs.metric < rhs.metric;
      
    } else if (lhs.type != rhs.type) {
      return lhs.type < rhs.type;
      
    } else if (lt(lhs.start_path, rhs.start_path)) {
      return true;
      
    } else if (lt(rhs.start_path, lhs.start_path)) {
      return false;
      
    } else if (lt(lhs.end_path, rhs.end_path)) {
      return true;
      
    } else if (lt(rhs.end_path, lhs.end_path)) {
      return false;
      
    } else {
      return true;
    }
  }

  
} // namespace
