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



} //namespace
