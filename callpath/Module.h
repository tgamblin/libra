#ifndef MODULE_H
#define MODULE_H

#include "UniqueId.h"

class Module : public UniqueId<Module> {
public:
  Module(const std::string& id);
};

#endif // MODULE_H
