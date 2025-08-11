#pragma once

#include "riaecs/include/interfaces/registry.h"
#include "riaecs/include/interfaces/loader.h"

#include <memory>
#include <string_view>

namespace riaecs
{
    class IFileData
    {
    public:
        virtual ~IFileData() = default;
    };

    using IFileLoader = ILoader<std::unique_ptr<IFileData>, std::string_view>;
    using IFileLoaderRegistry = IRegistry<IFileLoader>;

} // namespace riaecs