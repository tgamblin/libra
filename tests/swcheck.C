#include <string>
#include <cstdio>
#include <vector>
using namespace std;

#include "Symbol.h"
#include "walker.h"
#include "AddrLookup.h"
#include "frame.h"
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace Dyninst::Stackwalker;

static Walker *walker = NULL;

void baz() { 
  vector<Frame> swalk;
  walker->walkStack(swalk);

  for (unsigned i=0; i < swalk.size(); i++) {
    string name;
    swalk[i].getName(name);
    printf("%s\n", name.c_str());
  }
}
void bar() { baz(); }
void foo() { bar(); }


int main(int argc, char **argv) {
  walker = Walker::newWalker();
  AddressLookup *lookup = AddressLookup::createAddressLookup();

  vector<LoadedLibrary> libs;
  lookup->getLoadAddresses(libs);

  printf("%-20s%-20s%s\n", "Code", "Data", "Module");
  for (unsigned i=0; i < libs.size(); i++) {
    printf("0x%-18lx0x%-18lx%s\n", libs[i].codeAddr, libs[i].dataAddr, libs[i].name.c_str());
  }

  printf("\n\n");
  foo();

  exit(0);
}

