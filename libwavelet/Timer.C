#include "Timer.h"

#include <cmath>
#include <iomanip>
#include <sstream>
using namespace std;

Timer::Element::Element(const std::string& n, timing_t tns) 
  : name(n), time_ns(tns) { }

Timer::Element::Element(const Element& other)
  : name(other.name),
    time_ns(other.time_ns) { }

Timer::Element& Timer::Element::operator=(const Element& other) {
  name = other.name;
  time_ns = other.time_ns;
}



Timer::Timer() {
  restart();
}

Timer::~Timer() { }


void Timer::restart() {
  start_time = get_time_ns();
}


const Timer::Element Timer::record(const string& name) {
  timing_t now = get_time_ns();

  cerr << "now: " << now << endl;

  Element elt = Element(name, now - start_time);
  times.push_back(elt);

  cerr << "recorded: " << elt.time_ns << endl;

  start_time = now;
  return elt;
}


const Timer::Element& Timer::operator[](size_t i) const {
  return times[i];  
}


void Timer::write(std::ostream& out) const {
  if (times.empty()) return;

  size_t max_len = times[0].name.length();
  for (size_t i=1; i < times.size(); i++) {
    max_len = max(max_len, times[i].name.length());
  }

  size_t width = max_len + 2;
  
  for (size_t i=0; i < times.size(); i++) {
    ostringstream name;
    name << times[i].name << ":";
    out << left << setw(width) << name.str() << (times[i].time_ns / 1e9) << endl;
  }
}
