#pragma once
#include <MC/SimulatedPlayer.hpp>
#include <MC/CompoundTag.hpp>
#include <MC/Int64Tag.hpp>
#include <MC/StringTag.hpp>
#include <MC/ByteTag.hpp>
#include "Utils/ColorHelper.h"
#include <functional>
#include "SimulatedPlayerHelper.h"
#include "FakePlayerAPI.h"
#include "Main/FakePlayer.h"


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
