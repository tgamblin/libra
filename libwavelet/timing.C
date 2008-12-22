#include "timing.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H


#ifdef __blrts__
// -------------------------------------------------------- //
// Timing code for BlueGene/L
// -------------------------------------------------------- //
#include <rts.h>

// this will return number of nanoseconds in a single BGL cycle
// use for converting from cycle units returned by rts_gettimebase
// to nanoseconds.
static double get_ns_per_cycle() {
  BGLPersonality personality;
  if (rts_get_personality(&personality, sizeof(personality)) != 0)
    return 0;
  return 1.0e9/((double) personality.clockHz);
}

// returns time in nanoseconds.
timing_t get_time_ns () {
  static double ns_per_cycle = get_ns_per_cycle();
  return (timing_t)(ns_per_cycle * rts_get_timebase());
}



#elif (defined(HAVE_CLOCK_GETTIME) || defined(HAVE_LIBRT))
// -------------------------------------------------------- //
// Timing code using Linux hires timers.
// -------------------------------------------------------- //

#include <ctime>
#include <sys/time.h>

timing_t get_time_ns() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (ts.tv_sec * 1000000000 + ts.tv_nsec);
}


#elif defined(HAVE_GETTIMEOFDAY)
// -------------------------------------------------------- //
// Generic timing code using gettimeofday.
// -------------------------------------------------------- //

#include <ctime>
#include <sys/time.h>

timing_t get_time_ns() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000000 + tv.tv_usec * 1000;
}


#else // if we get to here, we don't even have gettimeofday.
#error "NO SUPPORTED TIMING FUNCTIONS FOUND!"
#endif // types of timers
