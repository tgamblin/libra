#ifndef EFFORT_SIGNATURE_H
#define EFFORT_SIGNATURE_H

#include <boost/shared_array.hpp>
#include "libra-config.h"

namespace effort {
  
  /// Reference-counted array of effort signature data.
  class effort_signature {
    /// Transformed Effort data for comparison with
    boost::shared_array<double> signature;
    
    /// Construct a new effort signature by transforming the data down to the
    /// specified level and using its low-frequency information.  By default the 
    /// data is transformed to the lowest level possible.
    effort_signature(const double *data, size_t len, int level=-1);

    effort_signature(const effort_signature& other);

    ~effort_signature();
    
    effort_signature& operator=(const effort_signature& other);    

#ifdef LIBRA_HAVE_MPI
    int packed_size(MPI_Comm comm) const;
    void pack(void *buf, int bufsize, int *position, MPI_Comm comm) const;
    static T unpack(void *buf, int bufsize, int *position, MPI_Comm comm);
#endif // LIBRA_HAVE_MPI
  }; // effort_signature

} // namespace effort

#endif // EFFORT_SIGNATURE_H
