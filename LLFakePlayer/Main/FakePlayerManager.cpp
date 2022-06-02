#include "pch.h"
#include "FakePlayerManager.h"
#include <KVDBAPI.h>
#include <MC/StringTag.hpp>
#include <MC/ListTag.hpp>
#include <MC/LevelStorage.hpp>
#include <MC/NetworkIdentifier.hpp>
#include <MC/ServerNetworkHandler.hpp>
#include <MC/RakNet.hpp>
#include <MC/DBStorage.hpp>
#include <Utils/CryptHelper.h>
#include "SimulatedPlayerHelper.h"
#include "Utils/ColorHelper.h"
#include <LoggerAPI.h>
#include "Utils/MyPackets.h"

#ifdef VERBOSE
#include <MC/PrettySnbtFormat.hpp>
#include <MC/ColorFormat.hpp>
class PrettySnbtFormatConsole : public PrettySnbtFormat
{
public:
    inline void setMaxLevel(unsigned int maxLevel)
    {
        mMaxLevel = maxLevel;
    }
};
void debugLogNbt(CompoundTag const& tag)
{
    PrettySnbtFormat format = PrettySnbtFormat::getDefaultFormat(false);
    // format.setValueColor<Tag::Type::Short>(ColorFormat::colorCodeToColorMap.at("§7"));
    ((PrettySnbtFormatConsole&)format).setMaxLevel(2);
    logger.info("Snbt: \n{}", tag.toPrettySNBT(format));
}

void logPlayerInfo(Player* player)
{
    DEBUGW("FakePlayer: {}", player->getNameTag());
    DEBUGW("Dimension: {}, Position: ({})", (int)player->getDimensionId(), player->getPosition().toString());
    auto tag = player->getNbt();
    for (auto& [k, v] : *tag)
    {
        auto tag = const_cast<CompoundTagVariant&>(v).asTag();
        std::string value;
        switch (tag->getTagType())
        {
            case Tag::Byte:
                value = std::to_string((int)tag->asByteTag()->value());
                break;
            case Tag::End:
            case Tag::Short:
            case Tag::Int:
            case Tag::Int64:
            case Tag::Float:
            case Tag::Double:
            case Tag::ByteArray:
            case Tag::String:
            case Tag::Compound:
            case Tag::IntArray:
                value = tag->toString();
                break;
            case Tag::List:
            {
                value = "[";
                auto listTag = tag->asListTag();
                for (auto& tagi : *listTag)
                {
                    value += tagi->toString() + ", ";
                }
                value += "]";
                break;
            }
            default:
                break;
        }
        DEBUGL("{}: {}", k, value);
    }
}
#else
#define debugLogNbt(...) (void)0
#endif // VERBOSE

// for import data from [FakePlayer](https://github.com/ddf8196/FakePlayer)
mce::UUID JAVA::nameUUIDFromBytes(std::string const& name)
{
    auto md5 = CalcMD5(name);
    ASSERT(md5.size() == 32);
    // md5[6] &= 0x0f;  /* clear version        */
    // md5[6] |= 0x30;  /* set to version 3     */
    // md5[8] &= 0x3f;  /* clear variant        */
    // md5[8] |= 0x80;  /* set to IETF variant  */

    md5[12] = '3'; /* set to version 3     */
    auto c = (unsigned char)md5[16];
    ASSERT(c >= '0');
    auto b = c - '0';
    if (b > 9)
        b = 10 + c - 'A';
    if (b > 15)
        b = 10 + c - 'a';
    b &= 0x3; /* clear variant        */
    b |= 0x8; /* set to IETF variant  */

    md5[16] = "0123456789abcdef"[b];

    md5.insert(20, 1, '-');
    md5.insert(16, 1, '-');
    md5.insert(12, 1, '-');
    md5.insert(8, 1, '-');
    return mce::UUID::fromString(md5);
}

bool FakePlayer::mLoggingIn = false;
FakePlayer* FakePlayer::mLoggingInPlayer = nullptr;
NetworkIdentifier FakePlayer::mNetworkID(RakNet::UNASSIGNED_RAKNET_GUID);

#pragma region FakePlayer

unsigned char FakePlayer::getNextClientSubID()
{
    static unsigned char currentCliendSubId = 0;
    return currentCliendSubId++;
}

FakePlayer::FakePlayer(std::string const& realName, std::string xuid, mce::UUID uuid, time_t lastOnlineTime, bool autoLogin, FakePlayerManager* manager)
    : mRealName(realName)
    , mXUID(xuid)
    , mUUID(uuid)
    , mLastOnlineTime(lastOnlineTime)
    , mAutoLogin(autoLogin)
    , mManager(manager)
{
    if (!mManager)
        mManager = &FakePlayerManager::getManager();
    DEBUGL("FakePlayer::FakePlayer({}, {}, {}, {}", realName, uuid.asString(), lastOnlineTime, autoLogin);
}

FakePlayer::~FakePlayer()
{
    DEBUGL("FakePlayer::~FakePlayer() - {}", mRealName);
}

std::shared_ptr<FakePlayer> FakePlayer::deserialize(CompoundTag const& tag, FakePlayerManager* manager)
{
    try
    {
        std::string name = *tag.getStringTag("realName");
        std::string xuid = *tag.getStringTag("xuid");
        std::string suuid = *tag.getStringTag("uuid");
        auto uuid = mce::UUID::fromString(suuid);
        time_t lastOnlineTime = *tag.getInt64Tag("lastOnlineTime");
        bool autoLogin = *tag.getByteTag("autoLogin");
#ifdef DEBUG
        ASSERT(xuid == std::to_string(do_hash(name.c_str())));
        ASSERT(uuid == mce::UUID::seedFromString(name));
#endif // DEBUG
        if (name.empty() || !uuid)
        {
            logger.info("FakePlayer Data Error, name: {}, uuid: {}", name, suuid);
            return {};
        }
        return std::make_shared<FakePlayer>(name, xuid, uuid, lastOnlineTime, autoLogin, manager);
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
    tag->putString("realName", mRealName);
    tag->putString("uuid", mUUID.asString());
    tag->putString("xuid", mXUID);
    tag->putInt64("lastOnlineTime", mLastOnlineTime);
    tag->putBoolean("autoLogin", mAutoLogin);
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

bool FakePlayer::login()
{
    if (mLoggingIn || isOnline())
        return false;
    LoginHolder holder(this);
    auto player = SimulatedPlayerHelper::create(mRealName);
    if (!player)
    {
        mLoggingIn = false;
        mLoggingInPlayer = nullptr;
        return false;
    }
    mUniqueID = player->getUniqueID();
    mClientSubID = player->getClientSubId();
    time(&mLastOnlineTime);

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
        time(&mLastOnlineTime);
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

std::unique_ptr<CompoundTag> FakePlayer::getPlayerTag() const
{
    if (!isOnline())
    {
        return getStoragePlayerTag();
    }
    return CompoundTag::fromPlayer(getPlayer());
}

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

#pragma endregion

#pragma region FakePlayerManager

FakePlayerManager::FakePlayerManager(std::string const& dbPath)
{
    DEBUGL("FakePlayerManager::FakePlayerManager({})", dbPath);
    dAccess<short>(&FakePlayer::mNetworkID, 152) = 4;

    mStorage = std::make_unique<FakePlayerStorage>(dbPath, this);

    initFakePlayers();
}
FakePlayerManager::~FakePlayerManager()
{
    DEBUGW("FakePlayerManager::~FakePlayerManager()");
}
bool FakePlayerManager::savePlayers(bool onlineOnly)
{
    auto res = true;
    for (auto& [uuid, player] : mMap)
    {
        if (player->isOnline() && !saveData(*player))
            res = false;
    }
    return res;
}

bool FakePlayerManager::saveData(mce::UUID uuid)
{
    auto playerIter = mMap.find(uuid);
    if (playerIter != mMap.end())
        return saveData(*playerIter->second);
    return false;
}
bool FakePlayerManager::saveData(FakePlayer const& fakePlayer)
{
    if (fakePlayer.isOnline())
        time(&fakePlayer.mLastOnlineTime);
    return mStorage->savePlayer(fakePlayer);
}
bool FakePlayerManager::saveData(SimulatedPlayer const& simulatedPlayer)
{
    auto fakePlayer = tryGetFakePlayer(simulatedPlayer);
    if (fakePlayer)
        return saveData(*fakePlayer);
    return false;
}

bool FakePlayerManager::importData_DDF(std::string const& name)
{
    auto uuid = JAVA::nameUUIDFromBytes(name);
    auto suuid = uuid.asString();
    auto dbStorage = (DBStorage*)&Global<Level>->getLevelStorage();
    auto tag = dbStorage->getCompoundTag("player_" + uuid.asString(), (DBHelpers::Category)7);
    if (!tag)
    {
        logger.error("Error in getting PlayerStorageIds");
        return false;
    }
    auto& serverId = tag->getString("ServerId");
    if (serverId.empty())
    {
        logger.error("Error in getting player's ServerId");
        return false;
    }
    auto playerData = dbStorage->getCompoundTag(serverId, (DBHelpers::Category)7);
    auto fp = create(name, std::move(playerData));
    if (fp)
        return true;
    logger.error("Error in reading player's data");
    return false;
}

void FakePlayerManager::initFakePlayers()
{
    mMap = mStorage->loadPlayerList();

    mSortedNames.clear();
    std::vector<std::tuple<time_t, std::string_view>> tmp;
    for (auto& [uuid, fakePlayer] : mMap)
    {
        mMapByName.emplace(fakePlayer->getRealName(), fakePlayer);
        tmp.push_back({fakePlayer->mLastOnlineTime, fakePlayer->mRealName});
    }
    std::sort(tmp.begin(), tmp.end(), [](std::tuple<time_t, std::string_view> const& left, std::tuple<time_t, std::string_view> const& right) {
        return std::get<0>(left) > std::get<0>(right);
    });
    for (auto& [time, name] : tmp)
    {
        mSortedNames.push_back(std::string(name));
    }
}

FakePlayerManager& FakePlayerManager::getManager()
{
    static FakePlayerManager manager(PLUGIN_DATA_PATH);
    return manager;
}

extern void updateLLFakePlayerSoftEnum();

FakePlayer* FakePlayerManager::create(std::string const& name, std::unique_ptr<CompoundTag> playerData)
{
    if (tryGetFakePlayer(name))
        return nullptr;
    auto uuid = mce::UUID::seedFromString(name);
    auto xuid = std::to_string(do_hash(name.c_str()));
    auto player = std::make_shared<FakePlayer>(name, xuid, uuid, time(0), false);
    mMapByName.try_emplace(name, player);
    mMap.try_emplace(uuid, player);
    mSortedNames.push_back(name);
    saveData(*player);
    if (playerData)
        mStorage->savePlayerTag(*player, *playerData);
    //player->login();
    auto ptr = player.get();
    if (ptr)
        updateLLFakePlayerSoftEnum();
    return ptr;
}

bool FakePlayerManager::remove(std::string const& name)
{
    auto fakePlayer = tryGetFakePlayer(name);
    if (fakePlayer)
    {
        if (fakePlayer->isOnline())
            fakePlayer->logout(false);
        auto uuid = fakePlayer->getUuid();
        auto serverId = fakePlayer->getServerId();
        mMap.erase(uuid);
        mMapByName.erase(name);
        mSortedNames.erase(std::find(mSortedNames.begin(), mSortedNames.end(), name));
        mStorage->removePlayerData(*fakePlayer);
        updateLLFakePlayerSoftEnum();
        return true;
    }
    return false;
}

SimulatedPlayer* FakePlayerManager::login(std::string const& name) const
{
    auto fakePlayer = tryGetFakePlayer(name);
    if (fakePlayer && !fakePlayer->isOnline() && fakePlayer->login())
        return fakePlayer->getPlayer();
    return nullptr;
}

bool FakePlayerManager::logout(FakePlayer& fakePlayer) const
{
    return fakePlayer.logout(true);
}
inline std::string compareTag(CompoundTag& left, CompoundTag& right)
{
    debugLogNbt(left);
    debugLogNbt(right);
    std::unordered_set<std::string> keys;
    std::ostringstream oss;
    for (auto& [key, val] : left) {
        keys.insert(key);
    }
    for (auto& [key, val] : right)
    {
        keys.insert(key);
    }
    for (auto& key : keys)
    {
        auto leftTag = left.get(key);
        auto rightTag = right.get(key);
        if (!leftTag || !rightTag || !leftTag->equals(*rightTag))
        {
            if (leftTag)
            {
                auto type = leftTag->getId();
                DEBUGW("{}({})", magic_enum::enum_name(type), (int)type);
                if (type == Tag::Compound)
                {
                    auto k = ColorHelper::gold(key);
                    DEBUGL(ColorHelper::transformToConsole(k));
                    debugLogNbt(*leftTag->asCompoundTag());
                }
            }
            if (rightTag)
            {
                auto type = rightTag->getId();
                DEBUGW("{}({})", magic_enum::enum_name(type), (int)type);
                if (type == Tag::Compound)
                {
                    auto k = ColorHelper::gold(key);
                    DEBUGL(ColorHelper::transformToConsole(k));
                    debugLogNbt(*rightTag->asCompoundTag());
                }
            }
            if (leftTag && rightTag && (leftTag->getId() == Tag::Compound) && (rightTag->getId() == Tag::Compound))
            {
                oss << ColorHelper::gold(key) << std::endl
                    << compareTag(*leftTag->asCompoundTag(), *rightTag->asCompoundTag()) << std::endl;
            }
            else
                oss << ColorHelper::green(key) << " - " << ColorHelper::green("Left") << ": " << (!leftTag ? "nullptr" : leftTag->toString()) << ", " << ColorHelper::green("Right") << ": " << (!rightTag ? "nullptr" : rightTag->toString()) << std::endl;
        }
    }
    auto str = oss.str();
    return ColorHelper::transformToConsole(str);
}

inline void syncKey(CompoundTag& left, CompoundTag& right)
{
    for (auto& [key, val] : left)
    {
        if (!right.get(key))
        {
            auto tag = left.get(key)->copy();
            tag->deleteChildren();
            right.put(key, std::move(tag));
        }
    }
    for (auto& [key, val] : right)
    {
        if (!left.get(key))
        {
            auto tag = right.get(key)->copy();
            tag->deleteChildren();
            left.put(key, std::move(tag));
        }
    }
}

bool FakePlayerManager::swapData(FakePlayer& fakePlayer, Player& player) const
{
    auto plTag = CompoundTag::create();
    player.save(*plTag);
    auto fpTag = fakePlayer.getPlayerTag();
    syncKey(*fpTag, *plTag);

    auto vftbl = dlsym("??_7DefaultDataLoadHelper@@6B@");
    // auto helper = &vftbl;
    //player.remove();
    player.load(*fpTag, *(DataLoadHelper*)&vftbl);
    player.setUniqueID(fpTag->getInt64("UniqueID"));
    DEBUGW("Player: before - after:\n{}", compareTag(*fpTag, *player.getNbt()));
    // player.reload();
    SetActorDataPacket pkt;
    pkt.mRuntimeId = player.getRuntimeID();
    pkt.mDataItems = player.getEntityData().packAll();
    player.sendNetworkPacket(pkt);
    player.sendInventory(true);

    if (fakePlayer.isOnline())
    {
        //fakePlayer.mPlayer->remove();
        auto sp = fakePlayer.getPlayer();
        sp->load(*plTag, *(DataLoadHelper*)&vftbl);
        sp->setUniqueID(plTag->getInt64("UniqueID"));
        DEBUGW("FakePlayer: before - after:\n{}", compareTag(*plTag, *sp->getNbt()));
        sp->reload();
        sp->_sendDirtyActorData();
        sp->sendInventory(true);
    }
    else
    {
        mStorage->savePlayerTag(fakePlayer, *plTag);
    }
    return true;
}

//bool FakePlayerManager::logout(std::string const& name) const
//{
//    auto fakePlayer = tryGetFakePlayer(name);
//    if (fakePlayer)
//        return fakePlayer->logout(true);
//    return false;
//}
//
//bool FakePlayerManager::logout(mce::UUID const& uuid) const
//{
//    auto fakePlayer = tryGetFakePlayer(uuid);
//    if (fakePlayer)
//        return fakePlayer->logout(true);
//    return false;
//}
//
//bool FakePlayerManager::logout(SimulatedPlayer const& simulatedPlayer) const
//{
//    auto fakePlayer = tryGetFakePlayer(simulatedPlayer);
//    if (fakePlayer)
//        return fakePlayer->logout(true);
//    return false;
//}

#pragma endregion

#include <MC/Certificate.hpp>
TInstanceHook(SimulatedPlayer*, "??0SimulatedPlayer@@QEAA@AEAVLevel@@AEAVPacketSender@@AEAVNetworkHandler@@AEAVActiveTransfersManager@Server@ClientBlobCache@@W4GameType@@AEBVNetworkIdentifier@@EV?$function@$$A6AXAEAVServerPlayer@@@Z@std@@VUUID@mce@@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$unique_ptr@VCertificate@@U?$default_delete@VCertificate@@@std@@@std@@H_NAEAVEntityContext@@@Z",
              SimulatedPlayer,
              class Level& level,
              class PacketSender& sender,
              class NetworkHandler& handler,
              class ActiveTransfersManager& blobCache,
              enum GameType gameType,
              class NetworkIdentifier const& nid,
              unsigned char subId,
              class std::function<void(class ServerPlayer&)> onPlayerLoadedCallback,
              class mce::UUID uuid,
              std::string const& clientId,
              class std::unique_ptr<Certificate> cert,
              int unk_int,
              bool unk_bool,
              class EntityContext& entity)
{
    ASSERT(subId == 0);
    subId = 255;
    // fix Player subClinetId, for identify packet
    if (FakePlayer::mLoggingIn)
    {
        auto fp = FakePlayer::mLoggingInPlayer;
        uuid = fp->getUuid();
        subId = fp->getClientSubId();
    }
    else if (Config::DebugMode)
        logger.warn("Unknown SimulatedPlayer creation detected, it is recommended to create an SimulatedPlayer through FakePlayerManager");
    if (subId == 255)
    {
        subId = FakePlayer::getNextClientSubID();
    }
    if (FakePlayer::mLoggingIn)
    {
        FakePlayer::mLoggingInPlayer->setClientSubId(subId);
    }
    DEBUGW("SimulatedPlayer(level, sender, handler, blobCache, gameType = {}, nid, subId = {}, func, uuid = {}, clientId = {}, cert, unk_int = {}, unk_bool = {}, entity)",
           (int)gameType, (int)subId, uuid.asString(), clientId, unk_int, unk_bool);
    // fix client sub id for identify packet
#ifdef DEBUG
    auto ptr = (*(void***)&onPlayerLoadedCallback)[2];
    auto rva = (uintptr_t)ptr - (uintptr_t)GetModuleHandleW(nullptr);
    auto syms = dlsym_reverse((int)ptr);
    for (auto& sym : syms)
        logger.warn(sym);
    onPlayerLoadedCallback = [=](ServerPlayer & sp)
    {
        onPlayerLoadedCallback(sp);
    };
#endif // DEBUG

    auto rtn = original(this, level, sender, handler, blobCache, gameType, nid, subId, onPlayerLoadedCallback, uuid, clientId, std::move(cert), unk_int, unk_bool, entity);
    // fix runtime id
    auto ueif = rtn->getUserEntityIdentifierComponent();
    ASSERT(dAccess<unsigned char>(ueif, 160) == subId);

    DEBUGW("Simulated Player: {}, client sub id: {}, runtime id: {}", rtn->getNameTag(), (int)rtn->getClientSubId(), rtn->getRuntimeID().id);
    return rtn;
}

// replace player's data
// == ServerPlayer
// bedrock_server_mod.exe!ServerNetworkHandler::_loadNewPlayer(class ServerPlayer &,bool)
// bedrock_server_mod.exe!ServerNetworkHandler::trytLoadPlayer(class ServerPlayer &,class ConnectionRequest const &)
// bedrock_server_mod.exe!VanillaServerNetworkHandler::handle(class NetworkIdentifier const &,class ResourcePackClientResponsePacket const &)
// == SimulatedPlayer
// bedrock_server_mod.exe!ServerNetworkHandler::_loadNewPlayer(class ServerPlayer &,bool)
// bedrock_server_mod.exe!ServerNetworkHandler::createSimulatedPlayer(class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > const &)

THook(std::unique_ptr<CompoundTag>&, "?loadServerPlayerData@LevelStorage@@QEAA?AV?$unique_ptr@VCompoundTag@@U?$default_delete@VCompoundTag@@@std@@@std@@AEBVPlayer@@_N@Z",
      class LevelStorage* _this, std::unique_ptr<CompoundTag>& res, Player& player, bool unkBool)
{
    DEBUGW("LevelStorage::loadServerPlayerData({}, {})", player.getNameTag(), unkBool);
    auto& rtn = original(_this, res, player, unkBool);
    if (player.isSimulatedPlayer())
    {
        // ASSERT(!rtn);
        if (rtn)
        {
            logger.debug("Nbt for SimulatedPlayer is not empty");
#ifdef DEBUG
            debugLogNbt(*rtn);
#endif // DEBUG
        }
        if (FakePlayer::mLoggingIn && FakePlayer::mLoggingInPlayer)
        {
            DEBUGW("Replace SimulatedPlayer data");
            auto& fp = FakePlayer::mLoggingInPlayer;
            DEBUGW("Uuids: {}, {}", player.getUuid(), fp->getUuid().asString());
            ASSERT(player.getUuid() == fp->getUuid().asString());
            auto playerTag = FakePlayer::mLoggingInPlayer->getStoragePlayerTag();
#ifdef DEBUG
            if (playerTag)
                debugLogNbt(*playerTag);
#endif // DEBUG
            if (playerTag)
                rtn = std::move(playerTag);
        }
    }
    return rtn;
}

TInstanceHook(void, "?savePlayers@Level@@UEAAXXZ", Level)
{
    original(this);
    try
    {
        FakePlayerManager::getManager().savePlayers(true);
    }
    catch (...)
    {
        logger.error("Error in FakePlayer::savePlayers");
    }
}

