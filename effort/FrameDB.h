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
