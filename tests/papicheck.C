
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
