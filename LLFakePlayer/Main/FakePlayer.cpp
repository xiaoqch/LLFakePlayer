#include "pch.h"
#include "FakePlayer.h"
#include "Header/SimulatedPlayerHelper.h"
#include "Header/FakePlayerManager.h"
#include <MC/NetworkIdentifier.hpp>
#include <MC/RakNet.hpp>

bool FakePlayer::mLoggingIn = false;
FakePlayer* FakePlayer::mLoggingInPlayer = nullptr;
NetworkIdentifier FakePlayer::mNetworkID(RakNet::UNASSIGNED_RAKNET_GUID);

unsigned char FakePlayer::getNextClientSubID()
{
    static unsigned char currentCliendSubId = 0;
    return currentCliendSubId++;
}

FakePlayer::FakePlayer(std::string const& realName, std::string xuid, mce::UUID uuid, time_t lastUpdateTime, bool autoLogin, FakePlayerManager* manager)
    : mRealName(realName)
    , mXUID(xuid)
    , mUUID(uuid)
    , mLastUpdateTime(lastUpdateTime)
    , mAutoLogin(autoLogin)
    , mManager(manager)
{
    if (!mManager)
        mManager = &FakePlayerManager::getManager();
    DEBUGL("FakePlayer::FakePlayer({}, {}, {}, {}", realName, uuid.asString(), lastUpdateTime, autoLogin);
}

FakePlayer::~FakePlayer()
{
    DEBUGL("FakePlayer::~FakePlayer() - {}", mRealName);
}

std::shared_ptr<FakePlayer> FakePlayer::deserialize(CompoundTag const& tag, FakePlayerManager* manager)
{
    try
    {
        std::string name = *tag.getStringTag("RealName");
        std::string xuid = *tag.getStringTag("XUID");
        std::string suuid = *tag.getStringTag("UUID");
        auto uuid = mce::UUID::fromString(suuid);
        time_t lastUpdateTime = *tag.getInt64Tag("LastUpdateTime");
        bool autoLogin = *tag.getByteTag("AutoLogin");
#ifdef DEBUG
        ASSERT(xuid == std::to_string(do_hash(name.c_str())));
        ASSERT(uuid == mce::UUID::seedFromString(name));
#endif // DEBUG
        if (name.empty() || !uuid)
        {
            logger.error("FakePlayer Data Error, name: {}, uuid: {}", name, suuid);
            return {};
        }
        return std::make_shared<FakePlayer>(name, xuid, uuid, lastUpdateTime, autoLogin, manager);
    }
    catch (...)
    {
        logger.error("Error in " __FUNCTION__);
    }
    return {};
}

std::unique_ptr<CompoundTag> FakePlayer::serialize() const
{
    auto tag = CompoundTag::create();
    tag->putString("RealName", mRealName);
    tag->putString("UUID", mUUID.asString());
    tag->putString("XUID", mXUID);
    tag->putInt64("LastUpdateTime", mLastUpdateTime);
    tag->putBoolean("AutoLogin", mAutoLogin);
    return std::move(tag);
}

struct LoginHolder
{
    LoginHolder(FakePlayer* fp)
    {
        FakePlayer::mLoggingIn = true;
        FakePlayer::mLoggingInPlayer = fp;
    }
    ~LoginHolder()
    {
        FakePlayer::mLoggingIn = false;
        FakePlayer::mLoggingInPlayer = nullptr;
    }
};

bool FakePlayer::login(BlockPos* bpos, class AutomaticID<class Dimension, int> dimId)
{
    if (mLoggingIn || isOnline())
        return false;
    if (Global<Level>->getPlayer(mRealName))
    {
        logger.warn("Player with same name \"{}\" already online", mRealName);
        return false;
    }
    LoginHolder holder(this);
    auto player = SimulatedPlayerHelper::create(mRealName, bpos, dimId);
    if (!player)
    {
        mLoggingIn = false;
        mLoggingInPlayer = nullptr;
        return false;
    }
    mUniqueID = player->getUniqueID();
    mClientSubID = player->getClientSubId();
    time(&mLastUpdateTime);

    mLoggingIn = false;
    mLoggingInPlayer = nullptr;
    // FakePlayerManager::getManager().saveData(*this);
#ifdef DEBUG
    debugLogNbt(*player->getNbt());
#endif // DEBUG
    return true;
};

bool FakePlayer::logout(bool save)
{
    if (!isOnline())
        return false;
    if (save)
    {
        time(&mLastUpdateTime);
        FakePlayerManager::getManager().saveData(*this);
    }
    getPlayer()->simulateDisconnect();
    mClientSubID = -1;
    return true;
}

mce::UUID const& FakePlayer::getUuid() const
{
    return mUUID;
}

std::string FakePlayer::getUUIDString() const
{
    return mUUID.asString();
}

std::string FakePlayer::getServerId() const
{
    return "player_server_" + mUUID.asString();
}

std::string FakePlayer::getStorageId() const
{
    return "player_" + mUUID.asString();
}

void FakePlayer::setClientSubId(unsigned char subId)
{
    mClientSubID = subId;
}

std::unique_ptr<CompoundTag> FakePlayer::getPlayerTag() const
{
    if (!isOnline())
    {
        return getStoragePlayerTag();
    }
    return CompoundTag::fromPlayer(getPlayer());
}
#include "FakePlayerStorage.h"
std::unique_ptr<CompoundTag> FakePlayer::getStoragePlayerTag() const
{
    return mManager->mStorage->getPlayerTag(*this);
}

std::unique_ptr<CompoundTag> FakePlayer::getOnlinePlayerTag() const
{
    if (!isOnline())
        return {};
    return CompoundTag::fromPlayer(getPlayer());
}
