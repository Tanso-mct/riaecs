#pragma once
#include "riaecs/include/dll_config.h"

#include "riaecs/include/registry.h"
#include "riaecs/include/ecs.h"

#include <memory>

namespace riaecs
{
    extern RIAECS_API std::unique_ptr<IComponentFactoryRegistry> gComponentFactoryRegistry;
    extern RIAECS_API std::unique_ptr<IComponentMaxCountRegistry> gComponentMaxCountRegistry;

    template <typename COMPONENT, size_t MAX_COUNT> 
    class ComponentRegistrar
    {
    private:
        size_t componentID_;

    public:
        ComponentRegistrar()
        {
            componentID_ = gComponentFactoryRegistry->Add(std::make_unique<ComponentFactory<COMPONENT>>());
            gComponentMaxCountRegistry->Add(std::make_unique<size_t>(MAX_COUNT));
        }

        size_t operator()() const
        {
            return componentID_;
        }
    };

} // namespace riaecs