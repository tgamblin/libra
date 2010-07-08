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
#ifndef FRAME_DB_H
#define FRAME_DB_H

#include <string>
#include <map>
#include "FrameId.h"
#include "FrameInfo.h"

namespace effort {
  
  /// Reads in and holds info about a file full of frame info output by
  /// libra-build-viewer-data.
  class FrameDB {
  public:
    /// Constructs an empty FrameDB
    ~FrameDB();
    
    /// Gets symbol info for a particular frame.
    FrameInfo info_for(FrameId key);

    /// De-aliases key if its module has an alias.
    FrameId unalias(FrameId key);

    /// Number of mappings in the db.
    size_t size();
    
    /// Loads a file full of symbol data.  Returns NULL on error.
    static FrameDB *load_from_file(const std::string& filename);

  private:
    typedef std::map<FrameId, FrameInfo> frame_map;
    typedef std::map<ModuleId, ModuleId> alias_map;

    FrameDB();

    void add_info(const std::string& line);       /// parses a frame info line
    void add_alias(const std::string& line);      /// parses an alias mapping line

    frame_map frames;
    alias_map aliases;
  }; // class FrameDB
  
} // namespace effort

#endif // FRAME_DB_H
