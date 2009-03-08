#include "Callpath.h"

#include <signal.h>
#include <execinfo.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <map>
using namespace std;

#ifdef LIBRA_HAVE_MPI
#include "mpi_utils.h"
#endif //LIBRA_HAVE_MPI

#include "io_utils.h"
using namespace wavelet;


/// This is the set of all unique callpaths seen so far.  Used to unique
/// callpaths on creation, so that instances can be compared by pointer.
typedef set<const pathvector*, pathvector_lt<less<FrameId> > > callpath_set;

static callpath_set& paths() {
  static callpath_set pathset;
  return pathset;
}

Callpath::Callpath(const pathvector *p) : path(p) { }


Callpath::Callpath(const Callpath& other) : path(other.path) { }


Callpath Callpath::create(const pathvector& path) {
  callpath_set::iterator u = paths().find(&path);
  if (u == paths().end()) {
    // if the vector isn't in there already then create a copy to add
    pathvector *temp = new pathvector(path);

    // unique all the module pointers in the new vector.
    for (pathvector::iterator i=temp->begin(); i != temp->end(); i++) {
      i->module = FrameId::module_for(*i->module);
    }
    u = paths().insert(temp).first;
  }
  return Callpath(*u);
}


Callpath& Callpath::operator=(const Callpath& other) {
  path = other.path;
  return *this;
}


#ifdef LIBRA_HAVE_MPI

size_t Callpath::packed_size_modules(MPI_Comm comm) {
  size_t pack_size = 0;
  pack_size += mpi_packed_size(1, MPI_INT, comm);  // number of modules
  for (module_set::iterator i=FrameId::modules().begin(); i != FrameId::modules().end(); i++) {
    pack_size += mpi_packed_size(1, MPI_UINTPTR_T, comm);         // local addr of module string
    pack_size += mpi_packed_size(1, MPI_INT, comm);              // length
    pack_size += mpi_packed_size((*i)->size(), MPI_CHAR, comm);  // characters in modulename.
  }
  return pack_size;
}


void Callpath::pack_modules(void *buf, int bufsize, int *position, MPI_Comm comm) {
  int len = FrameId::modules().size();
  PMPI_Pack(&len, 1, MPI_INT, buf, bufsize, position, comm);
  for (module_set::iterator i=FrameId::modules().begin(); i != FrameId::modules().end(); i++) {
    // local addr of module string
    PMPI_Pack(const_cast<string**>(&(*i)), 1, MPI_UINTPTR_T, buf, bufsize, position, comm);

    len = (*i)->size();
    PMPI_Pack(&len, 1, MPI_INT, buf, bufsize, position, comm);           // length of string
    if (len) {
      PMPI_Pack(const_cast<char*>(&(**i)[0]), len, MPI_CHAR, buf, bufsize, position, comm); // characters in string
    }
  }
}


void Callpath::unpack_modules(void *buf, int bufsize, int *position, module_map& dest, MPI_Comm comm) {
  int len;
  PMPI_Unpack(buf, bufsize, position, &len, 1, MPI_INT, comm);
  for (int i=0; i < len; i++) {
    uintptr_t remote_addr;     // addr of string on remote machine
    PMPI_Unpack(buf, bufsize, position, &remote_addr, 1, MPI_UINTPTR_T, comm); 
    
    int modlen;
    PMPI_Unpack(buf, bufsize, position, &modlen, 1, MPI_INT, comm);           // length of module

    char modname[modlen + 1];
    if (modlen) {
      PMPI_Unpack(buf, bufsize, position, modname, modlen, MPI_CHAR, comm);   // actual characters
    }
    modname[modlen] = '\0';

    // map local address of module name string to remote address
    dest[remote_addr] = FrameId::module_for(string(modname)); 
  }
}


size_t Callpath::packed_size(MPI_Comm comm) const {
  size_t pack_size = 0;
  pack_size += mpi_packed_size(1, MPI_INT, comm);  // for length
  for (size_t i=0; i < size(); i++) {      // for each elt
    pack_size += (*path)[i].packed_size(comm);
  }
  return pack_size;
}

  
void Callpath::pack(void *buf, int bufsize, int *position, MPI_Comm comm) const {
  int len = size();
  PMPI_Pack(&len, 1, MPI_INT, buf, bufsize, position, comm);

  for (int i=0; i < len; i++) {
    (*path)[i].pack(buf, bufsize, position, comm);
  }
}


Callpath Callpath::unpack(module_map& modules, void *buf, int bufsize, int *position, MPI_Comm comm) {
  int len;
  PMPI_Unpack(buf, bufsize, position, &len, 1, MPI_INT, comm); // number of elements
  
  if (!len) {
    return null();
  }
   
  vector<FrameId> path;
  
  for (int i=0; i < len; i++) {
    // unpack and translate module address, then push onto the vector.
    FrameId raw_frame = FrameId::unpack(buf, bufsize, position, comm);
    raw_frame.module = modules[reinterpret_cast<uintptr_t>(raw_frame.module)];
    path.push_back(raw_frame);
  }
  
  return create(path);
}
#endif // LIBRA_HAVE_MPI

std::ostream& operator<<(std::ostream& out, const Callpath& cp) {
  if (!cp.path) {
    out << "null_callpath";

  } else {
    pathvector::const_reverse_iterator i=cp.path->rbegin();
    if (i != cp.path->rend()) {
      out << (*i->module) << "(" << hex << i->offset << ")";
      i++;
    }
    while (i != cp.path->rend()) {
      out << " : "
          << (*i->module) << "(" << hex << i->offset << ")";
      i++;
    }
  }
  out << dec; // revert to decimal.
  return out;
}


size_t Callpath::size() const {
  return path ? path->size() : 0;
}


void Callpath::write_out(ostream& out) {
  // build set of modules referenced in this particular callpath
  module_set my_modules;
  for (size_t i=0; i < size(); i++) {
    my_modules.insert((*path)[i].module);
  }

  // write out names/string addrs of all module strings used here
  size_t num_modules = my_modules.size();
  vl_write(out, num_modules);
  for (module_set::iterator i=my_modules.begin(); i != my_modules.end(); i++) {
    const string& modname = **i;
    vl_write(out, reinterpret_cast<uintptr_t>(&modname));   // address
    vl_write(out, modname.size());                         // strlen
    if (modname.size()) {                                  // actual string
      out.write(&modname[0], modname.size());
    }
  }

  // write out each frame id
  vl_write(out, size());
  for (size_t i=0; i < size(); i++) {
    (*path)[i].write_out(out);
  }
}


Callpath Callpath::read_in(istream& in) {
  size_t num_modules = vl_read(in);

  // read in strings and make entries to translate addresses
  module_map translation;
  for (size_t i=0; i < num_modules; i++) {
    uintptr_t remote_addr = vl_read(in);
    size_t modlen = vl_read(in);

    char modbuf[modlen+1];
    in.read(modbuf, modlen);
    modbuf[modlen] = '\0';
    translation[remote_addr] = FrameId::module_for(string(modbuf));
  }

  size_t len = vl_read(in);
  if (!len) {
    return Callpath(NULL);

  } else {
    vector<FrameId> temp;
    
    for (size_t i=0; i < len; i++) {
      FrameId fid = FrameId::read_in(in);
      fid.module = translation[reinterpret_cast<uintptr_t>(fid.module)];
      temp.push_back(fid);
    }
    return create(temp);
  }
}

void Callpath::dump(ostream& out) {
  out << paths().size() << " total paths" << endl;

  for (callpath_set::iterator i=paths().begin(); i != paths().end(); i++) {
    out << Callpath(*i) << endl;
  }
}


const FrameId& Callpath::get(size_t i) const {
  if (i > size()) {
    cerr << "Index out of bounds: " << i << endl;
    exit(1);
  }
  return (*path)[i]; 
}
