// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "Config.h"
#include <ServerAPI.h>

Logger logger(PLUGIN_NAME);

void entry();

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            LL::registerPlugin(
                PLUGIN_NAME,
                PLUGIN_INTRODUCTION,
                LL::Version(PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_REVISION, PLUGIN_LLVERSION_STATUS),
                std::map<std::string, std::string>{
#ifdef PLUGIN_AUTHOR
                    {"Author", PLUGIN_AUTHOR},
#endif // PLUGIN_AUTHOR
                    {"Git", PLUGIN_WEBSIDE},
                    {"License", PLUGIN_LICENCE},
                    {"Website", PLUGIN_WEBSIDE},
                    {"Target BDS Version: ", TO_VERSION_STRING(TARGET_BDS_VERSION)},
                });
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

extern "C" {
_declspec(dllexport) void onPostInit()
{
    std::ios::sync_with_stdio(false);
#ifdef PLUGIN_IS_BETA
    logger.warn("This plugin is a beta version and may have bugs");
#else
    // Set global SEH-Exception handler
    _set_se_translator(seh_exception::TranslateSEHtoCE);

#endif // DEBUG
#ifdef TARGET_BDS_PROTOCOL_VERSION
    auto currentProtocol = LL::getServerProtocolVersion();
    if (currentProtocol != TARGET_BDS_PROTOCOL_VERSION)
    {
        logger.error("Protocol version not match, target version: {}, current version: {}.",
                               TARGET_BDS_PROTOCOL_VERSION, currentProtocol);
        logger.error("please use the " PLUGIN_NAME " that matches the BDS version");
    }
#endif // TARGET_BDS_PROTOCOL_VERSION
    if constexpr (ENABLE_CONFIG)
        Config::initConfig();
    entry();
    logger.info("{} Loaded, Version: {}, Author: {}", PLUGIN_NAME, PLUGIN_FILE_VERSION_STRING, PLUGIN_AUTHOR);
    if constexpr (PLUGIN_USAGE)
        logger.info("Usage: \n{}", PLUGIN_USAGE);
    if constexpr (ENABLE_LOG_FILE)
        Logger::setDefaultFile(PLUGIN_LOG_PATH, std::ios::app);
}
}
