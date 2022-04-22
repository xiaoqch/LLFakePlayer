#pragma once
#include "pch.h"
#include <MC/ColorFormat.hpp>

namespace TestTools
{
template <typename... Args>
inline void success(const char* formatStr, const Args&... args)
{
    static string const green = ColorFormat::consoleCodeFromColorCode(ColorFormat::GREEN);
    static string const reset = ColorFormat::consoleCodeFromColorCode(ColorFormat::RESET);

    if constexpr (0 == sizeof...(args))
    {
        logger.info << green << formatStr << reset << logger.endl;
    }
    else
    {
        std::string str = fmt::format(std::string(formatStr), args...);
        logger.info << green << str << reset << logger.endl;
    }
}
template <typename... Args>
inline void error(const char* formatStr, const Args&... args)
{
    static string const red = ColorFormat::consoleCodeFromColorCode(ColorFormat::DARK_RED);
    static string const reset = ColorFormat::consoleCodeFromColorCode(ColorFormat::RESET);

    if constexpr (0 == sizeof...(args))
    {
        logger.error << red << formatStr << reset << logger.endl;
    }
    else
    {
        std::string str = fmt::format(std::string(formatStr), args...);
        logger.error << red << str << reset << logger.endl;
    }
}
}