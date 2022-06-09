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