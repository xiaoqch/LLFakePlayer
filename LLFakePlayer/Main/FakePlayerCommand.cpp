#include "pch.h"
#include "FakePlayerCommand.h"
#include <MC/DBStorage.hpp>
#include <MC/ServerNetworkHandler.hpp>
#include <MC/SimulatedPlayer.hpp>
#include <MC/Dimension.hpp>
#include <MC/CompoundTag.hpp>
#include <PlayerInfoAPI.h>
#include "FakePlayerManager.h"
#include "Utils/ColorHelper.h"

#define KEY_NO_TARGET "%commands.generic.noTargetMatch"
#define KEY_TOO_MANY_TARGER "%commands.generic.tooManyTargets"

#define AssertUniqueTarger(results)         \
    if (results.empty())                    \
        return output.error(KEY_NO_TARGET); \
    else if (results.count() > 1)           \
        return output.error(KEY_TOO_MANY_TARGER);

#define successf(...) success(fmt::format(__VA_ARGS__))
#define errorf(...) error(ColorHelper::red(fmt::format(__VA_ARGS__)))

using namespace RegisterCommandHelper;
std::vector<std::string> FakePlayerCommand::mList;

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
                playerList += fp->online() ? ColorHelper::green(fp->getRealName()) : fp->getRealName();
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
            else if (fakePlayer->online())
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
        case FakePlayerCommand::Operation::Swap:
        {
            auto fp = manager.tryGetFakePlayer(name);
            if (!fp)
                output.errorf("Fake Player {} not found", name);
            else if (manager.swapData(*fp, *Command::getPlayerFromOrigin(origin)))
                output.successf("Fake Player {} swap data success", name);
            else
                output.errorf("Unknown Error in Fake Player {} swap data", name);
            break;
        }
        case FakePlayerCommand::Operation::Import:
            if (name_isSet)
            {
                manager.importData_DDF(name);
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
        case FakePlayerCommand::Operation::GUI:
            output.error("NoImpl");
            break;
        default:
            break;
    }
}
class TestCommand;
constexpr auto FULL_COMMAND_NAME = "llfakeplayer";
void FakePlayerCommand::setup(CommandRegistry& registry)
{

    registry.registerCommand(FULL_COMMAND_NAME, "FakePlayer For LiteLoader", CommandPermissionLevel::GameMasters, {(CommandFlagValue)0}, {(CommandFlagValue)0x80});
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
                                    {"swap", Operation::Swap},
                                });
    registry.addEnum<Operation>("FpManageAction",
                                {
                                    {"list", Operation::List},
                                    {"help", Operation::Help},
                                    {"gui", Operation::GUI},
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

    auto nameSoftEnum = registry.addSoftEnum("playerList", FakePlayerManager::getManager().getSortedNames());
    auto nameSoftEnumParam = makeMandatory<CommandParameterDataType::SOFT_ENUM>(&FakePlayerCommand::name, "name", "playerList", &FakePlayerCommand::name_isSet);
    auto nameParam = makeMandatory(&FakePlayerCommand::name, "name", &FakePlayerCommand::name_isSet);
    auto nameOptional = makeOptional(&FakePlayerCommand::name, "name", &FakePlayerCommand::name_isSet);
    auto posParam = makeOptional(&FakePlayerCommand::commandPos, "position", &FakePlayerCommand::commandPos_isSet);
    auto dimidParam = makeOptional(&FakePlayerCommand::dimensionId, "dimension", &FakePlayerCommand::dimensionId_isSet);

    // registry.addEnum<Operation>("HelpAction", { {"help", Operation::Help}, });
    // auto actionHelp = makeMandatory<CommandParameterDataType::ENUM>(&FakePlayerCommand::operation, "action", "HelpAction");
    // actionHelp.addOptions((CommandParameterOption)1);
    // registry.registerOverload<TestCommand>("Test", actionHelp);

    registry.registerOverload<FakePlayerCommand>(FULL_COMMAND_NAME, action);
    registry.registerOverload<FakePlayerCommand>(FULL_COMMAND_NAME, actionWithName, nameSoftEnumParam);
    registry.registerOverload<FakePlayerCommand>(FULL_COMMAND_NAME, actionCreate, nameParam);
    registry.registerOverload<FakePlayerCommand>(FULL_COMMAND_NAME, actionImport, nameOptional);
}

#ifdef PLUGIN_IS_BETA

// =============== Test ===============
#include <MC/BlockSource.hpp>
#include <MC/LevelChunk.hpp>

std::pair<std::string, int> genTickingInfo(BlockSource const& region, BlockPos const& bpos, int range)
{
    int chunk_x = bpos.x >> 4;
    int chunk_z = bpos.z >> 4;
    auto max_cx = chunk_x + range;
    auto min_cx = chunk_x - range;
    auto max_cz = chunk_z + range;
    auto min_cz = chunk_z - range;
    auto totalChunksCount = (max_cx - min_cx + 1) * (max_cz - min_cz + 1);
    auto chunksCount = 0;
    static std::string centerLabel = ColorHelper::red_bold("Ｘ");
    static std::string tickingLabel = ColorHelper::green("＃");
    static std::string inRegionLabel = "Ｏ";
    static std::string outofRegionLabel = "＋";
    static std::string unloadedLabel = ColorHelper::dark_gray("－");
    std::ostringstream loadInfo;
    std::set<__int64> playerChunkHashs;
    for (auto player : Level::getAllPlayers()) {
        if (player->getDimensionId() == region.getDimensionId()) {
            playerChunkHashs.insert(ChunkPos(player->getPos()).hash());
        }
    }
    auto& mainBlockSource = region.getDimensionConst().getBlockSourceFromMainChunkSource();
    for (auto cx = min_cx; cx <= max_cx; ++cx)
    {
        loadInfo << " \n";
        for (auto cz = min_cz; cz <= max_cz; ++cz)
        {
            auto chunkHash = ChunkPos(cx, cz).hash();
            std::string label;
            auto chunk = region.getChunk({cx, cz});
            if (cx == chunk_x && cz == chunk_z)
            {
                label=  centerLabel;
            }
            else
            {
                if (chunk)
                {
                    if (chunk->getLastTick().t == Global<Level>->getCurrentServerTick().t)
                    {
                        label = tickingLabel;
                    }
                    else
                    {
                        label = inRegionLabel;
                    }
                }
                else
                {
                    auto chunk = mainBlockSource.getChunk({cx, cz});
                    if (chunk)
                        label = outofRegionLabel;
                    else
                        label = unloadedLabel;
                }
            }
            if (chunk)
                ++chunksCount;
            if (playerChunkHashs.find(chunkHash)!= playerChunkHashs.end()) {
                ColorFormat::removeColorCode(label);
                label = ColorHelper::red_bold(label);
            }
            loadInfo << label;
        }
    }
    return {loadInfo.str(), chunksCount};
}

bool processCommand(class CommandOrigin const& origin, class CommandOutput& output, BlockSource const& region, BlockPos const& bpos, int range)
{
    auto loadInfo = genTickingInfo(region, bpos, range);
    output.success(fmt::format("Center ChunkPos: ({}, {}), Info: {}", (int)(bpos.x >> 4), (int)(bpos.z >> 4), loadInfo.first));
    return true;
}

bool processCommand(class CommandOrigin const& origin, class CommandOutput& output, Actor* actor, int range)
{
    auto bpos = actor->getBlockPos();
    auto& region = actor->getRegion();
    if (!&region)
        return false;
    auto loadInfo = genTickingInfo(region, bpos, range);
    output.success(fmt::format("Center ChunkPos: ({}, {}), Info: {}", (int)(bpos.x >> 4), (int)(bpos.z >> 4), loadInfo.first));
    return true;
}

void TickingCommand::execute(class CommandOrigin const& origin, class CommandOutput& output) const
{
    Actor* actor;
    if (selector_isSet)
    {
        auto result = selector.results(origin);
        if (result.empty())
        {
            output.error(KEY_NO_TARGET);
            return;
        }
        else if (result.count() > 1)
        {
            output.error(KEY_NO_TARGET);
            return;
        }
        actor = *result.begin();
    }
    else if (commandPos_isSet)
    {
        auto bpos = commandPos.getBlockPos(origin, Vec3::ZERO);
        auto dim = origin.getDimension();
        if (dimensionId_isSet) {
            dim = Level::getDimension(dimensionId);
        }
        if (!dim)
            return output.error("Error in get Dimension");

        if (!processCommand(origin, output, dim->getBlockSourceFromMainChunkSource(), bpos, range_isSet ? range : 10))
            return output.error("Error when executing command \"ticking\"");
        return;
    }
    else
    {
        if (auto entity = origin.getEntity())
        {
            actor = entity;
        }
        else
        {
            output.error(KEY_NO_TARGET);
            return;
        }
    }
    if (!processCommand(origin, output, actor, range_isSet ? range : 10))
        output.error("Error when executing command \"ticking\"");
}

void TickingCommand::setup(CommandRegistry& registry)
{
    registry.registerCommand("ticking", "Show Ticking chunks", CommandPermissionLevel::Any, {(CommandFlagValue)0}, {(CommandFlagValue)0x80});
    registry.registerOverload<TickingCommand>(
        "ticking",
        makeOptional(&TickingCommand::selector, "target", &TickingCommand::selector_isSet),
        makeOptional(&TickingCommand::range, "range", &TickingCommand::range_isSet));

    registry.registerOverload<TickingCommand>(
        "ticking",
        makeMandatory(&TickingCommand::range, "range", &TickingCommand::range_isSet));

    registry.registerOverload<TickingCommand>(
        "ticking",
        makeMandatory(&TickingCommand::commandPos, "coord", &TickingCommand::commandPos_isSet),
        makeOptional(&TickingCommand::dimensionId, "dimid", &TickingCommand::dimensionId_isSet),
        makeOptional(&TickingCommand::range, "range", &TickingCommand::range_isSet));
    
}

#endif // PLUGIN_IS_BETA
