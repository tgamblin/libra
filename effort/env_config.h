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
