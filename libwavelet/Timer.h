#ifndef TIMER_H
#define TIMER_H

#include "timing.h"

#include <vector>
#include <string>
#include <iostream>

class Timer {
  struct Element {
    std::string name;      /// name of timing (user-supplied)
    timing_t time_ns;      /// elapsed time in the region, 
    Element(const std::string& name, timing_t time_ns);
    Element(const Element& other);
    Element& operator=(const Element& other);
  };
  
  std::vector<Element> times;      /// All timings recorded
  timing_t start_time;      /// Last time restart() or record() was called.
  
public:
  Timer();

  ~Timer();

  /// Sets timer's start time to now.  Affects subsequent calls to record().
  void restart();

  /// Records time since start or last call to record.
  const Element record(const std::string& name);

  /// Returns the i-th timing recorded (starting with 0)
  const Element& operator[](size_t i) const;

  /// Prints all timings (nicely formatted, in sec) to a file.
  void write(std::ostream& out = std::cout) const;
};

/// Syntactic sugar; calls write on the timer and passes the ostream.
inline std::ostream& operator<<(std::ostream& out, const Timer& timer) {
  timer.write(out);
}


#endif // TIMER_H
