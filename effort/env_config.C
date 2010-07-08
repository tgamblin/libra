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
#include "env_config.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <cstring>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <cstdio>
using namespace std;


template <class Fn>
int get_configuration(Fn get_value, struct config_desc *configuration) {
  char *err;
  int errval = 0;

  for (config_desc *record = configuration; 
       record->name && record->type && record->dest;
       record++) 
  {
    char *value;
    int arg_err = get_value(&value, record->name);
    if (arg_err != 0) {
      continue;
    }
    
    switch (record->type) {
    case CONFIG_BOOL:
      *((int*)record->dest) = (value && strcasecmp("true", value) == 0);
      break;
      
    case CONFIG_bool:
      *((bool*)record->dest) = (value && strcasecmp("true", value) == 0);
      break;
      
    case CONFIG_INT:
      *((int*)record->dest) = (int)strtol(value, &err, 10);
      if (*err) {
        fprintf(stderr, "Error: Invalid integer value for %s.\n", record->name);
        errval = 1;
      }
      break;
      
    case CONFIG_LONG_LONG:
      *((long long*)record->dest) = strtoll(value, &err, 10);
      if (*err) {
        fprintf(stderr, "Error: Invalid long long value for %s.\n", record->name);
        errval = 1;
      }
      break;
      
    case CONFIG_DBL:
      *((double*)record->dest) = strtod(value, &err);
      if (*err) {
        fprintf(stderr, "Error: Invalid double value for %s.\n", record->name);
        errval = 1;
      }
      break;
      
    case CONFIG_STRING:
      *((char**)record->dest) = strdup(value);
      break;
      
    default:
      fprintf(stderr, "Error: invalid confgiuration record.");
      assert(0);
      break;
    }
  }

  return errval;  
}


struct env_get_value {
  int operator()(char **value, const char *name) {
    char *val = getenv(name);
    if (val) {
      *value = val;
      return 0;
    } else {
      return 1;
    }
  }
};


int env_get_configuration(struct config_desc *configuration) {
  return get_configuration(env_get_value(), configuration);
}
