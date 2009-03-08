#include "Module.h"


class Module : public UniqueId<Module> {
  Module(const std::string& id) : UniqueId<Module><id> { }
};
