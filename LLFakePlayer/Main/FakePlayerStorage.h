#pragma once
#include <KVDBAPI.h>
#include "FakePlayerAPI.h"
#include <MC/PlayerDataSystem.hpp>
#include <Utils/DbgHelper.h>

class FakePlayerManager;

class FakePlayerStorage
{
protected:
    std::unique_ptr<KVDB> mStorage;
    Logger mLogger;
    CsLock mLock;
    FakePlayerManager* mManager;

public:
    inline static std::string playerKey(mce::UUID uuid)
    {
        return "player_" + uuid.asString();
    }
    inline static std::string serverKey(mce::UUID uuid)
    {
        return "player_server_" + uuid.asString();
    }

    FakePlayerStorage(std::string const& storagePath, FakePlayerManager* manager);

    bool saveData(std::string const& key, std::string const& data);
    bool loadData(std::string const& key, std::string& result);
    bool hasKey(std::string const& key);

public:
    std::unordered_map<mce::UUID, std::shared_ptr<FakePlayer>> loadPlayerList();
    bool removePlayer(FakePlayer const& fakePlayer);
    bool savePlayer(FakePlayer const& fakePlayer);
    bool savePlayerData(FakePlayer const& fakePlayer);
    bool savePlayerWithTag(FakePlayer const& fakePlayer, CompoundTag const& tag);
    bool savePlayerInfo(FakePlayer const& fakePlayer);

    std::string getPlayerData(mce::UUID uuid);

    std::unique_ptr<CompoundTag> getPlayerTag(mce::UUID uuid);
    std::unique_ptr<CompoundTag> getPlayerTag(FakePlayer const& fakePlayer);
    std::shared_ptr<FakePlayer> getFakePlayer(mce::UUID uuid);
};