#ifndef STL_UTILS
#define STL_UTILS
#include <utility>

///
/// This file contains some utility functions for dealing with the STL.
///


/// Functor to get the first element of a pair.  Use with STL functions like transform().
struct get_first {
  template <typename P>
  typename P::first_type operator()(const P& pair) {
    return pair.first;
  }
};

/// Functor to get the second element of a pair.  Use with STL functions like transform().
struct get_second {
  template <typename P>
  typename P::second_type operator()(const P& pair) {
    return pair.second;
  }
};

#endif // STL_UTILS
