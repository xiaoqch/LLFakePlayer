#include "pch.h"
#include "TickingCommand.h"
#include "CommandHelper.h"
#include "Utils/ColorHelper.h"
#include <MC/Dimension.hpp>

#ifdef PLUGIN_IS_BETA

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
    auto labels = fmt::format("player: {}, ticking: {}, In Region: {}, Out Of Region: {}, Unloaded: {}",
                centerLabel, tickingLabel, inRegionLabel, outofRegionLabel, unloadedLabel);
    logger.info(ColorFormat::convertToColsole(labels));
    std::ostringstream loadInfo;
    std::set<__int64> playerChunkHashs;
    for (auto player : Level::getAllPlayers())
    {
        if (player->getDimensionId() == region.getDimensionId())
        {
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
                label = centerLabel;
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
            if (playerChunkHashs.find(chunkHash) != playerChunkHashs.end())
            {
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
    if (mTarget_isSet)
    {
        auto result = mTarget.results(origin);
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
    else if (mPosition_isSet)
    {
        auto bpos = mPosition.getBlockPos(origin, Vec3::ZERO);
        auto dim = origin.getDimension();
        if (mDimensionId_isSet)
        {
            dim = Global<Level>->getDimension(mDimensionId);
        }
        if (!dim)
            return output.error("Error in get Dimension");

        if (!processCommand(origin, output, dim->getBlockSourceFromMainChunkSource(), bpos, mRange_isSet ? mRange : 10))
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
    if (!processCommand(origin, output, actor, mRange_isSet ? mRange : 10))
        output.error("Error when executing command \"ticking\"");
}

void TickingCommand::setup(CommandRegistry& registry)
{
    registry.registerCommand("ticking", "Show Ticking chunks", CommandPermissionLevel::Any, {(CommandFlagValue)0}, {(CommandFlagValue)0x80});
    registry.registerOverload<TickingCommand>(
        "ticking",
        makeOptional(&TickingCommand::mTarget, "target", &TickingCommand::mTarget_isSet),
        makeOptional(&TickingCommand::mRange, "range", &TickingCommand::mRange_isSet));

    registry.registerOverload<TickingCommand>(
        "ticking",
        makeMandatory(&TickingCommand::mRange, "range", &TickingCommand::mRange_isSet));

    registry.registerOverload<TickingCommand>(
        "ticking",
        makeMandatory(&TickingCommand::mPosition, "coord", &TickingCommand::mPosition_isSet),
        makeOptional(&TickingCommand::mDimensionId, "dimid", &TickingCommand::mDimensionId_isSet),
        makeOptional(&TickingCommand::mRange, "range", &TickingCommand::mRange_isSet));
}

#endif // PLUGIN_IS_BETA