#ifndef BYTE_BUDGET_EXCEPTION
#define BYTE_BUDGET_EXCEPTION

#include <stdexcept>
#include <string>

namespace wavelet {
  
  class byte_budget_exception : public std::exception {
  public:
    byte_budget_exception();
  };
  
}

#endif // BYTE_BUDGET_EXCEPTION
