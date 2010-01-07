#include "effort_signature.h"

#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef HAVE_MPI
#include <mpi.h>
#include "mpi_utils.h"
#endif // HAVE_MPI

#include "wt_lift.h"
#include "io_utils.h"


using namespace wavelet;
using namespace std;

namespace effort {

  void effort_signature::init(const double *data, size_t len, int level) {
    vector<double> tmp(len);
    copy(data, data+len, tmp.begin());

    int max_level = (int)log2pow2(len);
    if (level < 0) {
      level = max_level - 4;
    }

    // transform data
    wt_lift wt;
    wt.fwt_1d(&tmp[0], tmp.size(), level);

    // copy lowest band of transformed coefficients into signature array.
    sig_size = len >> level;
    double *sig = new double[sig_size];
    copy(tmp.begin(), tmp.begin() + sig_size, sig);

    // finally give the shared array ownership.
    signature.reset(sig);
  }


  effort_signature::effort_signature() : sig_size(0) { }

  effort_signature::effort_signature(const effort_signature& other) 
    : signature(other.signature), sig_size(other.sig_size) { }

  effort_signature::~effort_signature() { }

  effort_signature& effort_signature::operator=(const effort_signature& other) {
    signature = other.signature;
    sig_size = other.sig_size;
    return *this;
  }


#ifdef HAVE_MPI
  int effort_signature::packed_size(MPI_Comm comm) const {
    int size = 0;
    size += mpi_packed_size(1, MPI_SIZE_T, comm);
    size += mpi_packed_size(sig_size, MPI_DOUBLE, comm);
    return size;
  }

  void effort_signature::pack(void *buf, int bufsize, int *position, MPI_Comm comm) const {
    PMPI_Pack(const_cast<size_t*>(&sig_size), 1, MPI_SIZE_T, buf, bufsize, position, comm);
    PMPI_Pack(const_cast<double*>(signature.get()), sig_size, MPI_DOUBLE, buf, bufsize, position, comm);
  }

  effort_signature effort_signature::unpack(void *buf, int bufsize, int *position, MPI_Comm comm) {
    effort_signature result;
    PMPI_Unpack(buf, bufsize, position, &result.sig_size, 1, MPI_SIZE_T, comm);

    double *sig = new double[result.sig_size];
    PMPI_Unpack(buf, bufsize, position, sig, result.sig_size, MPI_DOUBLE, comm);
    result.signature.reset(sig);

    return result;
  }
#endif // HAVE_MPI

} // namespace effort
