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
#ifndef ENV_CONFIG_H
#define ENV_CONFIG_H

#include <stdlib.h>
#include <stdint.h>

/// Types for configuration data.  
/// CONFIG_BOOL can be "TRUE" or "FALSE", and translates to 0 or 1
/// CONFIG_INT can be any int-parsable string, and is eval'd w/atoi
typedef enum {
  CONFIG_NULL,               // Use for null-termination of configuration
  CONFIG_BOOL,               // if "TRUE" or "true", then 1, otherwise 0. Dest is an int*.
  CONFIG_bool,               // if "TRUE" or "true", then true, otherwise false. Dest is bool*.
  CONFIG_INT,                // int parsable by strtol().
  CONFIG_LONG_LONG,          // long long parsable by strtoll().
  CONFIG_STRING,             // standard c string
  CONFIG_DBL                 // double parsable by strtod().
} config_type;


struct config_desc {
  const char *name;      // Name of configuration argument in .pnmpi_conf
  config_type type;      // Type of configuration variable.
  void *dest;            // value of configuration variable

#ifdef __cplusplus
  // These overloads let omit the explicit type.
  config_desc(const char *n, bool *d)        : name(n), type(CONFIG_bool),      dest((void*)d) { }
  config_desc(const char *n, int *d)         : name(n), type(CONFIG_INT),       dest((void*)d) { }
  config_desc(const char *n, long long *d)   : name(n), type(CONFIG_LONG_LONG), dest((void*)d) { }
  config_desc(const char *n, const char** d) : name(n), type(CONFIG_STRING),    dest((void*)d) { }
  config_desc(const char *n, double* d)      : name(n), type(CONFIG_DBL),       dest((void*)d) { }

  // default constructs null terminator.
  config_desc() : name(NULL), type(CONFIG_NULL), dest(NULL) { }
#endif // __cplusplus
};


#ifdef __cplusplus
extern "C" {
#endif

  ///
  /// Takes configuration from environment variables.
  ///
  int env_get_configuration(struct config_desc *configuration);

#ifdef __cplusplus
}
#endif

#endif // ENV_CONFIG_H
