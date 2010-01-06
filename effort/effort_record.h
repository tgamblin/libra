#ifndef EFFORT_RECORD_H
#define EFFORT_RECORD_H

#include <vector>

namespace effort {
  
  /// An effort record keeps track of the value of one particular type of effort
  /// over all timesteps.  These are stored in a map and indexed by effort_keys.
  struct effort_record {
    double current;         /// Current value of this type of effort.
    std::vector<double> values;  /// Effort values indexed by progress step

    /// Constructs an emty effort record
    effort_record() : current(0) { }
    
    /// Constructs an effort record with all values zero
    effort_record(int size) : current(0), values(size, 0) { }
    
    /// Destructor
    ~effort_record() { }
    
    /// Pushes current effort on back of values, resets current effort to zero.
    void commit(size_t progress_count);
    
    /// Gets the effort for a particular progress step.
    double& operator[](int p) { return values[p]; }

    /// Gets the size so far.
    size_t size() { return values.size(); }
    
    /// Adds to current value of this effort.
    void operator+=(double value) { current += value; }
  };
  
  
} // namespace

#endif // EFFORT_RECORD_H
