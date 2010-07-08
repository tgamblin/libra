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
#include "effort_key.h"
#include "Callpath.h"

#ifdef LIBRA_HAVE_MPI
#include "mpi_utils.h"
#endif // LIBRA_HAVE_MPI

#include "string_utils.h"
using namespace stringutils;

#include "io_utils.h"
using namespace wavelet;

using namespace std;

namespace effort {

  effort_key::effort_key(Metric met, int t, Callpath start, Callpath end) 
    : metric(met), type(t), start_path(start), end_path(end) 
  { }

  bool effort_key::isComm() const {
    return (start_path == Callpath::null()) && (end_path == Callpath::null());
  }


  effort_key::effort_key(const effort_key& other) 
    : metric(other.metric), 
      type(other.type), 
      start_path(other.start_path), 
      end_path(other.end_path) { }


  effort_key& effort_key::operator=(const effort_key& other) {
    metric = other.metric;
    type = other.type;
    start_path = other.start_path;
    end_path = other.end_path;
    return *this;
  }
  

  bool operator==(const effort_key& lhs, const effort_key& rhs) {
    return lhs.metric   == rhs.metric 
      && lhs.type       == rhs.type 
      && lhs.start_path == rhs.start_path
      && lhs.end_path   == rhs.end_path;
  }

  bool operator<(const effort_key& lhs, const effort_key& rhs) {
    if (lhs.metric != rhs.metric) {
      return lhs.metric < rhs.metric;

    } else if (lhs.type != rhs.type) {
      return lhs.type < rhs.type;

    } else if (lhs.start_path != rhs.start_path) {
      return lhs.start_path < rhs.start_path;

    } else {
      return lhs.end_path < rhs.end_path;
    }
  }

  std::ostream& operator<<(std::ostream& out, const effort_key& key) {
    out << "[" 
        << key.metric << " "
        << key.type << " " 
        << key.start_path << " => " << key.end_path 
        << "]";

    return out;
  }
#ifdef LIBRA_HAVE_MPI
  int effort_key::packed_size(MPI_Comm comm) const {
    int size = 0;
    size += metric.packed_size(comm);           // metric
    size += mpi_packed_size(1, MPI_INT, comm);  // type
    size += start_path.packed_size(comm);       // start signature
    size += end_path.packed_size(comm);         // end signature 
    return size;
  }


  void effort_key::pack(void *buf, int bufsize, int *position, MPI_Comm comm) const {
    metric.pack(buf, bufsize, position, comm);
    PMPI_Pack(const_cast<int*>(&type), 1, MPI_INT, buf, bufsize, position, comm);
    start_path.pack(buf, bufsize, position, comm);
    end_path.pack(buf, bufsize, position, comm);
  }


  effort_key effort_key::unpack(const ModuleId::id_map& modules, void *buf, int bufsize, int *position, MPI_Comm comm) {
    effort_key key;
    key.metric = Metric::unpack(buf, bufsize, position, comm);
    PMPI_Unpack(buf, bufsize, position, &key.type, 1, MPI_INT,  comm);
    key.start_path = Callpath::unpack(modules, buf, bufsize, position, comm);
    key.end_path = Callpath::unpack(modules, buf, bufsize, position, comm);
    return key;
  }

#endif // LIBRA_HAVE_MPI

  bool effort_key_full_lt::operator()(const effort_key& lhs, const effort_key& rhs) {
    if (lhs.metric.str() != rhs.metric.str()) {
      return lhs.metric.str() < rhs.metric.str();
      
    } else if (lhs.type != rhs.type) {
      return lhs.type < rhs.type;
      
    } else if (path_lt(lhs.start_path, rhs.start_path)) {
      return true;
      
    } else if (path_lt(rhs.start_path, lhs.start_path)) {
      return false;
      
    } else if (path_lt(lhs.end_path, rhs.end_path)) {
      return true;
      
    } else if (path_lt(rhs.end_path, lhs.end_path)) {
      return false;
      
    } else {
      return true;
    }
  }


  void effort_key::write_out(ostream& out) {
    metric.write_out(out);
    wavelet::write_generic(out, type);
    start_path.write_out(out);
    end_path.write_out(out);
  }


  void effort_key::read_in(istream& in, effort_key& md) {
    md.metric = Metric::read_in(in);
    md.type = wavelet::read_generic<int>(in);
    md.start_path = Callpath::read_in(in);
    md.end_path = Callpath::read_in(in);
  }


  bool parse_filename(const string& filename, string *metric, int *out_type, int *number) {
    if (filename.find("effort") != 0) {
      return false;
    }

    vector<string> parts;
    split(filename, "-", parts);
    if (parts.size() != 4) {
      return false;
    }
  
    if (metric) {
      *metric = parts[1];
    }

    char *err;
    int t = strtol(parts[2].c_str(), &err, 10);
    if (*err) return false;
    if (out_type) *out_type = t;

    int n = strtol(parts[3].c_str(), &err, 10);
    if (*err) return false;
    if (number) *number = n;

    return true;
  }

} // namespace
