#ifndef EFFORT_DATA_H
#define EFFORT_DATA_H

#include <stdint.h>
#include <map>
#include <string>
#include <ostream>
#include "effort_key.h"
#include "effort_record.h"
#include "ezw.h"

namespace effort {

  typedef std::map<effort_key, effort_record> effort_map;

  ///
  /// This is the central bookkeeping structure for the effort module.
  /// Maps effort keys to effort_records and keeps track of how many progress
  /// steps have happened so far.
  ///
  struct effort_data {
  public:
    typedef effort_map::iterator iterator;

    effort_map emap;          /// Recorded effort values within the map
    size_t progress_count;    /// Number or progress steps so far.

    /// Constructor just inits progress count to zero.
    effort_data();

    /// Destructor
    ~effort_data() { }

    /// Calls commit on each effort record and increments the current progress count.
    /// If progress_count is provided, pads effort values with zeros up to the 
    /// specified progress step.
    void progress_step(size_t step_to = 0);

    effort_record& operator[](const effort_key& key) {
      return emap[key];
    }

    effort_data::iterator begin() { return emap.begin(); }
    effort_data::iterator end()   { return emap.end(); }
    size_t size() { return emap.size(); }
    bool contains(const effort_key& key) {
      return emap.find(key) != emap.end();
    }
    void clear() { emap.clear(); }
    
    /// Writes keys and values for current progress step
    void write_current_step(std::ostream& out);

    /// Loads an effort_log full of keys from a particular directory full of 
    /// effort files. 
    /// NOTE: This does not fill the log up with values. See parallel_decompressor.
    static void load_keys(const std::string& dirname, effort_data& log, 
                          wavelet::ezw_header& header, 
                          std::map<effort_key, std::string> *filenames = NULL);

  private:

    /// Functor for invoking commit on an effort record.
    struct committer {
      size_t count;
      committer(size_t c) : count(c) { }
      void operator()(effort_map::value_type& p) {
        p.second.commit(count);
      }
    };


    /// Functor for writing out current step from effort_map
    struct write_current {
      std::ostream& out;
      std::string indent;

      write_current(std::ostream& o, std::string i="") : out(o), indent(i) { }
      void operator()(effort_map::value_type& p) {
        out << indent << "[" << p.first << "]" << p.second.current << std::endl;
      }
    };
    
  };

} // namespace
  
#endif //EFFORT_MAP_H
