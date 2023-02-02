#pragma once

#include <winbase.h>
#include <stringapiset.h>

#include <map>
#include <string>
#include <string_view>
#include <wil/resource.h>

#include "string_utility.h"

namespace jtasler::environment_utility
{
    [[nodiscard]]
    inline std::wstring get_variable(PCWSTR variableName)
    {
        std::wstring value;
        value.resize(::GetEnvironmentVariableW(variableName, nullptr, 0));
        value.resize(::GetEnvironmentVariableW(variableName, value.data(), static_cast<uint32_t>(value.capacity())));
        return value;
    }

    [[nodiscard]]
    inline std::map<std::wstring, std::wstring, string_utility::ordinal_ignore_case_less> get_variables()
    {
        std::invoke_result_t<decltype(get_variables)> result;

        auto environmentBlock = ::GetEnvironmentStringsW();
        auto freeBlock = wil::scope_exit([&]
        {
            LOG_IF_WIN32_BOOL_FALSE(::FreeEnvironmentStringsW(environmentBlock));
        });

        auto pointer = environmentBlock;
        for (std::wstring_view variable(pointer); variable.length() != 0; variable = { pointer += variable.length() + 1 })
        {
            auto equalSignPosition = variable.find(L'=');
            if ((equalSignPosition == 0) || (equalSignPosition == (variable.length() - 1)))
            {
                continue;
            }
            else if (equalSignPosition == std::wstring::npos)
            {
                break;
            }

            std::wstring_view key(variable.data(), equalSignPosition);
            std::wstring_view value(variable.data() + equalSignPosition + 1, variable.length() - equalSignPosition - 1);
            result.emplace(key, value);
        }

        for (auto& [key, value] : result)
        {
            wprintf(L"%ls=%ls\n", key.c_str(), value.c_str());
        }

        return result;
    }

}
