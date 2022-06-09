#include "pch.h"
#include "FakePlayerCommand.h"
#include "FakePlayerManager.h"
#include <MC/ServerNetworkHandler.hpp>
#include <MC/SimulatedPlayer.hpp>
#include <PlayerInfoAPI.h>
#include "CommandHelper.h"


std::vector<std::string> FakePlayerCommand::mList;
constexpr auto FULL_COMMAND_NAME = "llfakeplayer";

void FakePlayerCommand::execute(CommandOrigin const& origin, CommandOutput& output) const
{
    auto& manager = FakePlayerManager::getManager();
    bool res = false;
    switch (operation)
    {
        case FakePlayerCommand::Operation::Help:
            output.success(PLUGIN_USAGE);
            break;
        case FakePlayerCommand::Operation::List:
        {
            std::string playerList = "";
            bool first = true;
            for (auto& fp : manager.getFakePlayerList())
            {
                if (!first)
                    playerList += ", ";
                first = false;
                playerList += fp->isOnline() ? ColorHelper::green(fp->getRealName()) : fp->getRealName();
                output.success();
            }
            if (playerList.empty())
                output.error(ColorHelper::red("No fake player."));
            else
                output.addMessage(fmt::format("List: {}", playerList));
            break;
        }
        case FakePlayerCommand::Operation::Create:
        {
            if (auto fp = manager.create(name))
            {
                output.success(fmt::format("Create fake player {} successfully.", fp->getRealName()));
            }
            else
            {
                output.error(fmt::format("Failed to create fake player {}.", name));
            }
            break;
        }
        case FakePlayerCommand::Operation::Remove:
        {
            res = manager.remove(name);
            if (res)
                output.success("Remove Success");
            else
                output.error("Remove Failed");
            break;
        }
        case FakePlayerCommand::Operation::Login:
        {
            auto fakePlayer = manager.tryGetFakePlayer(name);
            if (!fakePlayer)
                output.errorf("Fake Player {} not found", name);
            else if (fakePlayer->isOnline())
                output.errorf("Fake Player {} already online", name);
            else if (fakePlayer->login())
                output.successf("Fake Player {} login success", name);
            else
                output.errorf("Unknown Error in Fake Player {} login", name);
            break;
        }
        case FakePlayerCommand::Operation::Logout:
            res = manager.logout(name);
            if (res)
                output.success("Logout Success");
            else
                output.error("Logout Failed");
            break;
#ifdef COMMAND_AS_SUB_COMMAND
        case FakePlayerCommand::Operation::As:
        {
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
            auto port = std::stoi(addr.substr(sep+1));
            pl->transferServer(ip, port);
            
            if (!fp)
                output.errorf("Fake Player {} not found", name);
            else if (!pl)
                output.errorf("\"As\" command can only be used by player");
            //else if (manager.swapData(*fp, *Command::getPlayerFromOrigin(origin)))
            //    output.successf("Login As Query Success ");
            //else
                output.errorf("Unknown Error in Fake Player {} swap data", name);
            break;
        }
#endif // COMMAND_AS_SUB_COMMAND
        case FakePlayerCommand::Operation::Import:
            if (name_isSet)
            {
                manager.importClientFakePlayerData(name);
            }
            else
            {
                PlayerInfo::forEachInfo([&output](std::string_view name, std::string_view xuid, std::string_view uuid) -> bool {
                    auto nameUUID = JAVA::nameUUIDFromBytes(std::string(name));
                    if (uuid == nameUUID.asString())
                    {
                        output.success(fmt::format("{} - {} - {}", name, xuid, uuid));
                    }
                    return true;
                });
            }
            break;
        default:
            output.error("NoImpl");
            break;
    }
}

void FakePlayerCommand::setup(CommandRegistry& registry)
{

    registry.registerCommand(FULL_COMMAND_NAME, "FakePlayer For LiteLoader", CommandPermissionLevel::GameMasters, {(CommandFlagValue)0}, {(CommandFlagValue)0x80});
    if (!Config::CommandAlias.empty())
        registry.registerAlias(FULL_COMMAND_NAME, Config::CommandAlias);
    registry.addEnum<Operation>("FpCreateAction",
                                {
                                    {"create", Operation::Create},
                                });
    registry.addEnum<Operation>("FpOtherAction",
                                {
                                    {"remove", Operation::Remove},
                                    {"login", Operation::Login},
                                    {"logout", Operation::Logout},
#ifdef COMMAND_AS_SUB_COMMAND
                                    {"as", Operation::As},
#endif // COMMAND_AS_SUB_COMMAND
                                });
    registry.addEnum<Operation>("FpManageAction",
                                {
                                    {"list", Operation::List},
                                    {"help", Operation::Help},
                                });
    registry.addEnum<Operation>("FpImportAction",
                                {
                                    {"import", Operation::Import},
                                });
    auto actionCreate = makeMandatory<CommandParameterDataType::ENUM>(&FakePlayerCommand::operation, "action", "FpCreateAction");
    auto actionWithName = makeMandatory<CommandParameterDataType::ENUM>(&FakePlayerCommand::operation, "action", "FpOtherAction");
    auto action = makeMandatory<CommandParameterDataType::ENUM>(&FakePlayerCommand::operation, "action", "FpManageAction");
    auto actionImport = makeMandatory<CommandParameterDataType::ENUM>(&FakePlayerCommand::operation, "action", "FpImportAction");
    actionCreate.addOptions(CommandParameterOption::EnumAutocompleteExpansion);
    actionWithName.addOptions(CommandParameterOption::EnumAutocompleteExpansion);
    action.addOptions(CommandParameterOption::EnumAutocompleteExpansion);

    auto nameSoftEnum = registry.addSoftEnum(FAKEPLAYER_LIST_SOFT_ENUM_NAME, FakePlayerManager::getManager().getSortedNames());
    auto nameSoftEnumParam = makeMandatory<CommandParameterDataType::SOFT_ENUM>(&FakePlayerCommand::name, "name", FAKEPLAYER_LIST_SOFT_ENUM_NAME, &FakePlayerCommand::name_isSet);
    auto nameParam = makeMandatory(&FakePlayerCommand::name, "name", &FakePlayerCommand::name_isSet);
    auto nameOptional = makeOptional(&FakePlayerCommand::name, "name", &FakePlayerCommand::name_isSet);
    auto posParam = makeOptional(&FakePlayerCommand::commandPos, "position", &FakePlayerCommand::commandPos_isSet);
    auto dimidParam = makeOptional(&FakePlayerCommand::dimensionId, "dimension", &FakePlayerCommand::dimensionId_isSet);


    registry.registerOverload<FakePlayerCommand>(FULL_COMMAND_NAME, action);
    registry.registerOverload<FakePlayerCommand>(FULL_COMMAND_NAME, actionWithName, nameSoftEnumParam);
    registry.registerOverload<FakePlayerCommand>(FULL_COMMAND_NAME, actionCreate, nameParam);
    registry.registerOverload<FakePlayerCommand>(FULL_COMMAND_NAME, actionImport, nameOptional);
}

void UpdateLLFakePlayerSoftEnum()
{
    if (Global<CommandRegistry>)
        Global<CommandRegistry>->setSoftEnumValues(FAKEPLAYER_LIST_SOFT_ENUM_NAME, FakePlayerManager::getManager().getSortedNames());
}

