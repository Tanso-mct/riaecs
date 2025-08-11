#include "riaecs/src/pch.h"
#include "riaecs/include/global_registry.h"

RIAECS_API std::unique_ptr<riaecs::IComponentFactoryRegistry> riaecs::gComponentFactoryRegistry 
= std::make_unique<riaecs::ComponentFactoryRegistry>();

RIAECS_API std::unique_ptr<riaecs::IComponentMaxCountRegistry> riaecs::gComponentMaxCountRegistry 
= std::make_unique<riaecs::ComponentMaxCountRegistry>();