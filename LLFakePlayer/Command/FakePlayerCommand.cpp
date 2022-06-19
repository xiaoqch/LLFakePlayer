#include "pch.h"
#include "FakePlayerCommand.h"
#include "FakePlayerManager.h"
#include <MC/ServerNetworkHandler.hpp>
#include <MC/SimulatedPlayer.hpp>
#include <PlayerInfoAPI.h>
#include "CommandHelper.h"


constexpr auto FULL_COMMAND_NAME = "llfakeplayer";

bool hasPlayerOnline(std::string const& name)
{
    bool exists = false;
    Global<Level>->forEachPlayer([&](Player& player) {
        if (player.getName() != name)
            return true;
        exists = true;
        return false;
    });
    return exists;
}

template <>
void FakePlayerCommand::executeAction<FakePlayerCommand::Action::Help>(CommandOrigin const& origin, CommandOutput& output) const
{
    output.success(PLUGIN_USAGE);
}

template <>
void FakePlayerCommand::executeAction<FakePlayerCommand::Action::List>(CommandOrigin const& origin, CommandOutput& output) const
{
    auto& manager = FakePlayerManager::getManager();
    auto list = manager.getFakePlayerList();
    if (list.empty())
    {
        return output.error("No fake players.");
    }
    auto result = std::accumulate(list.begin(), list.end(), std::string(), [](std::string const& str, FakePlayer* fp) {
        auto name = fp->isAutoLogin() ? fp->getRealName() + "<auto>" : fp->getRealName();
        if (fp->online)
            name = ColorHelper::green(name);
        if (str.empty())
            return str + name;
        else
            return str + ", " + name;
    });
    output.success(result);
}

template <>
void FakePlayerCommand::executeAction<FakePlayerCommand::Action::Add>(CommandOrigin const& origin, CommandOutput& output) const
{
    auto& manager = FakePlayerManager::getManager();
    bool exists = hasPlayerOnline(mName);
    if (exists)
        return output.error("Player already exists and is online.");
    exists = manager.tryGetFakePlayer(mName).get();
    if (exists)
        return output.error("Fake Player already exists.");
    auto result = manager.create(mName);
    if (!result)
        return output.error("Failed to create fake player.");
    output.success("Fake player created.");
}

template <>
void FakePlayerCommand::executeAction<FakePlayerCommand::Action::Remove>(CommandOrigin const& origin, CommandOutput& output) const
{
    auto& manager = FakePlayerManager::getManager();
    auto exists = manager.tryGetFakePlayer(mName);
    if (!exists)
        return output.error("Fake player does not exist.");
    auto result = manager.remove(mName);
    if (!result)
        return output.error("Failed to remove fake player.");
    output.success("Fake player removed.");
}

template <>
void FakePlayerCommand::executeAction<FakePlayerCommand::Action::Login>(CommandOrigin const& origin, CommandOutput& output) const
{
    auto& manager = FakePlayerManager::getManager();
    auto fp = manager.tryGetFakePlayer(mName);
    if (!fp)
        return output.error("Fake player does not exist.");
    if (fp->online)
        return output.error("Fake player is already online.");

    if (hasPlayerOnline(fp->getRealName()))
        return output.error("Real player is already online.");
    if (!fp->login())
        return output.error("Failed to login fake player.");
    output.success("Fake player logged in.");
}

template <>
void FakePlayerCommand::executeAction<FakePlayerCommand::Action::Logout>(CommandOrigin const& origin, CommandOutput& output) const
{
    auto& manager = FakePlayerManager::getManager();
    auto fp = manager.tryGetFakePlayer(mName);
    if (!fp)
        return output.error("Fake player does not exist.");
    if (!fp->online)
        return output.error("Fake player is not online.");
    if (!fp->logout())
        return output.error("Failed to logout fake player.");
    output.success("Fake player logged out.");
}

template <>
void FakePlayerCommand::executeAction<FakePlayerCommand::Action::AutoLogin>(CommandOrigin const& origin, CommandOutput& output) const
{
    auto& manager = FakePlayerManager::getManager();
    if (mName_isSet)
    {

        auto fp = manager.tryGetFakePlayer(mName);
        if (!fp)
            return output.error("Fake player does not exists.");
        if (mAutoLogin_isSet) {
            fp->setAutoLogin(mAutoLogin);
        }
        else
        {
            auto autoLogin = fp->isAutoLogin();
            fp->setAutoLogin(!autoLogin);
        }
        output.successf("Fake player {} now {} auto login.", fp->getRealName(), fp->isAutoLogin() ? "will" : "will not");
    }
    else
    {
        auto list = manager.getFakePlayerList();
        std::vector<std::string> names;
        for (auto& fp : list)
        {
            if (fp->isAutoLogin())
                names.push_back(fp->getRealName());
        }
        if (names.empty())
            return output.error("No fake players are auto logged in.");
        auto result = std::accumulate(names.begin(), names.end(), std::string(), [](std::string const& str, std::string const& name) {
            if (str.empty())
                return str + name;
            else
                return str + ", " + name;
        });
        output.successf("There are {} fake players auto logged in:\n{}", names.size(), result);
    }
}

template <>
void FakePlayerCommand::executeAction<FakePlayerCommand::Action::Import>(CommandOrigin const& origin, CommandOutput& output) const
{
    auto& manager = FakePlayerManager::getManager();
    if (mName_isSet)
    {

        auto fp = manager.tryGetFakePlayer(mName);
        if (fp)
            return output.error("Fake player already exists.");
        auto result = manager.importClientFakePlayerData(mName);
        if (!result.empty())
            return output.error(result);
        output.success("Fake player imported.");
    }
    else
    {
        output.success("Detected fake players:");
        PlayerInfo::forEachInfo([&output](std::string_view name, std::string_view xuid, std::string_view uuid) -> bool {
            auto nameUUID = JAVA::nameUUIDFromBytes(std::string(name));
            if (uuid == nameUUID.asString())
            {
                output.success(fmt::format("{} - {} - {}", name, xuid, uuid));
            }
            return true;
        });
    }
}

#ifdef COMMAND_AS_ACTION

#include <MC/ConnectionRequest.hpp>
template <>
void FakePlayerCommand::executeAction<FakePlayerCommand::Action::As>(CommandOrigin const& origin, CommandOutput& output) const
{
    auto& manager = FakePlayerManager::getManager();
    auto fp = manager.tryGetFakePlayer(name);
    auto pl = origin.getPlayer();

    auto& map = Global<ServerNetworkHandler>->fetchConnectionRequest(*pl->getNetworkIdentifier()).rawToken.get()->dataInfo.value_.map_;
    for (auto iter = map->begin(); iter != map->end(); ++iter)
    {
        string s(iter->first.c_str());
        if (s.find("ServerAddress") != s.npos)
        {
            auto ServerAddress = iter->second.value_.string_;
        }
    }
    auto addr = pl->getServerAddress();
    size_t sep = addr.find_last_of(':');
    auto ip = addr.substr(0, sep);
    auto port = std::stoi(addr.substr(sep + 1));
    pl->transferServer(ip, port);

    if (!fp)
        output.errorf("Fake Player {} not found", name);
    else if (!pl)
        output.errorf("\"As\" command can only be used by player");
    // else if (manager.swapData(*fp, *Command::getPlayerFromOrigin(origin)))
    //     output.successf("Login As Query Success ");
    // else
    output.errorf("Unknown Error in Fake Player {} swap data", name);
}

#endif // COMMAND_AS_ACTION

void FakePlayerCommand::execute(CommandOrigin const& origin, CommandOutput& output) const
{
    switch (mAction)
    {
        case FakePlayerCommand::Action::Help:
            executeAction<FakePlayerCommand::Action::Help>(origin, output);
            break;
        case FakePlayerCommand::Action::List:
            executeAction<FakePlayerCommand::Action::List>(origin, output);
            break;
        case FakePlayerCommand::Action::Add:
            executeAction<FakePlayerCommand::Action::Add>(origin, output);
            break;
        case FakePlayerCommand::Action::Remove:
            executeAction<FakePlayerCommand::Action::Remove>(origin, output);
            break;
        case FakePlayerCommand::Action::Login:
            executeAction<FakePlayerCommand::Action::Login>(origin, output);
            break;
        case FakePlayerCommand::Action::Logout:
            executeAction<FakePlayerCommand::Action::Logout>(origin, output);
            break;
#ifdef COMMAND_AS_ACTION
        case FakePlayerCommand::Action::As:
            executeAction<FakePlayerCommand::Action::As>(origin, output);
            break;
#endif // COMMAND_AS_ACTION
        case FakePlayerCommand::Action::Import:
            executeAction<FakePlayerCommand::Action::Import>(origin, output);
            break;
        case FakePlayerCommand::Action::AutoLogin:
            executeAction<FakePlayerCommand::Action::AutoLogin>(origin, output);
            break;
        default:
            output.errorf("No Implementation for action {}", static_cast<int>(mAction));
            break;
    }
}

template <FakePlayerCommand::Action... ops>
CommandParameterData FakePlayerCommand::_registerAction(CommandRegistry& registry, char const* name)
{
    registry.addEnum<Action>(name, {{magic_enum::lower_enum_name<ops>().data(), ops}...});
    auto action = makeMandatory<CommandParameterDataType::ENUM>(&FakePlayerCommand::mAction, "action", name);
    action.addOptions(CommandParameterOption::EnumAutocompleteExpansion);
    return action;
}


void FakePlayerCommand::setup(CommandRegistry& registry)
{
    registry.registerCommand(FULL_COMMAND_NAME, "FakePlayer For LiteLoader", CommandPermissionLevel::GameMasters, {(CommandFlagValue)0}, {(CommandFlagValue)0x80});
    if (!Config::CommandAlias.empty())
        registry.registerAlias(FULL_COMMAND_NAME, Config::CommandAlias);
    
    auto actionAdd = _registerAction<Action::Add>(registry, "FpAddAction");
    auto actionOther = _registerAction<Action::Remove, Action::Login, Action::Logout>(registry, "FpOtherAction");
    auto actionManage = _registerAction<Action::List, Action::Help>(registry, "FpManageAction");
    auto actionImport = _registerAction<Action::Import>(registry, "FpImportAction");
    auto actionAutoLogin = _registerAction<Action::AutoLogin>(registry, "FpAutoLoginAction");


    auto nameSoftEnum = registry.addSoftEnum(FAKEPLAYER_LIST_SOFT_ENUM_NAME, FakePlayerManager::getManager().getSortedNames());
    auto nameSoftEnumParam = makeMandatory<CommandParameterDataType::SOFT_ENUM>(&FakePlayerCommand::mName, "name", FAKEPLAYER_LIST_SOFT_ENUM_NAME, &FakePlayerCommand::mName_isSet);
    auto nameParam = makeMandatory(&FakePlayerCommand::mName, "name", &FakePlayerCommand::mName_isSet);
    auto nameOptional = makeOptional(&FakePlayerCommand::mName, "name", &FakePlayerCommand::mName_isSet);
    auto autoLoginOpt = makeOptional(&FakePlayerCommand::mAutoLogin, "value", &FakePlayerCommand::mAutoLogin_isSet);
    //auto posParam = makeOptional(&FakePlayerCommand::mPosition, "position", &FakePlayerCommand::mPosition_isSet);
    //auto dimidParam = makeOptional(&FakePlayerCommand::mDimensionId, "dimension", &FakePlayerCommand::mDimensionId_isSet);

    registry.registerOverload<FakePlayerCommand>(FULL_COMMAND_NAME, actionManage);
    registry.registerOverload<FakePlayerCommand>(FULL_COMMAND_NAME, actionOther, nameSoftEnumParam);
    registry.registerOverload<FakePlayerCommand>(FULL_COMMAND_NAME, actionAdd, nameParam);
    registry.registerOverload<FakePlayerCommand>(FULL_COMMAND_NAME, actionImport, nameOptional);
    registry.registerOverload<FakePlayerCommand>(FULL_COMMAND_NAME, actionAutoLogin, nameOptional, autoLoginOpt);
}

void UpdateLLFakePlayerSoftEnum()
{
    if (Global<CommandRegistry>)
        Global<CommandRegistry>->setSoftEnumValues(FAKEPLAYER_LIST_SOFT_ENUM_NAME, FakePlayerManager::getManager().getSortedNames());
}
