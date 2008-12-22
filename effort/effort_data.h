#ifndef EFFORT_DATA_H
#define EFFORT_DATA_H

#include <stdint.h>
#include <map>
#include "effort_key.h"
#include "effort_record.h"

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

  private:

    /// Functor for invoking commit on an effort record.
    struct committer {
      size_t count;
      committer(size_t c) : count(c) { }
      void operator()(effort_map::value_type& p) {
        p.second.commit(count);
      }
    };
  };

} // namespace
  
#endif //EFFORT_MAP_H
