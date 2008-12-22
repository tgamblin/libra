#ifndef EFFORT_METADATA_H
#define EFFORT_METADATA_H

#include <iostream>
#include "Callpath.h"
#include <string>
#include <ostream>

namespace effort {
  
  /// Class for all metadata associated with effort files.
  /// Separate from effort key because keys require the runtime
  /// to be meaningful.
  struct effort_metadata {
    std::string metric;
    int type;
    Callpath start_path;
    Callpath end_path;
    
    effort_metadata();
    
    effort_metadata(const std::string& metric, int type, const Callpath& start, const Callpath& end);
    
    /// Writes out this metadata to a stream
    void write_out(std::ostream& out);
    
    /// Reads just the metadata out of a compressed effort file.
    static void read_in(std::istream& in, effort_metadata& md);
  };

  std::ostream& operator<<(std::ostream& out, const effort_metadata& md);

  /// Utility function to parse effort filenames output by thetool
  /// TODO : provide something to make a filename here too.
  bool parse_filename(const std::string& filename, std::string *metric = NULL, int *type = NULL, int *number = NULL);

}

#endif //EFFORT_METADATA_H
