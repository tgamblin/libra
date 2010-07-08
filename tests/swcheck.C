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

