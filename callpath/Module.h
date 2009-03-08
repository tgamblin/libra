#ifndef MODULE_H
#define MODULE_H

#include "UniqueId.h"

class Module : public UniqueId<Module> {
  Module(const std::string& id);
};

#endif // MODULE_H
