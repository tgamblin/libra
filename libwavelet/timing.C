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
#include "timing.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H

#include <cmath>
using namespace std;


#if defined(BLUEGENE_L)
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


#elif defined(BLUEGENE_P)
// -------------------------------------------------------- //
// Timing code for BlueGene/P
// -------------------------------------------------------- //
#define SPRN_TBRL         0x10C        // Time Base Read Lower Register (user & sup R/O)
#define SPRN_TBRU         0x10D        // Time Base Read Upper Register (user & sup R/O)
#define BGP_NS_PER_CYCLE  (1.0/0.85)   // Nanoseconds per cycle on BGP (850Mhz clock)

#define _bgp_mfspr(SPRN) ({ \
   unsigned int tmp; \
   do { \
      asm volatile ("mfspr %0,%1" : "=&r" (tmp) : "i" (SPRN) : "memory" ); \
   } while(0); \
   tmp; \
})

union bgp_time_reg {
  unsigned int ul[2];
  unsigned long long ull;
};

static inline unsigned long long timebase() {
  bgp_time_reg reg;
  unsigned int utmp;
  
  do {
    utmp      = _bgp_mfspr(SPRN_TBRU);
    reg.ul[1] = _bgp_mfspr(SPRN_TBRL);
    reg.ul[0] = _bgp_mfspr(SPRN_TBRU);
  }
  while( utmp != reg.ul[0] );
  
  return reg.ull;
}

timing_t get_time_ns() {
  return llround(BGP_NS_PER_CYCLE * timebase());
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
  return (ts.tv_sec * 1000000000ll + ts.tv_nsec);
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
  return tv.tv_sec * 1000000000ll + tv.tv_usec * 1000ll;
}


#else // if we get to here, we don't even have gettimeofday.
#error "NO SUPPORTED TIMING FUNCTIONS FOUND!"
#endif // types of timers
