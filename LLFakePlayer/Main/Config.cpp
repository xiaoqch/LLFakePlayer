#include "pch.h"
#include "Config.h"
#include <third-party/Nlohmann/json.hpp>
#include <filesystem>

#pragma region Config

// Write config value to json
#define SerializeVaule(var) json[#var] = Config::var
// Write config enum value to json
#define SerializeEnumVaule(var) json[#var] = magic_enum::enum_name(Config::var)

// Read config value from json
#define DeserializeVaule(var)                                                         \
    if (json.find(#var) != json.end())                                                \
    {                                                                                 \
        Config::var = json.value(#var, Config::var);                                  \
    }                                                                                 \
    else                                                                              \
    {                                                                                 \
        logger.info("Missing config {}, use default value ", #var /*, Config::var*/); \
        needUpdate = true;                                                            \
    }

// Read config enum value from json
#define DeserializeEnumVaule(var)                                            \
    if (json.find(#var) != json.end())                                       \
    {                                                                        \
        auto svar = magic_enum::enum_name(Config::var);                      \
        svar = json.value(#var, svar);                                       \
        auto enumValue = magic_enum::enum_cast<decltype(Config::var)>(svar); \
        if (enumValue.has_value())                                           \
            Config::var = enumValue.value();                                 \
        else                                                                 \
        {                                                                    \
            logger.warn("Unsupported config value {}, use default value {}", \
                        svar, magic_enum::enum_name(Config::var));           \
            needUpdate = true;                                               \
        }                                                                    \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        logger.warn("Missing config {}, use default value {}",               \
                    #var, magic_enum::enum_name(Config::var));               \
        needUpdate = true;                                                   \
    }

namespace Config
{

std::string serialize()
{
    nlohmann::json json;

    SerializeVaule(CommandAlias);
    SerializeVaule(ChunkRadius);
    SerializeVaule(DebugMode);

    return json.dump(4);
}

bool deserialize(std::string const& jsonStr)
{
    auto json = nlohmann::json::parse(jsonStr, nullptr, false, true);
    bool needUpdate = false;

    DeserializeVaule(CommandAlias);
    DeserializeVaule(ChunkRadius);
    DeserializeVaule(DebugMode);

    return !needUpdate;
}

bool saveConfig()
{
    return WriteAllFile(PLUGIN_CONFIG_PATH, serialize(), false);
}

bool initConfig()
{
    bool needToSave = false;
    auto jsonStr = ReadAllFile(PLUGIN_CONFIG_PATH);
    if (jsonStr.has_value())
    {
        try
        {
            needToSave = !deserialize(jsonStr.value());
        }
        catch (const std::exception&)
        {
            logger.error("Error in loading config file \"{}\", Use default config", PLUGIN_CONFIG_PATH);
            needToSave = true;
        }
    }
    else
    {
        logger.info("Config File \"{}\" Not Found, Use Default Config", PLUGIN_CONFIG_PATH);
        std::filesystem::create_directories(std::filesystem::path(PLUGIN_CONFIG_PATH).remove_filename());
        needToSave = true;
    }
    if (needToSave)
        needToSave = saveConfig();
    return !needToSave;
}

} // namespace Config

#pragma endregion

#if PLUGIN_VERSION_IS_BETA
void logBetaInfo()
{
    logger.warn("beta version, log info:");
    LOG_VAR(PLUGIN_NAME);
    if (ENABLE_LOG_FILE)
        LOG_VAR(PLUGIN_LOG_PATH);
    if (ENABLE_CONFIG)
        LOG_VAR(PLUGIN_CONFIG_PATH);
}
#endif // PLUGIN_VERSION_IS_BETA