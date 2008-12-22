#include "FrameId.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef LIBRA_HAVE_MPI
#include "mpi_utils.h"
#endif // LIBRA_HAVE_MPI

#include "io_utils.h"
#include <iomanip>
using namespace wavelet;
using namespace std;

//----------------------------------------------------//
// Set of unique module names (string pointers)
//----------------------------------------------------//
module_set FrameId::modules;

FrameId::FrameId(const FrameId& other) 
  : module(other.module), offset(other.offset) { }


FrameId& FrameId::operator=(const FrameId& other) {
  module = other.module;
  offset = other.offset;
  return *this;
}

void FrameId::write_out(ostream& out) const {
  vl_write(out, reinterpret_cast<uintptr_t>(module));
  vl_write(out, offset);
}

FrameId FrameId::read_in(istream& in) {
  FrameId fid;
  fid.module = reinterpret_cast<const string*>(vl_read(in));
  fid.offset = vl_read(in);
  return fid;
}

ostream& operator<<(ostream& out, const FrameId& fid) {
  out << *fid.module << "(0x" << hex << fid.offset << ")" << dec;
  return out;
}

const string *FrameId::module_for(const string& name) {
  module_set::iterator m = modules.find(&name);
  if (m == modules.end()) {
    m = modules.insert(new string(name)).first;
  }
  return *m;
}

FrameId FrameId::create(const string& name, uintptr_t offset) {
  return FrameId(module_for(name), offset);
}

#ifdef LIBRA_HAVE_MPI

size_t FrameId::packed_size(MPI_Comm comm) const {
  size_t pack_size = 0;
  pack_size += mpi_packed_size(1, MPI_UINTPTR_T, comm);  // module pointer
  pack_size += mpi_packed_size(1, MPI_UINTPTR_T, comm);  // offset
  return pack_size;
}

void FrameId::pack(void *buf, int bufsize, int *position, MPI_Comm comm) const {
  PMPI_Pack(const_cast<const string**>(&module), 1, MPI_UINTPTR_T, buf, bufsize, position, comm);
  PMPI_Pack(const_cast<uintptr_t*>(&offset), 1, MPI_UINTPTR_T, buf, bufsize, position, comm);
}

FrameId FrameId::unpack(void *buf, int bufsize, int *position, MPI_Comm comm) {
  FrameId result;
  PMPI_Unpack(buf, bufsize, position, &result.module, 1, MPI_UINTPTR_T, comm);
  PMPI_Unpack(buf, bufsize, position, &result.offset, 1, MPI_UINTPTR_T, comm);
  return result;
}

#endif // LIBRA_HAVE_MPI

