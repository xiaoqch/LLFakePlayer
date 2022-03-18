#pragma once
#include "Config.h"
#include <MC/SimulatedPlayer.hpp>
#include <MC/CompoundTag.hpp>
#include <MC/Int64Tag.hpp>
#include <MC/StringTag.hpp>
#include <MC/ByteTag.hpp>
#include "Utils/ColorHelper.h"
#include <functional>
#include <KVDBAPI.h>
#define FPPUBLIC public
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
void debugLogNbt(CompoundTag const& tag);

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
    mce::UUID mUUID;
    mutable time_t mLastOnlineTime;
    bool mAutoLogin = false;
    FakePlayerManager* mManager;

    // Online Data
    ActorUniqueID mUniqueID;
    bool mOnline = false;
    SimulatedPlayer* mPlayer = nullptr;
    unsigned char mClientSubID = -1;

    friend class FakePlayerManager;
    friend class FakePlayerStorage;

FPPUBLIC:
    
    static bool mLoggingIn;
    static FakePlayer* mLoggingInPlayer;
    static NetworkIdentifier mNetworkID;
    static unsigned char mMaxClientSubID;

    static unsigned char getNextClientSubID();

public:
    FakePlayer(std::string const& realName, mce::UUID uuid, time_t lastOnlineTime = 0, bool autoLogin = false, FakePlayerManager* manager = nullptr);
    ~FakePlayer();
    static std::shared_ptr<FakePlayer> deserialize(CompoundTag const& tag, FakePlayerManager* manager = nullptr);
    std::unique_ptr<CompoundTag> serialize() const;

    bool login();
    bool logout(bool save = true);
    mce::UUID const& getUUID() const;
    std::string getUUIDString() const;
    std::string getServerId() const;
    std::string getStorageId() const;
    inline unsigned char getClientSubId() const
    {
        return mClientSubID;
    }

    inline std::string const& getRealName() const
    {
        return mRealName;
    }
    inline bool online() const
    {
        return mOnline;
    }

    std::unique_ptr<CompoundTag> getPlayerTag() const;
    std::unique_ptr<CompoundTag> getStoragePlayerTag() const;
    std::unique_ptr<CompoundTag> getOnlinePlayerTag() const;
};

class FakePlayerManager
{
public:
    std::unique_ptr<FakePlayerStorage> mStorage;

    std::unordered_map<mce::UUID, std::shared_ptr<FakePlayer>> mMap;
    std::unordered_map<std::string, std::shared_ptr<FakePlayer>> mMapByName;
    std::vector<std::string> mSortedNames;

private:
    FakePlayerManager(std::string const& dbPath);
    ~FakePlayerManager();
    FakePlayerManager(const FakePlayerManager&) = delete;
    FakePlayerManager& operator=(const FakePlayerManager&) = delete;

public:
    void initFakePlayers();
    bool savePlayers(bool onlineOnly = false);
    bool saveData(mce::UUID uuid);
    bool saveData(FakePlayer const& fakePlayer);
    bool saveData(SimulatedPlayer const& simulatedPlayer);
    bool importData_DDF(std::string const& name);
    friend class FakePlayer;
    friend class SimulatedPlayer;

public:
    inline std::vector<std::string> const& getSortedNames() const
    {
        return mSortedNames;
    }
    static FakePlayerManager& getManager();
    inline void forEachFakePlayer(std::function<void(std::string_view name, FakePlayer const& fakePlayer)> callback) const
    {
        for (auto& [name, fakePlayer] : mMapByName)
        {
            callback(name, *fakePlayer);
        }
    }
    inline std::vector<FakePlayer const*> getFakePlayerList() const
    {
        std::vector<FakePlayer const*> list;
        forEachFakePlayer([&list](std::string_view name, FakePlayer const& fakePlayer) {
            list.push_back(&fakePlayer);
        });
        std::sort(list.begin(), list.end(), [](FakePlayer const* left, FakePlayer const* right) {
            return left->mLastOnlineTime > right->mLastOnlineTime;
        });
        return list;
    }
    FakePlayer* create(std::string const& name, std::unique_ptr<CompoundTag> playerData = {});
    bool remove(std::string const& name);
    SimulatedPlayer* login(std::string const& name) const;
    bool logout(FakePlayer& fakePlayer) const;
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

    bool swapData(FakePlayer& fakePlayer, Player& player) const;

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

class FakePlayerStorage
{
protected:
    std::unique_ptr<KVDB> mStorage;
    Logger mLogger;
    CsLock mLock;
    FakePlayerManager* mManager;

public:

    inline static std::string getStorageId(mce::UUID uuid)
    {
        return "player_" + uuid.asString();
    }
    inline static std::string getServerId(mce::UUID uuid)
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
            if (val.size() < 1024)
                debugLogNbt(*CompoundTag::fromBinaryNBT((void*)val.data(), val.size()));
            return true;
        });
#endif // DEBUG
    }

protected:
    inline bool fixDatabase() {

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
            __debugbreak();
            return false;
        }
        auto serverId = fakePlayer.getServerId();
        res = mStorage->del(serverId);
        if (!res)
        {
            mLogger.error("Error in {} - {}", __FUNCTION__, serverId);
            __debugbreak();
            return false;
        }
        return true;
    };
    inline bool savePlayer(FakePlayer const& fakePlayer)
    {
        auto res = savePlayerInfo(fakePlayer);
        if (fakePlayer.online())
            res &= savePlayerOnlineData(fakePlayer);
        return res;
    }
    inline bool savePlayerOnlineData(FakePlayer const& fakePlayer)
    {
        CsLockHolder lock(mLock);
        auto serverId = fakePlayer.getServerId();
        auto tag = fakePlayer.getOnlinePlayerTag();
        if (tag && mStorage->set(serverId, tag->toBinaryNBT(true)))
            return true;
        mLogger.error("Error in {} - {}", __FUNCTION__, serverId);
        __debugbreak();
        return false;
    };
    inline bool savePlayerTag(FakePlayer const& fakePlayer, CompoundTag const& tag)
    {
        CsLockHolder lock(mLock);
        auto serverId = fakePlayer.getServerId();
        if (mStorage->set(serverId, const_cast<CompoundTag&>(tag).toBinaryNBT(true)))
            return true;
        mLogger.error("Error in {} - {}", __FUNCTION__, serverId);
        __debugbreak();
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
        __debugbreak();
        return false;
    };
    inline std::string getPlayerData(mce::UUID uuid)
    {
        CsLockHolder lock(mLock);
        auto serverId = getServerId(uuid);
        std::string data = "";
        if (mStorage->get(serverId, data))
            return data;
        mLogger.error("Error in {} - {}", __FUNCTION__, serverId);
        __debugbreak();
        return "";
    };
    inline std::string getPlayerData(FakePlayer const& fakePlayer)
    {
        return getPlayerData(fakePlayer.getUUID());
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
        return getPlayerTag(fakePlayer.getUUID());
    }
    inline std::shared_ptr<FakePlayer> getFakePlayer(mce::UUID uuid)
    {
        CsLockHolder lock(mLock);
        auto storageId = getStorageId(uuid);
        std::string data = "";
        auto res = mStorage->get(storageId, data);
        if (!res)
        {
            mLogger.error("Error in {} - {}", __FUNCTION__, storageId);
            __debugbreak();
            return {};
        }
        auto info = CompoundTag::fromBinaryNBT(data, true);
        return FakePlayer::deserialize(*info);
    };
};