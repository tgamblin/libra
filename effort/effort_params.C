#include "effort_params.h"
#include <iomanip>
#include <cstring>
using namespace std;

#include "string_utils.h"
using namespace stringutils;

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


  void parse_metrics() {
    if (strlen(metrics) && !metric_names.size()) {
      
    }
    

    // Split out what the user asked for into separate strings.
    split(metrics, " ,", metric_names);

    // move time first if it's there 
    vector<string>::iterator todel = remove_if(metric_names.begin(), metric_names.end(), 
                                               bind2nd(equal_to<string>(), METRIC_TIME));
    keep_time = todel != metric_names.end()
    metric_names.erase(todel, metric_names.end());
  }

    size_t num_metrics();

  const string& metric_name(int id) {


    if (id < 0) {
      return METRIC_TIME;
    } else if (id < (int)metric_names.size()) {
      return metric_names[id];
    } else {
      ostringstream idstr;
      idstr << id;
      return idstr.str();
    }

  }

} //namespace
