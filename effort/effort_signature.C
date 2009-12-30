#include "effort_signature.h"

#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "wt_lift.h"
using namespace std;

namespace effort {

  effort_signature::effort_signature(const double *data, size_t len, int level=-1) {
    vector<double> tmp(data, data+len);    // make temporary copy of inputs

    wt_lift wt;
    wt.fwt_1d();
  }

  effort_signature::effort_signature(const effort_signature& other) 
    : signature(other.signature) { }

  effort_signature::~effort_signature() { }
    
  effort_signature& effort_signature::operator=(const effort_signature& other) {
    signature = other.signature;
  }


#ifdef HAVE_MPI
  int effort_signature::packed_size(MPI_Comm comm) const {

  }

  void effort_signature::pack(void *buf, int bufsize, int *position, MPI_Comm comm) const {
    
  }

  static effort_signature effort_signature::unpack(void *buf, int bufsize, int *position, MPI_Comm comm) {
    
  }
#endif // HAVE_MPI


  
} // namespace effort
