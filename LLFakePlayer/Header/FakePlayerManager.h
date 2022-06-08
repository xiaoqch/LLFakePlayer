#pragma once
#include <MC/SimulatedPlayer.hpp>
#include <MC/CompoundTag.hpp>
#include <MC/Int64Tag.hpp>
#include <MC/StringTag.hpp>
#include <MC/ByteTag.hpp>
#include "Utils/ColorHelper.h"
#include <functional>
#include <KVDBAPI.h>
#include "SimulatedPlayerHelper.h"
#include "../FakePlayerAPI.h"

#define FPPUBLIC public

#ifndef FPAPI
#ifdef LLFAKEPLAYER_EXPORTS
#define FPAPI __declspec(dllexport)
#else
#define FPAPI __declspec(dllimport)
#endif
#endif // !FPAPI


namespace JAVA
{
mce::UUID nameUUIDFromBytes(std::string const& name);
}

inline mce::UUID seedFromString(std::string const& str)
{
    return mce::UUID::seedFromString(str);
}

class FakePlayerManager;
class FakePlayer;
class FakePlayerStorage;
#ifdef VERBOSE
void debugLogNbt(CompoundTag const& tag);
#else
#define debugLogNbt(...) (void)0
#endif // VERBOSE

template <>
struct std::hash<mce::UUID>
{
    constexpr size_t operator()(const mce::UUID& _Keyval) const noexcept
    {
        // std::_Hash_find_last_result<std::_List_node<std::pair<mce::UUID const ,void *>,void *> *> *__fastcall std::_Hash<std::_Umap_traits<mce::UUID,void *,std::_Uhash_compare<mce::UUID,std::hash<mce::UUID>,std::equal_to<mce::UUID>>,std::allocator<std::pair<mce::UUID const,void *>>,0>>::_Find_last<mce::UUID>
        // uuid_b = uuid[1];
        // uuid_a = *uuid;
        // _HashVal = *uuid ^ (0x1F1F1F1F * uuid_b);
        auto& uuid_b = *(size_t*)(((uintptr_t)&_Keyval) + 8);
        auto& uuid_a = *(size_t*)(((uintptr_t)&_Keyval) + 0);
        return uuid_a ^ (0x1F1F1F1F * uuid_b);
    }
};
template <>
struct std::equal_to<mce::UUID>
{
    constexpr bool operator()(const mce::UUID& _Left, const mce::UUID& _Right) const
    {
        auto& _Left_b = *(size_t*)(((uintptr_t)&_Left) + 8);
        auto& _Left_a = *(size_t*)(((uintptr_t)&_Left) + 0);
        auto& _Right_b = *(size_t*)(((uintptr_t)&_Right) + 8);
        auto& _Right_a = *(size_t*)(((uintptr_t)&_Right) + 0);
        return _Left_b == _Right_b && _Left_a == _Right_a;
    }
};

class FakePlayer
{
    // Offline Data
    std::string mRealName;
    std::string mXUID;
    mce::UUID mUUID;
    mutable time_t mLastUpdateTime;
    FakePlayerManager* mManager;
    bool mAutoLogin = false;
    bool mSaveData = true;

    // Online Data
    ActorUniqueID mUniqueID;
    unsigned char mClientSubID = -1;

    friend class FakePlayerManager;
    friend class FakePlayerStorage;

public:
    static bool mWaitingAs;
    static bool mLoggingIn;
    static FakePlayer* mLoggingInPlayer;
    static NetworkIdentifier mNetworkID;
    static unsigned char mMaxClientSubID;

    static unsigned char getNextClientSubID();

    FPAPI FakePlayer(std::string const& realName, std::string xuid, mce::UUID uuid, time_t lastUpdateTime = 0, bool autoLogin = false, FakePlayerManager* manager = nullptr);
    FPAPI ~FakePlayer();
    FPAPI static std::shared_ptr<FakePlayer> deserialize(CompoundTag const& tag, FakePlayerManager* manager = nullptr);
    FPAPI std::unique_ptr<CompoundTag> serialize() const;

    FPAPI bool login(BlockPos* bpos = nullptr, class AutomaticID<class Dimension, int> dimId = 0);
    FPAPI bool logout(bool save = true);
    FPAPI mce::UUID const& getUuid() const;
    FPAPI std::string getUUIDString() const;
    FPAPI std::string getServerId() const;
    FPAPI std::string getStorageId() const;
    inline unsigned char getClientSubId() const
    {
        return mClientSubID;
    }
    inline void setClientSubId(unsigned char subId)
    {
        mClientSubID = subId;
    }

    inline std::string const& getRealName() const
    {
        return mRealName;
    }
    inline std::string const& getXuid() const
    {
        return mXUID;
    }
    inline time_t getLastUpdateTime() const
    {
        return mLastUpdateTime;
    }
    inline bool isAutoLogin() const
    {
        return mAutoLogin;
    }
    inline bool isOnline() const
    {
        return getSimulatedPlayerByUuid(mUUID) != nullptr;
    }
    inline SimulatedPlayer* getPlayer() const
    {
        return getSimulatedPlayerByUuid(mUUID);
    }
    inline bool shouldSaveData() const
    {
        return mSaveData;
    }

    FPAPI std::unique_ptr<CompoundTag> getPlayerTag() const;
    FPAPI std::unique_ptr<CompoundTag> getStoragePlayerTag() const;
    FPAPI std::unique_ptr<CompoundTag> getOnlinePlayerTag() const;
};
class FakePlayerManager
{
public:
    std::unique_ptr<FakePlayerStorage> mStorage;

    std::unordered_map<mce::UUID, std::shared_ptr<FakePlayer>> mMap;
    std::unordered_map<std::string, std::shared_ptr<FakePlayer>> mMapByName;
    std::vector<std::string> mSortedNames;

    std::unordered_map<size_t, std::function<void(FakePlayerAPI::Event&)>> mEventHandlers;
    inline size_t subscribeEvent(std::function<void(FakePlayerAPI::Event&)> const& handler)
    {
        static size_t id = 0;
        mEventHandlers[id] = handler;
        return id++;
    }
    inline bool unsubscribeEvent(size_t id)
    {
        return mEventHandlers.erase(id) > 0;
    }
    inline void callEvent(FakePlayerAPI::Event::EventType type, FakePlayer& fp)
    {
        if (!mEventHandlers.empty())
        {
            FakePlayerAPI::Event ev{type, fp};
            for (auto& handler : mEventHandlers)
                handler.second(ev);
        }
    }
    inline void onAdd(FakePlayer& fp)
    {
        callEvent(FakePlayerAPI::Event::EventType::Add, fp);
    }
    inline void onRemove(FakePlayer& fp)
    {
        callEvent(FakePlayerAPI::Event::EventType::Remove, fp);
    }
    inline void onLogin(FakePlayer& fp)
    {
        callEvent(FakePlayerAPI::Event::EventType::Login, fp);
    }
    inline void onLogout(FakePlayer& fp)
    {
        callEvent(FakePlayerAPI::Event::EventType::Logout, fp);
    }
    inline void onChange(FakePlayer& fp)
    {
        callEvent(FakePlayerAPI::Event::EventType::Change, fp);
    }


private:
    FPAPI FakePlayerManager(std::string const& dbPath);
    FPAPI ~FakePlayerManager();
    FakePlayerManager() = delete;
    FakePlayerManager(FakePlayerManager const&) = delete;
    FakePlayerManager& operator=(FakePlayerManager const&) = delete;
    FakePlayerManager(FakePlayerManager&&) = delete;
    FakePlayerManager& operator=(FakePlayerManager&&) = delete;

    void initFakePlayers();
    void initEventListeners();

public:
    FPAPI bool savePlayers(bool onlineOnly = false);
    FPAPI bool saveData(mce::UUID uuid);
    FPAPI bool saveData(FakePlayer const& fakePlayer);
    FPAPI bool saveData(SimulatedPlayer const& simulatedPlayer);
    FPAPI bool importClientFakePlayerData(std::string const& name);
    friend class FakePlayer;
    friend class SimulatedPlayer;

public:
    inline std::vector<std::string> const& getSortedNames() const
    {
        return mSortedNames;
    }
    FPAPI static FakePlayerManager& getManager();
    inline void forEachFakePlayer(std::function<void(std::string_view name, FakePlayer& fakePlayer)> callback) const
    {
        for (auto& [name, fakePlayer] : mMapByName)
        {
            callback(name, *fakePlayer);
        }
    }
    inline std::vector<FakePlayer *> getFakePlayerList() const
    {
        std::vector<FakePlayer *> list;
        forEachFakePlayer([&list](std::string_view name, FakePlayer & fakePlayer) {
            list.push_back(&fakePlayer);
        });
        std::sort(list.begin(), list.end(), [](FakePlayer const* left, FakePlayer const* right) {
            return left->mLastUpdateTime > right->mLastUpdateTime;
        });
        return list;
    }
    FPAPI FakePlayer* create(std::string const& name, std::unique_ptr<CompoundTag> playerData = {});
    FPAPI bool remove(std::string const& name);
    FPAPI SimulatedPlayer* login(std::string const& name) const;
    FPAPI bool logout(FakePlayer& fakePlayer) const;
    // bool logout(std::string const& name) const;
    // bool logout(mce::UUID const& uuid) const;
    // bool logout(SimulatedPlayer const& simulatedPlayer) const;

    template <typename T>
    inline bool logout(T const& id) const
    {
        auto fakePlayer = tryGetFakePlayer(id);
        if (fakePlayer)
            return logout(*fakePlayer);
        return false;
    }

    inline std::shared_ptr<FakePlayer> tryGetFakePlayer(Player const& player) const
    {
        auto uuid = mce::UUID::fromString(const_cast<Player&>(player).getUuid());
        return tryGetFakePlayer(uuid);
    };
    inline std::shared_ptr<FakePlayer> tryGetFakePlayer(mce::UUID const& uuid) const
    {
        if (!uuid)
            return {};
        auto fpIter = mMap.find(uuid);
        if (fpIter == mMap.end())
            return {};
        return fpIter->second;
    };
    inline std::shared_ptr<FakePlayer> tryGetFakePlayer(std::string const& nameOrUuid) const
    {
        auto iter = mMapByName.find(nameOrUuid);
        if (iter != mMapByName.end())
            return iter->second;
        auto uuid = mce::UUID::fromString(nameOrUuid);
        return tryGetFakePlayer(uuid);
    };
};

#include <MC/PlayerDataSystem.hpp>
#include <Utils/DbgHelper.h>
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

    inline FakePlayerStorage(std::string const& storagePath, FakePlayerManager* manager)
        : mManager(manager)
        , mLogger("FakePlayerStorage")
        , mLock()
        , mStorage(KVDB::create(storagePath))
    {
        if (!mManager)
            mManager = &FakePlayerManager::getManager();
        CsLockHolder lock(mLock);
#ifdef DEBUG
        mStorage->iter([this](std::string_view key, std::string_view val) -> bool {
            DEBUGW("key: {}, size: {}, data: {}",
                   ColorFormat::convertToColsole(ColorHelper::green(key)),
                   ColorFormat::convertToColsole(ColorHelper::green(val.size())),
                   ColorFormat::convertToColsole(ColorHelper::green(CompoundTag::fromBinaryNBT((void*)val.data(), val.size())->toString())));
#ifndef VERBOSE
            if (val.size() < 1024)
#endif // VERBOSE
                debugLogNbt(*CompoundTag::fromBinaryNBT((void*)val.data(), val.size()));
            return true;
        });
#endif // DEBUG
    }

    inline bool saveData(std::string const& key, std::string const& data)
    {
        CsLockHolder lock(mLock);
        if (mStorage->set(key, data))
            return true;
#ifdef VERBOSE
        DEBUGL("Error to save Data: {}", key);
        PrintCurrentStackTraceback(&logger);
        DEBUGBREAK();
#endif // VERBOSE
        return false;
    }
    inline bool loadData(std::string const& key, std::string& result)
    {
        CsLockHolder lock(mLock);
        auto data = mStorage->get(key);
        if (data.has_value())
        {
            result = result.data();
            return true;
        }
#ifdef VERBOSE
        DEBUGL("Key Not Found: {}", key);
        PrintCurrentStackTraceback(&logger);
        DEBUGBREAK();
#endif // VERBOSE
        return false;
    }
    inline bool hasKey(std::string const& key)
    {
        CsLockHolder lock(mLock);
        return mStorage->get(key).has_value();
    }

protected:
    inline bool fixDatabase()
    {
        return false;
    };

public:
    inline std::unordered_map<mce::UUID, std::shared_ptr<FakePlayer>> loadPlayerList()
    {
        std::unordered_map<mce::UUID, std::shared_ptr<FakePlayer>> fakePlayers;
        mStorage->iter([&](std::string_view key, std::string_view val) -> bool {
            if (key._Starts_with("player_") && !key._Starts_with("player_server_"))
            {
                auto tag = CompoundTag::fromBinaryNBT((void*)val.data(), val.size());
                auto player = FakePlayer::deserialize(*tag.get(), mManager);
                ASSERT("player_" + player->getUUIDString() == key);
                if (!player)
                {
                    mLogger.warn("Error in loading FakePlayer UUID({})", key);
                }
                else
                {
                    fakePlayers.emplace(player->mUUID, std::move(player));
                }
            }
            return true;
        });
        return fakePlayers;
    }
    inline bool removePlayerData(FakePlayer const& fakePlayer)
    {
        CsLockHolder lock(mLock);
        auto storageId = fakePlayer.getStorageId();
        auto res = mStorage->del(storageId);
        if (!res)
        {
            mLogger.error("Error in {} - {}", __FUNCTION__, storageId);
            DEBUGBREAK();
            return false;
        }
        auto serverId = fakePlayer.getServerId();
        res = mStorage->del(serverId);
        if (!res)
        {
            mLogger.error("Error in {} - {}", __FUNCTION__, serverId);
            DEBUGBREAK();
            return false;
        }
        return true;
    };
    inline bool savePlayer(FakePlayer const& fakePlayer)
    {
        auto res = savePlayerInfo(fakePlayer);
        if (fakePlayer.isOnline())
            res &= savePlayerData(fakePlayer);
        return res;
    }
    inline bool savePlayerData(FakePlayer const& fakePlayer)
    {
        CsLockHolder lock(mLock);
        auto serverId = fakePlayer.getServerId();
        auto tag = fakePlayer.getOnlinePlayerTag();

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
        if (tag && mStorage->set(serverId, tag->toBinaryNBT(true)))
            return true;
        mLogger.error("Error in {} - {}", __FUNCTION__, serverId);
        DEBUGBREAK();
        return false;
    };
    inline bool savePlayerTag(FakePlayer const& fakePlayer, CompoundTag const& tag)
    {
        auto serverId = fakePlayer.getServerId();

        if (mStorage->set(serverId, const_cast<CompoundTag&>(tag).toBinaryNBT(true)))
            return true;
        mLogger.error("Error in {} - {}", __FUNCTION__, serverId);
        DEBUGBREAK();
        return false;
    };
    inline bool savePlayerInfo(FakePlayer const& fakePlayer)
    {
        CsLockHolder lock(mLock);
        auto storageId = fakePlayer.getStorageId();
        auto info = fakePlayer.serialize()->toBinaryNBT(true);
        if (mStorage->set(storageId, info))
            return true;
        mLogger.error("Error in {} - {}", __FUNCTION__, storageId);
        DEBUGBREAK();
        return false;
    };
    inline std::string getPlayerData(mce::UUID uuid)
    {
        CsLockHolder lock(mLock);
        auto serverId = serverKey(uuid);
        std::string data = "";
        if (mStorage->get(serverId, data))
            return data;
#ifdef DEBUG
        //mLogger.error("Error in {} - {}", __FUNCTION__, serverId);
        //DEBUGBREAK();
#endif // DEBUG
        return "";
    };
    inline std::string getPlayerData(FakePlayer const& fakePlayer)
    {
        return getPlayerData(fakePlayer.getUuid());
    }
    inline std::unique_ptr<CompoundTag> getPlayerTag(mce::UUID uuid)
    {
        auto data = getPlayerData(uuid);
        if (!data.empty())
            return CompoundTag::fromBinaryNBT((void*)data.c_str(), data.size(), true);
        return {};
    };
    inline std::unique_ptr<CompoundTag> getPlayerTag(FakePlayer const& fakePlayer)
    {
        return getPlayerTag(fakePlayer.getUuid());
    }
    inline std::shared_ptr<FakePlayer> getFakePlayer(mce::UUID uuid)
    {
        CsLockHolder lock(mLock);
        auto storageId = playerKey(uuid);
        std::string data = "";
        auto res = mStorage->get(storageId, data);
        if (!res)
        {
            mLogger.error("Error in {} - {}", __FUNCTION__, storageId);
            DEBUGBREAK();
            return {};
        }
        auto info = CompoundTag::fromBinaryNBT(data, true);
        return FakePlayer::deserialize(*info);
    };
};