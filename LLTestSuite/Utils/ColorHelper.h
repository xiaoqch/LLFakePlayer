#pragma once
#include <MC/ColorFormat.hpp>

namespace ColorHelper
{

    template <typename T>
    inline std::string aqua(T const& arg) { return fmt::format("{}{}§r", ColorFormat::AQUA, arg); }
    template <typename T>
    inline std::string black(T const& arg) { return fmt::format("{}{}§r", ColorFormat::BLACK, arg); }
    template <typename T>
    inline std::string blue(T const& arg) { return fmt::format("{}{}§r", ColorFormat::BLUE, arg); }
    template <typename T>
    inline std::string bold(T const& arg) { return fmt::format("{}{}§r", ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string dark_aqua(T const& arg) { return fmt::format("{}{}§r", ColorFormat::DARK_AQUA, arg); }
    template <typename T>
    inline std::string dark_blue(T const& arg) { return fmt::format("{}{}§r", ColorFormat::DARK_BLUE, arg); }
    template <typename T>
    inline std::string dark_gray(T const& arg) { return fmt::format("{}{}§r", ColorFormat::DARK_GRAY, arg); }
    template <typename T>
    inline std::string dark_green(T const& arg) { return fmt::format("{}{}§r", ColorFormat::DARK_GREEN, arg); }
    template <typename T>
    inline std::string dark_purple(T const& arg) { return fmt::format("{}{}§r", ColorFormat::DARK_PURPLE, arg); }
    template <typename T>
    inline std::string dark_red(T const& arg) { return fmt::format("{}{}§r", ColorFormat::DARK_RED, arg); }
    template <typename T>
    inline std::string gold(T const& arg) { return fmt::format("{}{}§r", ColorFormat::GOLD, arg); }
    template <typename T>
    inline std::string gray(T const& arg) { return fmt::format("{}{}§r", ColorFormat::GRAY, arg); }
    template <typename T>
    inline std::string green(T const& arg) { return fmt::format("{}{}§r", ColorFormat::GREEN, arg); }
    template <typename T>
    inline std::string italic(T const& arg) { return fmt::format("{}{}§r", ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string light_purple(T const& arg) { return fmt::format("{}{}§r", ColorFormat::LIGHT_PURPLE, arg); }
    template <typename T>
    inline std::string minecoin_gold(T const& arg) { return fmt::format("{}{}§r", ColorFormat::MINECOIN_GOLD, arg); }
    template <typename T>
    inline std::string obfuscated(T const& arg) { return fmt::format("{}{}§r", ColorFormat::OBFUSCATED, arg); }
    template <typename T>
    inline std::string red(T const& arg) { return fmt::format("{}{}§r", ColorFormat::RED, arg); }
    template <typename T>
    inline std::string reset(T const& arg) { return fmt::format("{}{}§r", ColorFormat::RESET, arg); }
    template <typename T>
    inline std::string white(T const& arg) { return fmt::format("{}{}§r", ColorFormat::WHITE, arg); }
    template <typename T>
    inline std::string yellow(T const& arg) { return fmt::format("{}{}§r", ColorFormat::YELLOW, arg); }

    template <typename T>
    inline std::string aqua_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::AQUA, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string black_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::BLACK, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string blue_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::BLUE, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string dark_aqua_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::DARK_AQUA, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string dark_blue_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::DARK_BLUE, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string dark_gray_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::DARK_GRAY, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string dark_green_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::DARK_GREEN, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string dark_purple_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::DARK_PURPLE, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string dark_red_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::DARK_RED, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string gold_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::GOLD, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string gray_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::GRAY, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string green_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::GREEN, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string italic_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::ITALIC, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string light_purple_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::LIGHT_PURPLE, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string minecoin_gold_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::MINECOIN_GOLD, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string obfuscated_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::OBFUSCATED, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string red_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::RED, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string reset_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::RESET, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string white_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::WHITE, ColorFormat::BOLD, arg); }
    template <typename T>
    inline std::string yellow_bold(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::YELLOW, ColorFormat::BOLD, arg); }

    template <typename T>
    inline std::string aqua_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::AQUA, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string black_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::BLACK, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string blue_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::BLUE, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string dark_aqua_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::DARK_AQUA, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string dark_blue_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::DARK_BLUE, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string dark_gray_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::DARK_GRAY, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string dark_green_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::DARK_GREEN, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string dark_purple_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::DARK_PURPLE, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string dark_red_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::DARK_RED, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string gold_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::GOLD, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string gray_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::GRAY, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string green_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::GREEN, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string light_purple_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::LIGHT_PURPLE, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string minecoin_gold_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::MINECOIN_GOLD, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string obfuscated_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::OBFUSCATED, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string red_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::RED, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string reset_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::RESET, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string white_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::WHITE, ColorFormat::ITALIC, arg); }
    template <typename T>
    inline std::string yellow_italic(T const& arg) { return fmt::format("{}{}{}§r", ColorFormat::YELLOW, ColorFormat::ITALIC, arg); }

    inline std::string& transformToConsole(std::string& str)
    {
        return ColorFormat::convertToColsole(str, false);
    }

}; // namespace ColorHelper


