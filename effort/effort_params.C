#include "effort_params.h"
#include <iomanip>
#include <cstring>
using namespace std;

#include "sampling.h"
#include "string_utils.h"
using namespace stringutils;

namespace effort {

  ostream& operator<<(ostream& out, effort_params& params) {
    out << "   metrics              = " << params.metrics            << endl;
    out << "   pass_limit           = " << params.pass_limit         << endl;
    out << "   scale                = " << params.scale              << endl;
    out << "   rows_per_process     = " << params.rows_per_process   << endl;
    out << "   encoding             = " << params.encoding           << endl;
    out << "   verify               = " << params.verify             << endl;
    out << "   sequential           = " << params.sequential         << endl;
    out << "   chop_libc            = " << params.chop_libc          << endl;
    out << "   regions              = " << params.regions            << endl;
    out << "   sampling             = " << params.sampling           << endl;
    out << "   topo                 = " << params.topo               << endl;

    out << "   ampl                 = " << params.ampl               << endl;
    if (params.ampl) {
      out << "     confidence         = " << params.confidence         << endl;
      out << "     error              = " << params.error              << endl;
      out << "     normalized_error   = " << params.normalized_error   << endl;
      out << "     windows_per_update = " << params.windows_per_update << endl;
      out << "     ampl_stats         = " << params.ampl_stats         << endl;
      out << "     ampl_trace         = " << params.ampl_trace         << endl;
      out << "     ampl_guide         = ";

      const set<effort_key>& keys(params.guide_keys());

      set<effort_key>::const_iterator i=keys.begin();

      if (i != keys.end()) 
        out << *i++ << endl;;

      while (i != keys.end()) {
        out << "                          " << *i++ << endl;
      }
      out << endl;
    }

    return out;
  }

  /// Arguments for effort module configuration.
  config_desc *effort_params::get_config_arguments() {
    static config_desc args[] = {
      config_desc("rows_per_process",   &this->rows_per_process),
      config_desc("verify",             &this->verify),
      config_desc("pass_limit",         &this->pass_limit),
      config_desc("scale",              &this->scale),
      config_desc("sequential",         &this->sequential),
      config_desc("encoding",           &this->encoding),
      config_desc("metrics",            &this->metrics),
      config_desc("chop_libc",          &this->chop_libc),
      config_desc("regions",            &this->regions),
      config_desc("sampling",           &this->sampling),
      config_desc("topo",               &this->topo),
      config_desc("ampl",               &this->ampl),
      config_desc("confidence",         &this->confidence),
      config_desc("error",              &this->error),
      config_desc("normalized_error",   &this->normalized_error),
      config_desc("windows_per_update", &this->windows_per_update),
      config_desc("ampl_stats",         &this->ampl_stats),
      config_desc("ampl_trace",         &this->ampl_trace),
      config_desc("ampl_guide",         &this->ampl_guide),
      config_desc()
    };
    return args;
  }


  void effort_params::parse() {
    parse_keys();
    parse_metrics();
  }


  void effort_params::parse_keys() {
    parse_effort_keys(ampl_guide, inserter(guide_list, guide_list.end()));
  }


  void effort_params::parse_metrics() {
    // Split out what the user asked for into separate strings.
    vector<string> metric_names;
    split(metrics, " ,", metric_names);

    for (size_t i=0; i < metric_names.size(); i++) {
      Metric m(metric_names[i]);
      metric_to_index[m] = i;
      if (m == Metric::time()) {
        have_time = true;
      } else {
        all_metrics.push_back(m);
      }
    }

    parsed = true;
  }

} //namespace
