#include "effort_params.h"
#include <iomanip>
using namespace std;

namespace effort {

  ostream& operator<<(ostream& out, const effort_params& params) {
    out << "   metrics            = " << params.metrics           << endl;
    out << "   pass_limit         = " << params.pass_limit        << endl;
    out << "   scale              = " << params.scale             << endl;
    out << "   rows_per_process   = " << params.rows_per_process  << endl;
    out << "   encoding           = " << params.encoding          << endl;
    out << "   verify             = " << params.verify            << endl;
    out << "   sequential         = " << params.sequential        << endl;
    out << "   chop_libc          = " << params.chop_libc         << endl;
    out << "   regions            = " << params.regions           << endl;
    out << "   sampling           = " << params.sampling << endl;
    return out;
  }

  /// Arguments for effort module configuration.
  config_desc *effort_params::get_config_arguments() {
    static config_desc args[] = {
      config_desc("rows_per_process",  &this->rows_per_process),
      config_desc("verify",            &this->verify),
      config_desc("pass_limit",        &this->pass_limit),
      config_desc("scale",             &this->scale),
      config_desc("sequential",        &this->sequential),
      config_desc("encoding",          &this->encoding),
      config_desc("metrics",           &this->metrics),
      config_desc("chop_libc",         &this->chop_libc),
      config_desc("regions",           &this->regions),
      config_desc("sampling",          &this->sampling),
      config_desc()
    };
    return args;
  }

} //namespace
