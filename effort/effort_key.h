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
#ifndef EFFORT_KEY_H
#define EFFORT_KEY_H

#include "Callpath.h"
#include "Metric.h"
#include "ModuleId.h"
#include "string_utils.h"

#include "libra-config.h"
#ifdef LIBRA_HAVE_MPI
#include <mpi.h>
#endif // LIBRA_HAVE_MPI

/// integer type for progress to pass to MPI_Pcontrol()
#define PROGRESS_TYPE 0

namespace effort {
  
  /// An effort key describes an effort region in the code.  Its start_signature
  /// and end_signature are dynamic callpaths indicating what events started and
  /// ended the effort region.  Type is a user-defined value for phase id, which
  /// can be used to further differentiate effort.
  struct effort_key {
    Metric metric;
    int type;
    Callpath start_path;
    Callpath end_path;

    /// Constructs an effort key with two null callpaths.
    effort_key(Metric m = Metric::time(), int t = PROGRESS_TYPE) : metric(m), type(t) { }
    
    /// Value constructor.
    effort_key(Metric m, int t, Callpath start, Callpath end);

    /// Copy constructor.
    effort_key(const effort_key& other);

    // Assignment.
    effort_key& operator=(const effort_key& other);

    /// True if the effort key represents communciation.
    bool isComm() const;

    /// Writes out this effort id to a stream
    void write_out(std::ostream& out);
    
    /// Reads an effort id out of a stream.
    static void read_in(std::istream& in, effort_key& md);

#ifdef LIBRA_HAVE_MPI
    // For MPI packing -- you'll need Callpath::pack_modules() to get a 
    // module_map and unpack properly.
    int packed_size(MPI_Comm comm) const;
    void pack(void *buf, int bufsize, int *position, MPI_Comm comm) const;
    static effort_key unpack(const ModuleId::id_map& modules, void *buf, int bufsize, int *position, MPI_Comm comm);
#endif // LIBRA_HAVE_MPI
  };

  /// compares type and signatures to make sure they're the same.
  bool operator==(const effort_key& lhs, const effort_key& rhs);

  /// Less than comparator for effort keys.  Used in effort_map below.
  bool operator<(const effort_key& lhs, const effort_key& rhs);

  // other comparators derived from above for convenience
  inline bool operator!=(const effort_key& lhs, const effort_key& rhs) { 
    return !(lhs == rhs); 
  }
  
  inline bool operator>(const effort_key& lhs, const effort_key& rhs) { 
    return !(lhs == rhs) && !(lhs < rhs); 
  }

  /// Outputs effort key as as tring
  std::ostream& operator<<(std::ostream& out, const effort_key& key);
  
  /// Full (string, frame by frame) compare of effort keys.  Use this to make
  /// Order consistent across nodes.
  struct effort_key_full_lt {
    callpath_path_lt path_lt;
    dereference_lt metric_lt;
    bool operator()(const effort_key& lhs, const effort_key& rhs);
  };
  

  /// Utility function to identify valid filenames output by the tool
  /// TODO : provide something to make a filename here too.
  bool parse_filename(const std::string& filename, 
                      std::string *metric = NULL, int *type = NULL, int *number = NULL);

  
  template <class OutputIterator>
  void parse_effort_keys(const char *str, OutputIterator output) {
    if (!str) return;

    std::vector<std::string> key_strings;
    stringutils::split(str, ",", key_strings);
    
    for (size_t k=0; k < key_strings.size(); k++) {
      std::vector<std::string> path_strings;
      stringutils::split_str(key_strings[k], "=>", path_strings);

      Callpath start(make_path(path_strings[0]));
      Callpath end(start);
      if (path_strings.size() > 1) {
        end = make_path(path_strings[1]);        
      }
      
      *output++ = effort_key(Metric::time(), 0, start, end);
    }
  }

} // namespace

#endif // EFFORT_KEY_H
