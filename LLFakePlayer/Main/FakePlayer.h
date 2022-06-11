#pragma once
#include <Global.h>
#include "FakePlayerAPI.h"
#include <MC/SimulatedPlayer.hpp>

class FakePlayerManager;
class NetworkIdentifier;

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
    size_t mMaxCooldownTicks = Config::DefaultMaxCooldownTicks;

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
    void setClientSubId(unsigned char subId);

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
        return getPlayer() != nullptr;
    }
    inline SimulatedPlayer* getPlayer() const
    {
        auto level = Global<Level>;
        if (level)
        {
            auto player = level->getPlayer(mUUID);
            if (player->isSimulatedPlayer())
                return static_cast<SimulatedPlayer*>(player);
        }
        return nullptr;
    }
    inline bool shouldSaveData() const
    {
        return mSaveData;
    }

    FPAPI std::unique_ptr<CompoundTag> getPlayerTag() const;
    FPAPI std::unique_ptr<CompoundTag> getStoragePlayerTag() const;
    FPAPI std::unique_ptr<CompoundTag> getOnlinePlayerTag() const;
};
