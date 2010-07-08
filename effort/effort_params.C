/////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010, Lawrence Livermore National Security, LLC.  
// Produced at the Lawrence Livermore National Laboratory  
// Written by Todd Gamblin, tgamblin@llnl.gov.
// LLNL-CODE-417602
// All rights reserved.  
// 
// This file is part of Libra. For details, see http://github.com/tgamblin/libra.
// Please also read the LICENSE file for further information.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the disclaimer below.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the disclaimer (as noted below) in the documentation and/or other materials
//    provided with the distribution.
//  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
//    or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
/////////////////////////////////////////////////////////////////////////////////////////////////
#include "effort_params.h"
#include <iomanip>
#include <cstring>
using namespace std;

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef HAVE_SPRNG
#include "sampler.h"
#endif // HAVE_SPRNG

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
    out << "   dump_keys            = " << params.dump_keys          << endl;

    out << "   ampl                 = " << params.ampl               << endl;
    if (params.ampl) {
      out << "     confidence         = " << params.confidence         << endl;
      out << "     error              = " << params.error              << endl;
      out << "     normalized_error   = " << params.normalized_error   << endl;
      out << "     windows_per_update = " << params.windows_per_update << endl;
      out << "     ampl_stats         = " << params.ampl_stats         << endl;
      out << "     ampl_trace         = " << params.ampl_trace         << endl;
      out << "     ampl_strata        = " << params.ampl_strata        << endl;
      out << "     ampl_sig_level     = " << params.ampl_sig_level     << endl;
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
      config_desc("dump_keys",          &this->dump_keys),
      config_desc("ampl",               &this->ampl),
      config_desc("confidence",         &this->confidence),
      config_desc("error",              &this->error),
      config_desc("normalized_error",   &this->normalized_error),
      config_desc("windows_per_update", &this->windows_per_update),
      config_desc("ampl_stats",         &this->ampl_stats),
      config_desc("ampl_trace",         &this->ampl_trace),
      config_desc("ampl_strata",        &this->ampl_strata),
      config_desc("ampl_sig_level",     &this->ampl_sig_level),
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
#ifdef HAVE_SPRNG
    parse_effort_keys(ampl_guide, inserter(guide_list, guide_list.end()));
#endif // HAVE_SPRNG
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
