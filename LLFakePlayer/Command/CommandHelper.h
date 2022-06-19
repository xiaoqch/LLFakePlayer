#pragma once

#define KEY_NO_TARGET "%commands.generic.noTargetMatch"
#define KEY_TOO_MANY_TARGER "%commands.generic.tooManyTargets"
#define FAKEPLAYER_LIST_SOFT_ENUM_NAME "LLFakePlayerList"

#define AssertUniqueTarger(results)         \
    if (results.empty())                    \
        return output.error(KEY_NO_TARGET); \
    else if (results.count() > 1)           \
        return output.error(KEY_TOO_MANY_TARGER);

#define successf(...) success(fmt::format(__VA_ARGS__))
#define errorf(...) error(ColorHelper::red(fmt::format(__VA_ARGS__)))

#include <RegCommandAPI.h>
using namespace RegisterCommandHelper;

template <size_t N>
struct LowerString
{
    char buf[N + 1]{};
    constexpr LowerString(char const* s)
    {
        for (unsigned i = 0; i != N; ++i)
        {
            if (s[i] >= 'A' && s[i] <= 'Z')
                buf[i] = s[i] + 32;
            else
                buf[i] = s[i];
        }
    }
    constexpr operator char const*() const
    {
        return buf;
    }
    constexpr operator std::string_view() const
    {
        return std::string_view(buf);
    }
};

namespace magic_enum
{
template <auto V>
[[nodiscard]] constexpr auto lower_enum_name() noexcept -> detail::enable_if_enum_t<decltype(V), string_view>
{
    constexpr auto name = enum_name(V);
    constexpr auto data = name.data();
    constexpr auto size = name.size();
    return LowerString<size>{data};
}
} // namespace magic_enum