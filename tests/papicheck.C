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

#include <papi.h>
#include <unistd.h>
#include <sys/time.h>

#include <iostream>
#include <vector>
#include <string>
using namespace std;


long long get_us() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec * 1000000ll + tv.tv_usec;
}


int main(int argc, char **argv) {
  // Initialize the PAPI library 
  int retval = PAPI_library_init(PAPI_VER_CURRENT);

  if (retval != PAPI_VER_CURRENT && retval > 0) {
    cerr << "PAPI library version mismatch!" << endl;
    cerr << "retval was: " << retval << endl;
    exit(1);
  }

  if (retval < 0) {
    cerr << "PAPI Initialization error!" << endl;
    cerr << "retval was: " << retval << endl;
    exit(1);
  }
  
  int event_set = PAPI_NULL;
  size_t num_events = 0;

  retval = PAPI_create_eventset(&event_set);

  if (retval != PAPI_OK) {
    cerr << "Failed to create event set" << endl;
    cerr << "retval was: " << retval << endl;
    exit(1);
  }

  for (int i=1; i < argc; i++) {
    int event;
    char *name = argv[i];

    retval = PAPI_event_name_to_code(name, &event);
    if (retval != PAPI_OK) {
      cerr << "Error adding event: " << name << endl;
      continue;
    }
    
    // Add Total Instructions Executed to our EventSet
    retval = PAPI_add_event(event_set, event);
    if (retval != PAPI_OK) {
      cerr << "Failed to add event to set: " << name
           << " (" << event << ")" << endl;
      cerr << "retval was: " << retval << endl;
      exit(1);
    } else {
      num_events++;
    }
  }


  long long values[num_events];
  for (size_t i=0; i < num_events; i++) values[i] = 0;


  int i;
  for (i=0; i < 10; i++) {
    retval = PAPI_start(event_set);
    if (retval != PAPI_OK) {
      cerr << "Coudln't start event set!" << endl;
      cerr << "retval was: " << retval << endl;
      exit(1);
    }

    long long start = get_us();
    long long now;
    do {
      now = get_us();
    } while (now - start < 1000000);

    retval = PAPI_stop(event_set, values);
    if (retval != PAPI_OK) {
      cerr << "Coudln't accum event set!" << endl;
      exit(1);
    }
    
    printf("%3d", i);
    for (size_t e=0; e < num_events; e++) {
      printf("%12lld", values[e]);
    }
    printf("\n");
  }
  
  PAPI_cleanup_eventset(event_set);
  PAPI_destroy_eventset(&event_set);
}
