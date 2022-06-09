#include "pch.h"
#include "FakePlayerManager.h"
#include "FakePlayer.h"
#include "FakePlayerStorage.h"
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
#include <EventAPI.h>
#include <ScheduleAPI.h>

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


FakePlayerManager::FakePlayerManager(std::string const& dbPath)
{
    DEBUGL("FakePlayerManager::FakePlayerManager({})", dbPath);
    dAccess<short>(&FakePlayer::mNetworkID, 152) = 4;

    mStorage = std::make_unique<FakePlayerStorage>(dbPath, this);

    initFakePlayers();
    initEventListeners();
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

bool FakePlayerManager::saveData(FakePlayer const& fakePlayer)
{
    if (!fakePlayer.shouldSaveData())
        return false;
    if (fakePlayer.isOnline())
        time(&fakePlayer.mLastUpdateTime);
    return mStorage->savePlayer(fakePlayer);
}

bool FakePlayerManager::saveData(mce::UUID uuid)
{
    auto playerIter = mMap.find(uuid);
    if (playerIter != mMap.end())
        return saveData(*playerIter->second);
    return false;
}

bool FakePlayerManager::saveData(SimulatedPlayer const& simulatedPlayer)
{
    auto fakePlayer = tryGetFakePlayer(simulatedPlayer);
    if (fakePlayer)
        return saveData(*fakePlayer);
    return false;
}

bool FakePlayerManager::importClientFakePlayerData(std::string const& name)
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
    logger.error("Error in creating FakePlayer");
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
        tmp.push_back({fakePlayer->mLastUpdateTime, fakePlayer->mRealName});
    }
    std::sort(tmp.begin(), tmp.end(), [](std::tuple<time_t, std::string_view> const& left, std::tuple<time_t, std::string_view> const& right) {
        return std::get<0>(left) > std::get<0>(right);
    });
    for (auto& [time, name] : tmp)
    {
        mSortedNames.push_back(std::string(name));
    }
}
void FakePlayerManager::initEventListeners()
{
    Schedule::nextTick([]() {
        auto& manager = getManager();
        for (auto& fp : manager.getFakePlayerList()) {
            if (fp->isAutoLogin())
                fp->login();
        }
    });
    Event::PlayerJoinEvent::subscribe_ref([](Event::PlayerJoinEvent& ev) {
        auto& player = ev.mPlayer;
        auto& manager = FakePlayerManager::getManager();
        auto fakePlayer = manager.tryGetFakePlayer(*player);
        if (fakePlayer)
            manager.onLogin(*fakePlayer);
        return true;
    });
    Event::PlayerLeftEvent::subscribe_ref([](Event::PlayerLeftEvent& ev) {
        auto& manager = FakePlayerManager::getManager();
        auto fakePlayer = manager.tryGetFakePlayer(*ev.mPlayer);
        if (fakePlayer)
            manager.onLogout(*fakePlayer);
        return true;
        });
}

FakePlayerManager& FakePlayerManager::getManager()
{
    static FakePlayerManager manager(Config::DataBasePath);
    return manager;
}

extern void UpdateLLFakePlayerSoftEnum();

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
        mStorage->savePlayerWithTag(*player, *playerData);
    // player->login();
    auto ptr = player.get();
    if (ptr)
        UpdateLLFakePlayerSoftEnum();
    onAdd(*player);
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
        mStorage->removePlayer(*fakePlayer);
        UpdateLLFakePlayerSoftEnum();
        onRemove(*fakePlayer);
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

// bool FakePlayerManager::logout(std::string const& name) const
//{
//     auto fakePlayer = tryGetFakePlayer(name);
//     if (fakePlayer)
//         return fakePlayer->logout(true);
//     return false;
// }
//
// bool FakePlayerManager::logout(mce::UUID const& uuid) const
//{
//     auto fakePlayer = tryGetFakePlayer(uuid);
//     if (fakePlayer)
//         return fakePlayer->logout(true);
//     return false;
// }
//
// bool FakePlayerManager::logout(SimulatedPlayer const& simulatedPlayer) const
//{
//     auto fakePlayer = tryGetFakePlayer(simulatedPlayer);
//     if (fakePlayer)
//         return fakePlayer->logout(true);
//     return false;
// }


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
    auto syms = reinterpret_cast<std::vector<std::string>(*)(void*)>(&dlsym_reverse)(ptr);
    for (auto& sym : syms)
        logger.warn(sym);
    onPlayerLoadedCallback = [=](ServerPlayer& sp) {
        onPlayerLoadedCallback(sp);
    };
#endif // DEBUG

    auto rtn = original(this, level, sender, handler, blobCache, gameType, nid, subId, onPlayerLoadedCallback, uuid, clientId, std::move(cert), unk_int, unk_bool, entity);

#ifdef DEBUG
    auto ueif = rtn->getUserEntityIdentifierComponent();
    ASSERT(dAccess<unsigned char>(ueif, 160) == subId);
#endif // DEBUG

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
    auto ueic = player.getUserEntityIdentifierComponent();
    auto& rtn = original(_this, res, player, unkBool);
    if (player.isSimulatedPlayer())
    {
        // ASSERT(!rtn);
        if (rtn)
        {
            logger.debug("NBT for SimulatedPlayer <{}> is not empty", player.getNameTag());
#ifdef DEBUG
            debugLogNbt(*rtn);
#endif // DEBUG
        }
        if (FakePlayer::mLoggingIn && FakePlayer::mLoggingInPlayer)
        {
            DEBUGW("Replacing SimulatedPlayer data");
            auto& fp = FakePlayer::mLoggingInPlayer;
            DEBUGW("Uuids: {}, {}", player.getUuid(), fp->getUuid().asString());
            ASSERT(player.getUuid() == fp->getUuid().asString());
            auto playerTag = fp->getStoragePlayerTag();
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
