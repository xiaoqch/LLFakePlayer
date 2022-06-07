#pragma once
#include "../Version.h"

#include <LoggerAPI.h>
#include <third-party/magic_enum/magic_enum.hpp>
// Global Info
#define GITHUB_LINK "https://github.com/xiaoqch/"

// Path
#define LOG_DIR "./logs/"
#define PLUGINS_DIR "./plugins/"

// Transfer macro to string
#define __MACRO_TO_STR(str) #str
#define MACRO_TO_STR(str) __MACRO_TO_STR(str)


// Tools
#include "Utils/DebugHelper.h"

// Global logger
extern Logger logger;

// Plugin Info
#define PLUGIN_WEBSIDE GITHUB_LINK PLUGIN_NAME
#define PLUGIN_LICENCE "GPLv3"
#define PLUGIN_USAGE R"(
llfakeplayer help - Show this message
llfakeplayer create - Create new fake player
llfakeplayer remove - Remove a fake player forever
llfakeplayer list - List all fake player
llfakeplayer login - Login a existing fake player
llfakeplayer logout - Logout a existing fake player
llfakeplayer import - Import data from ddf8196's FakePlayer Client
)"


// Path
#define PLUGIN_DIR PLUGINS_DIR PLUGIN_NAME "/"
#define PLUGIN_LOG_PATH LOG_DIR PLUGIN_NAME ".log"
#define PLUGIN_CONFIG_PATH PLUGIN_DIR "config.json"
#define DEFAULT_DATABASE_PATH PLUGIN_DIR "leveldb/"
//"FakePlayerDB/"
// Switch
#define ENABLE_LOG_FILE true
#define ENABLE_CONFIG true

#define VERBOSE

#if !defined(DEBUG) && defined(VERBOSE)
#undef VERBOSE
#endif // !DEBUG


namespace Config
{
inline std::string CommandAlias = "fp";
inline unsigned int ChunkRadius = 9;
inline bool DebugMode = false;
inline std::string DataBasePath = DEFAULT_DATABASE_PATH;

bool saveConfig();
bool initConfig();
} // namespace Config
