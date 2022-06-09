#include "pch.h"
#include "FakePlayerStorage.h"
#include "Header/FakePlayerManager.h"
#include "FakePlayer.h"

FakePlayerStorage::FakePlayerStorage(std::string const& storagePath, FakePlayerManager* manager)
    : mManager(manager)
    , mLogger("FakePlayerStorage")
    , mLock()
    , mStorage(KVDB::create(storagePath))
{
    if (!mManager)
        mManager = &FakePlayerManager::getManager();
#ifdef DEBUG
    CsLockHolder lock(mLock);
    mStorage->iter([this](std::string_view key, std::string_view val) -> bool {
        auto tag = CompoundTag::fromBinaryNBT((void*)val.data(), val.size());
        DEBUGW("key: {}, size: {}, data: {}",
               ColorHelper::toConsole(ColorHelper::green(key)),
               ColorHelper::toConsole(ColorHelper::green(val.size())),
               ColorHelper::toConsole(ColorHelper::green(tag->toString())));
#ifndef VERBOSE
        if (val.size() < 1024)
#endif // VERBOSE
            debugLogNbt(*tag);
        return true;
    });
#endif // DEBUG
}

bool FakePlayerStorage::saveData(std::string const& key, std::string const& data)
{
    CsLockHolder lock(mLock);
    if (mStorage->set(key, data))
        return true;
    mLogger.error("Fail to save data: {}", key);
#ifdef VERBOSE
    PrintCurrentStackTraceback(&mLogger);
    DEBUGBREAK();
#endif // VERBOSE
    return false;
}

bool FakePlayerStorage::loadData(std::string const& key, std::string& result)
{
    CsLockHolder lock(mLock);
    auto data = mStorage->get(key);
    if (data.has_value())
    {
        result = data.value();
        return true;
    }
#ifdef VERBOSE
    DEBUGL("Key Not Found: {}", key);
    PrintCurrentStackTraceback(&mLogger);
    DEBUGBREAK();
#endif // VERBOSE
    return false;
}

bool FakePlayerStorage::hasKey(std::string const& key)
{
    CsLockHolder lock(mLock);
    return mStorage->get(key).has_value();
}

std::unordered_map<mce::UUID, std::shared_ptr<FakePlayer>> FakePlayerStorage::loadPlayerList()
{
    std::unordered_map<mce::UUID, std::shared_ptr<FakePlayer>> fakePlayers;
    CsLockHolder lock(mLock);
    mStorage->iter([&](std::string_view key, std::string_view val) -> bool {
        if (key.starts_with("player_") && !key.starts_with("player_server_"))
        {
            auto tag = CompoundTag::fromBinaryNBT((void*)val.data(), val.size());
            auto player = FakePlayer::deserialize(*tag, mManager);
            ASSERT("player_" + player->getUUIDString() == key);
            if (!player)
            {
                mLogger.warn("Error in loading FakePlayer UUID({})", key);
            }
            else
            {
                fakePlayers.emplace(player->mUUID, player);
            }
        }
        return true;
    });
    return fakePlayers;
}

bool FakePlayerStorage::removePlayer(FakePlayer const& fakePlayer)
{
    CsLockHolder lock(mLock);
    auto storageId = fakePlayer.getStorageId();
    auto serverId = fakePlayer.getServerId();
    auto res = mStorage->del(storageId);
    if (!res)
    {
        mLogger.error("Error in {} - {}", __FUNCTION__, storageId);
        DEBUGBREAK();
        return false;
    }
    res = mStorage->del(serverId);
    if (!res)
    {
        mLogger.error("Error in {} - {}", __FUNCTION__, serverId);
        DEBUGBREAK();
        return false;
    }
    return true;
}

bool FakePlayerStorage::savePlayer(FakePlayer const& fakePlayer)
{
    auto res = savePlayerInfo(fakePlayer);
    if (fakePlayer.isOnline())
        res &= savePlayerData(fakePlayer);
    return res;
}

bool FakePlayerStorage::savePlayerData(FakePlayer const& fakePlayer)
{
    auto serverId = fakePlayer.getServerId();
    auto tag = fakePlayer.getOnlinePlayerTag();
    auto data = tag->toBinaryNBT();
//#define TEST_SAVE_DATA
#ifdef TEST_SAVE_DATA
    debugLogNbt(*tag);
    if (mStorage->set(serverId, tag->toBinaryNBT(true)))
    {
        auto newData = mStorage->get(serverId);
        if (!newData)
            DEBUGBREAK();
        auto newTag = CompoundTag::fromBinaryNBT(*newData);
        debugLogNbt(*newTag);
        if (!tag->equals(*newTag))
            DEBUGBREAK();
        return true;
    }
    DEBUGBREAK();
#endif // TEST_SAVE_DATA
    if (tag && saveData(serverId, data))
        return true;
    mLogger.error("Error in {} - {}", __FUNCTION__, serverId);
    DEBUGBREAK();
    return false;
}

bool FakePlayerStorage::savePlayerWithTag(FakePlayer const& fakePlayer, CompoundTag const& tag)
{
    auto serverId = fakePlayer.getServerId();
    auto data = const_cast<CompoundTag&>(tag).toBinaryNBT();

    if (saveData(serverId, data))
        return true;
    mLogger.error("Error in {} - {}", __FUNCTION__, serverId);
    DEBUGBREAK();
    return false;
}

bool FakePlayerStorage::savePlayerInfo(FakePlayer const& fakePlayer)
{
    CsLockHolder lock(mLock);
    auto storageId = fakePlayer.getStorageId();
    auto info = fakePlayer.serialize()->toBinaryNBT(true);
    if (saveData(storageId, info))
        return true;
    mLogger.error("Error in {} - {}", __FUNCTION__, storageId);
    DEBUGBREAK();
    return false;
}

std::string FakePlayerStorage::getPlayerData(mce::UUID uuid)
{
    auto serverId = serverKey(uuid);
    if (!hasKey(serverId))
        return "";
    std::string data = "";
    if (loadData(serverId, data))
        return data;
#ifdef DEBUG
    mLogger.error("Error in {} - {}", __FUNCTION__, serverId);
    DEBUGBREAK();
#endif // DEBUG
    return "";
}

std::unique_ptr<CompoundTag> FakePlayerStorage::getPlayerTag(mce::UUID uuid)
{
    auto data = getPlayerData(uuid);
    if (!data.empty())
        return CompoundTag::fromBinaryNBT(data, true);
    return {};
}

std::unique_ptr<CompoundTag> FakePlayerStorage::getPlayerTag(FakePlayer const& fakePlayer)
{
    return getPlayerTag(fakePlayer.getUuid());
}

std::shared_ptr<FakePlayer> FakePlayerStorage::getFakePlayer(mce::UUID uuid)
{
    CsLockHolder lock(mLock);
    auto storageId = playerKey(uuid);
    std::string data = "";
    if (!loadData(storageId, data))
    {
        mLogger.error("Error in {} - {}", __FUNCTION__, storageId);
        DEBUGBREAK();
        return {};
    }
    auto info = CompoundTag::fromBinaryNBT(data, true);
    return FakePlayer::deserialize(*info);
}
