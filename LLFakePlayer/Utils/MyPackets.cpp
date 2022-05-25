#include "pch.h"
#include "MyPackets.h"
#ifdef DEBUG
Logger bsLogger("BinaryStream");
namespace
{
bool isWriting = false;
bool isHooking = false;
time_t minInterval = 1;
template <typename T>
time_t lastLogWriteTime = 0;
template <typename T>
size_t writeTimes = 0;
template <typename T>
time_t lastLogReadTime = 0;
template <typename T>
size_t readTimes = 0;
uint64_t lastReadHash = 0;
uint64_t lastWriteHash = 0;
bool LogSwitch = false;
} // namespace

#pragma region ForEachPacket

#define ForEachPacket(Func)                                                   \
    Func(ShowProfilePacket);                       /* 0    , 0     ,0      */ \
    Func(SetDefaultGameTypePacket);                /* 0    , 0     ,0      */ \
    Func(RemoveObjectivePacket);                   /* 0    , 0     ,0      */ \
    Func(RemoveVolumeEntityPacket);                /* 0    , 0     ,0      */ \
    Func(SyncActorPropertyPacket);                 /* 0    , 0     ,0      */ \
    Func(FilterTextPacket);                        /* 0    , 0     ,0      */ \
    Func(CodeBuilderPacket);                       /* 0    , 0     ,0      */ \
    Func(NetworkSettingsPacket);                   /* 0    , 0     ,0      */ \
    Func(MultiplayerSettingsPacket);               /* 0    , 0     ,0      */ \
    Func(SettingsCommandPacket);                   /* 0    , 0     ,0      */ \
    Func(MapCreateLockedCopyPacket);               /* 0    , 0     ,0      */ \
    Func(AddEntityPacket);                         /* 0    , 0     ,0      */ \
    Func(RemoveEntityPacket);                      /* 0    , 0     ,0      */ \
    Func(LevelSoundEventPacketV2);                 /* 0    , 0     ,0      */ \
    Func(ModalFormRequestPacket);                  /* 0    , 0     ,0      */ \
    Func(ModalFormResponsePacket);                 /* 0    , 0     ,0      */ \
    Func(ServerSettingsRequestPacket);             /* 0    , 0     ,0      */ \
    Func(ClientToServerHandshakePacket);           /* 0    , 0     ,0      */ \
    Func(ServerToClientHandshakePacket);           /* 0    , 0     ,0      */ \
    Func(SetTimePacket);                           /* 0    , 0     ,0      */ \
    Func(RemoveActorPacket);                       /* 0    , 0     ,0      */ \
    Func(PassengerJumpPacket);                     /* 0    , 0     ,0      */ \
    Func(LevelSoundEventPacketV1);                 /* 0    , 0     ,0      */ \
    Func(SetCommandsEnabledPacket);                /* 0    , 0     ,0      */ \
    Func(SetPlayerGameTypePacket);                 /* 0    , 0     ,0      */ \
    Func(RequestChunkRadiusPacket);                /* 0    , 0     ,0      */ \
    Func(ChunkRadiusUpdatedPacket);                /* 0    , 0     ,0      */ \
    Func(SetLastHurtByPacket);                     /* 0    , 0     ,0      */ \
    Func(AutomationClientConnectPacket);           /* 0    , 0     ,0      */ \
    Func(StopSoundPacket);                         /* 0    , 0     ,0      */ \
    Func(PlayerArmorDamagePacket);                 /* 0    , 0     ,0      */ \
    Func(ActorPickRequestPacket);                  /* 0    , 0     ,0      */ \
    Func(AddPaintingPacket);                       /* 0    , 0     ,0      */ \
    Func(StructureTemplateDataResponsePacket);     /* 0    , 0     ,0      */ \
    Func(TransferPacket);                          /* 0    , 0     ,0      */ \
    Func(UpdateEquipPacket);                       /* 0    , 0     ,0      */ \
    Func(UpdateSoftEnumPacket);                    /* 0    , 0     ,0      */ \
    Func(UpdateTradePacket);                       /* 0    , 0     ,0      */ \
    Func(CameraPacket);                            /* 0    , 0     ,0      */ \
    Func(MobArmorEquipmentPacket);                 /* 0    , 0     ,0      */ \
    Func(MobEffectPacket);                         /* 0    , 0     ,0      */ \
    Func(MoveActorAbsolutePacket);                 /* 0    , 0     ,0      */ \
    Func(NetworkStackLatencyPacket);               /* 0    , 0     ,0      */ \
    Func(NpcRequestPacket);                        /* 0    , 0     ,0      */ \
    Func(OnScreenTextureAnimationPacket);          /* 0    , 0     ,0      */ \
    Func(PacketViolationWarningPacket);            /* 0    , 0     ,0      */ \
    Func(PhotoTransferPacket);                     /* 0    , 0     ,0      */ \
    Func(PlaySoundPacket);                         /* 0    , 0     ,0      */ \
    Func(AnvilDamagePacket);                       /* 0    , 0     ,0      */ \
    Func(BlockEventPacket);                        /* 0    , 0     ,0      */ \
    Func(BookEditPacket);                          /* 0    , 0     ,0      */ \
    Func(BossEventPacket);                         /* 0    , 0     ,0      */ \
    Func(ClientboundMapItemDataPacket);            /* 0    , 0     ,0      */ \
    Func(CommandBlockUpdatePacket);                /* 0    , 0     ,0      */ \
    Func(CompletedUsingItemPacket);                /* 0    , 0     ,0      */ \
    Func(ContainerSetDataPacket);                  /* 0    , 0     ,0      */ \
    Func(CraftingEventPacket);                     /* 0    , 0     ,0      */ \
    Func(DebugInfoPacket);                         /* 0    , 0     ,0      */ \
    Func(EducationSettingsPacket);                 /* 0    , 0     ,0      */ \
    Func(EmoteListPacket);                         /* 0    , 0     ,0      */ \
    Func(EmotePacket);                             /* 0    , 0     ,0      */ \
    Func(HurtArmorPacket);                         /* 0    , 0     ,0      */ \
    Func(ItemFrameDropItemPacket);                 /* 0    , 0     ,0      */ \
    Func(LabTablePacket);                          /* 0    , 0     ,0      */ \
    Func(LecternUpdatePacket);                     /* 0    , 0     ,0      */ \
    Func(LevelEventGenericPacket);                 /* 0    , 0     ,0      */ \
    Func(PlayerEnchantOptionsPacket);              /* 0    , 0     ,0      */ \
    Func(PlayerInputPacket);                       /* 0    , 0     ,0      */ \
    Func(PlayerSkinPacket);                        /* 0    , 0     ,0      */ \
    Func(PositionTrackingDBClientRequestPacket);   /* 0    , 0     ,0      */ \
    Func(PositionTrackingDBServerBroadcastPacket); /* 0    , 0     ,0      */ \
    Func(PurchaseReceiptPacket);                   /* 0    , 0     ,0      */ \
    Func(ResourcePackChunkDataPacket);             /* 0    , 0     ,0      */ \
    Func(ResourcePackChunkRequestPacket);          /* 0    , 0     ,0      */ \
    Func(ResourcePackDataInfoPacket);              /* 0    , 0     ,0      */ \
    Func(ScriptCustomEventPacket);                 /* 0    , 0     ,0      */ \
    Func(CreatePhotoPacket);                       /* 0    , 0     ,0      */ \
    Func(EduUriResourcePacket);                    /* 0    , 0     ,0      */ \
    Func(NpcDialoguePacket);                       /* 0    , 0     ,0      */ \
    Func(SimulationTypePacket);                    /* 0    , 0     ,0      */ \
    Func(AddVolumeEntityPacket);                   /* 0    , 0     ,0      */ \
    Func(AnimateEntityPacket);                     /* 0    , 0     ,0      */ \
    Func(CameraShakePacket);                       /* 0    , 0     ,0      */ \
    Func(CodeBuilderSourcePacket);                 /* 0    , 0     ,0      */ \
    Func(ClientboundDebugRendererPacket);          /* 0    , 0     ,0      */ \
    Func(CorrectPlayerMovePredictionPacket);       /* 0    , 0     ,0      */ \
    Func(PlayerStartItemCooldownPacket);           /* 0    , 0     ,0      */ \
    Func(ScriptMessagePacket);                     /* 0    , 0     ,0      */ \
    Func(SetDisplayObjectivePacket);               /* 0    , 0     ,0      */ \
    Func(SetScorePacket);                          /* 0    , 0     ,0      */ \
    Func(SetScoreboardIdentityPacket);             /* 0    , 0     ,0      */ \
    Func(SetTitlePacket);                          /* 0    , 0     ,0      */ \
    Func(ShowCreditsPacket);                       /* 0    , 0     ,0      */ \
    Func(ShowStoreOfferPacket);                    /* 0    , 0     ,0      */ \
    Func(SpawnExperienceOrbPacket);                /* 0    , 0     ,0      */ \
    Func(SpawnParticleEffectPacket);               /* 0    , 0     ,0      */ \
    Func(StructureBlockUpdatePacket);              /* 0    , 0     ,0      */ \
    Func(StructureTemplateDataRequestPacket);      /* 0    , 0     ,0      */ \
    Func(SubClientLoginPacket);                    /* 0    , 0     ,0      */ \
    Func(SimpleEventPacket);                       /* 1    , 1     ,0      */ \
    Func(AddBehaviorTreePacket);                   /* 1    , 1     ,0      */ \
    Func(AvailableActorIdentifiersPacket);         /* 1    , 1     ,0      */ \
    Func(AvailableCommandsPacket);                 /* 1    , 1     ,0      */ \
    Func(BiomeDefinitionListPacket);               /* 1    , 1     ,0      */ \
    Func(CraftingDataPacket);                      /* 1    , 1     ,0      */ \
    Func(CreativeContentPacket);                   /* 1    , 1     ,0      */ \
    Func(GameRulesChangedPacket);                  /* 1    , 1     ,0      */ \
    Func(ResourcePackStackPacket);                 /* 1    , 1     ,0      */ \
    Func(ResourcePacksInfoPacket);                 /* 1    , 1     ,0      */ \
    Func(ItemComponentPacket);                     /* 1    , 1     ,0      */ \
    Func(SetSpawnPositionPacket);                  /* 1    , 1     ,0      */ \
    Func(StartGamePacket);                         /* 1    , 1     ,0      */ \
    Func(PlayerFogPacket);                         /* 1    , 1     ,0      */ \
    Func(AddPlayerPacket);                         /* 1    , 1     ,0      */ \
    Func(LoginPacket);                             /* 1    , 0     ,1      */ \
    Func(BlockPickRequestPacket);                  /* 1    , 0     ,1      */ \
    Func(DisconnectPacket);                        /* 1    , 1     ,0      */ \
    Func(GuiDataPickItemPacket);                   /* 1    , 1     ,0      */ \
    Func(SetLocalPlayerAsInitializedPacket);       /* 1    , 0     ,1      */ \
    Func(TextPacket);                              /* 2    , 2     ,0      */ \
    Func(PlayerListPacket);                        /* 2    , 2     ,0      */ \
    Func(PlayStatusPacket);                        /* 2    , 2     ,0      */ \
    Func(ClientCacheStatusPacket);                 /* 2    , 1     ,1      */ \
    Func(SetDifficultyPacket);                     /* 2    , 2     ,0      */ \
    Func(ChangeDimensionPacket);                   /* 2    , 2     ,0      */ \
    Func(ResourcePackClientResponsePacket);        /* 2    , 0     ,2      */ \
    Func(SetActorLinkPacket);                      /* 2    , 2     ,0      */ \
    Func(UpdatePlayerGameTypePacket);              /* 2    , 2     ,0      */ \
    Func(RespawnPacket);                           /* 3    , 3     ,0      */ \
    Func(TakeItemActorPacket);                     /* 3    , 3     ,0      */ \
    Func(PlayerHotbarPacket);                      /* 3    , 3     ,0      */ \
    Func(CommandOutputPacket);                     /* 3    , 3     ,0      */ \
    Func(TickSyncPacket);                          /* 4    , 2     ,2      */ \
    Func(CommandRequestPacket);                    /* 4    , 0     ,4      */ \
    Func(ContainerOpenPacket);                     /* 4    , 4     ,0      */ \
    Func(ContainerClosePacket);                    /* 8    , 4     ,4      */ \
    Func(InventoryContentPacket);                  /* 9    , 9     ,0      */ \
    Func(ServerSettingsResponsePacket);            /* 10   , 3     ,7      */ \
    Func(InventorySlotPacket);                     /* 12   , 12    ,0      */ \
    Func(UpdateBlockSyncedPacket);                 /* 17   , 17    ,0      */ \
    Func(PlayerActionPacket);                      /* 25   , 0     ,25     */ \
    Func(ItemStackRequestPacket);                  /* 27   , 0     ,27     */ \
    Func(ItemStackResponsePacket);                 /* 27   , 27    ,0      */ \
    Func(AdventureSettingsPacket);                 /* 42   , 2     ,40     */ \
    Func(BlockActorDataPacket);                    /* 42   , 42    ,0      */ \
    Func(EventPacket);                             /* 45   , 45    ,0      */ \
    Func(SetHealthPacket);                         /* 51   , 50    ,1      */ \
    Func(MobEquipmentPacket);                      /* 68   , 5     ,63     */ \
    Func(MotionPredictionHintsPacket);             /* 101  , 101   ,0      */ \
    Func(AddItemActorPacket);                      /* 149  , 149   ,0      */ \
    Func(UpdateAttributesPacket);                  /* 155  , 155   ,0      */ \
    Func(InteractPacket);                          /* 202  , 0     ,202    */ \
    Func(ActorEventPacket);                        /* 248  , 248   ,0      */ \
    Func(LevelEventPacket);                        /* 253  , 253   ,0      */ \
    Func(NetworkChunkPublisherUpdatePacket);       /* 315  , 315   ,0      */ \
    Func(AnimatePacket);                           /* 398  , 2     ,396    */ \
    Func(InventoryTransactionPacket);              /* 439  , 4     ,435    */ \
    Func(AddActorPacket);                          /* 459  , 459   ,0      */ \
    Func(MapInfoRequestPacket);                    /* 607  , 607   ,0      */ \
    Func(UpdateSubChunkBlocksPacket);              /* 758  , 758   ,0      */ \
    Func(SetActorMotionPacket);                    /* 894  , 894   ,0      */ \
    Func(LevelSoundEventPacket);                   /* 1187 , 627   ,560    */ \
    Func(SubChunkPacket);                          /* 2114 , 2114  ,0      */ \
    Func(SubChunkRequestPacket);                   /* 2114 , 0     ,2114   */ \
    Func(ClientCacheMissResponsePacket);           /* 2277 , 2277  ,0      */ \
    Func(ClientCacheBlobStatusPacket);             /* 2277 , 0     ,2277   */ \
    Func(LevelChunkPacket);                        /* 7153 , 7153  ,0      */ \
    Func(SetActorDataPacket);                      /* 9375 , 9375  ,0      */ \
    Func(UpdateBlockPacket);                       /* 12095, 12095 ,0      */ \
    Func(MoveActorDeltaPacket);                    /* 28931, 28931 ,0      */ \
    Func(MovePlayerPacket);                        /* 57189, 56755 ,434    */ \
    Func(PlayerAuthInputPacket);                   /* 69201, 0     ,69201  */

#define DISABLE_FOR_EACH


#pragma endregion

// TInstanceHook(void, "?writeVarInt@BinaryStream@@QEAAXH@Z",
//               BinaryStream, int value)
//{
//     if (!isHooking || !isWriting)
//         return original(this, value);
//     isWriting = true;
//     bsLogger.info("{}, {}");
//     original(this, value);
//     isWriting = false;
// }

void regCmd();

template <typename T, uint64_t hash>
void onPacketWrite(std::string const& type)
{
    regCmd();
    if (!LogSwitch) return;
    writeTimes<T> ++;
    auto newTime = time(0);
    if (hash != lastWriteHash)
    {
        lastWriteHash = hash;
        if ((writeTimes<T>) > 1000)
            return;
        logger.warn(">> {}", type);
    }
    // if (newTime - lastLogWriteTime<T> - minInterval > 0)
    //{
    //     logger.warn("{}::write", type);
    //     lastLogWriteTime<T> = newTime;
    // }
}
template <typename T, uint64_t hash>
void onPacketRead(std::string const& type)
{
    regCmd();
    if (!LogSwitch) return;
    readTimes<T> ++;
    auto newTime = time(0);
    if (hash != lastReadHash)
    {
        lastReadHash = hash;
        if ((readTimes<T>) > 1000)
            return;
        logger.warn("<< {}", type);
    }
}

#define HookPacketWrite(packetType)                                   \
    THook(void, "?write@" #packetType "@@UEBAXAEAVBinaryStream@@@Z",  \
          class packetType& packet, class BinaryStream& bs)           \
    {                                                                 \
        if (isHooking)                                                \
            return original(packet, bs);                              \
        isHooking = true;                                             \
        onPacketWrite<packetType, do_hash(#packetType)>(#packetType); \
        original(packet, bs);                                         \
        isHooking = false;                                            \
    }

#define HookPacketRead(packetType)                                                                \
    THook(void, "?_read@" #packetType "@@EEAA?AW4StreamReadResult@@AEAVReadOnlyBinaryStream@@@Z", \
          class packetType& packet, class BinaryStream& bs)                                       \
    {                                                                                             \
        if (isHooking)                                                                            \
            return original(packet, bs);                                                          \
        isHooking = true;                                                                         \
        onPacketRead<packetType, do_hash(#packetType)>(#packetType);                              \
        original(packet, bs);                                                                     \
        isHooking = false;                                                                        \
    }


ForEachPacket(HookPacketWrite);
ForEachPacket(HookPacketRead);
#define LogPacketTimes(packet) logger.warn("{:4},{:4},{}", writeTimes<packet>, readTimes<packet>, #packet)

void logPacketTimes()
{
    ForEachPacket(LogPacketTimes);
}

#include <MC/DataItem.hpp>
std::unordered_map<ActorDataIDs, std::set<DataItemType>> idTypeMap;

inline void logDataIdType(DataItem const& item)
{
    auto id = (ActorDataIDs)item.mId;
    auto iter = idTypeMap.find(id);
    // if (magic_enum::enum_name(id).empty())
    //     ASSERT(false);
    if (iter == idTypeMap.end())
        idTypeMap.emplace(id, std::set<DataItemType>{item.mType});
    else
        iter->second.emplace(item.mType);
}

THook(void, "?write@?$serialize@VDataItem@@@@SAXAEBVDataItem@@AEAVBinaryStream@@@Z",
      class DataItem const& dataItem, class BinaryStream& bs)
{
    logDataIdType(dataItem);
    original(dataItem, bs);
}
THook(std::unique_ptr<class DataItem>&, "?read@?$serialize@V?$unique_ptr@VDataItem@@U?$default_delete@VDataItem@@@std@@@std@@@@SA?AV?$unique_ptr@VDataItem@@U?$default_delete@VDataItem@@@std@@@std@@AEAVReadOnlyBinaryStream@@@Z",
      std::unique_ptr<class DataItem>& res, class ReadOnlyBinaryStream& bs)
{
    auto& rtn = original(res, bs);
    logDataIdType(*rtn);
    return rtn;
}
#include <ScheduleAPI.h>
bool Switch = false;

// THook(__int64, "??2@YAPEAX_K@Z", void* a)
//{
//     return original(a);
// }
template <typename T>
inline void* VFTABLE_ADDR;
template <typename T>
void* getVftableAddr()
{
    static bool inited = ([]() {
#define INIT_ADDR(type) \
    VFTABLE_ADDR<class type> = dlsym_real("??_7" #type "@@6B@");
        ForEachPacket(INIT_ADDR);
        return true;
    })();
    return VFTABLE_ADDR<T>;
}

std::string getClassName(Packet* packet)
{
#define RETURN_IF_FIND(type)                             \
    if (getVftableAddr<class type>() == *(void**)packet) \
        return #type;
    ForEachPacket(RETURN_IF_FIND);
    return "";
}

#include <MC/MinecraftPackets.hpp>
class PacketCommand : public Command
{
    virtual void execute(class CommandOrigin const& origin, class CommandOutput& output) const override
    {
        LogSwitch = !LogSwitch;
        logPacketTimes();
        output.success(fmt::format("§eSize of idTypeMap: {}", idTypeMap.size()));
        for (auto& [id, type] : idTypeMap)
        {
            auto key = fmt::format("{},{}", magic_enum::enum_name(id), (int)id);
            std::string types = "";
            for (auto t : type)
            {
                types += fmt::format("{},{},", magic_enum::enum_name(t), (int)t);
            }
            output.success(fmt::format("{},{}", key, types));
        }
        return;
        int packetId = 0;
        while (packetId < 200)
        {
            auto packet = MinecraftPackets::createPacket(packetId);
            if (packet)
            {
                auto size = _msize((void**)packet.get() - 2);
                // logger.warn("Packet: {},{},{},{},{}", magic_enum::enum_name((MinecraftPacketIds)packetId), packet->getName(), getClassName(packet.get()), packetId, size);

                auto className = getClassName(packet.get());
                // className.erase(className.find_last_of("Packet") - 5, 6);
                // logger.warn("{} = 0x{:02X},", className, packetId);

                // logger.info("static_assert(sizeof({}) == 0x{:X}, \"size of {} should be {}\");", className, size-16, className, size-16);

                logger.info("#include <MC/{}.hpp>", className);
            }
            packetId++;
        }
        if (auto player = Command::getPlayerFromOrigin(origin))
        {
            static std::vector<std::string> actorTypes = Global<CommandRegistry>->getEnumValues("EntityType");
            static size_t index = 0;
            static ScheduleTask task = Schedule::repeat(
                [=]() {
                    if (index == actorTypes.size())
                    {
                        player->sendText("task.cancel()");
                        const_cast<ScheduleTask&>(task).cancel();
                        return;
                    }
                    player->sendText("§e Summon " + actorTypes.at(index++));
                    player->runcmd(fmt::format("summon {}", actorTypes.at(index++)));
                },
                20);
        }
    };

public:
    static void setup(CommandRegistry& registry)
    {
        registry.registerCommand("packet", "Show Packet", CommandPermissionLevel::Any, {(CommandFlagValue)0}, {(CommandFlagValue)0x80});
        registry.registerOverload<PacketCommand>("packet");
    }
};
#include <DynamicCommandAPI.h>
#include <MC/SimulatedPlayer.hpp>

void regCmd()
{
    if (!Global<CommandRegistry> || !Global<Level>)
        return;
    static bool reg = false;
    if (reg)
        return;
    reg = !reg;
    PacketCommand::setup(*Global<CommandRegistry>);
    return;
}

using Result = DynamicCommand::Result;
using ParamType = DynamicCommand::ParameterType;
using ParamPtr = DynamicCommand::ParameterPtr;
using ParamData = DynamicCommand::ParameterData;
using ParamIndex = DynamicCommandInstance::ParameterIndex;
constexpr auto AutoExpandOption = CommandParameterOption::EnumAutocompleteExpansion;

auto FpcCommand = ([]() {
    auto cmd = DynamicCommand::createCommand("fpc", "fake player controller", CommandPermissionLevel::GameMasters, {(CommandFlagValue)0x81});
    cmd->setEnum("ActionMove", {"move"});

    cmd->mandatory("player", ParamType::Player);
    cmd->mandatory("action", ParamType::Enum, "ActionMove", AutoExpandOption);
    cmd->mandatory("direction", ParamType::Vec3);

    cmd->addOverload("player", "ActionMove", "direction");

    cmd->setCallback([](DynamicCommand const& cmd, CommandOrigin const& origin, CommandOutput& output, std::unordered_map<std::string, Result>& results) {
        auto players = results["player"].get<std::vector<Player*>>();
        switch (do_hash(results["action"].getRaw<std::string>().c_str()))
        {
            case do_hash("move"):
                for (auto& player : players)
                {
                    if (player->isSimulatedPlayer())
                    {
                        auto dir = results["direction"].get<Vec3>();
                        ((SimulatedPlayer*)player)->simulateWorldMove(dir, 1.0f);
                        output.success(fmt::format("simulateWorldMove {} -> ({})", player->getName(), dir.toString()));
                    }
                }
                break;
            case do_hash("navi"):
                for (auto& player : players)
                {
                    if (player->isSimulatedPlayer())
                    {
                        auto pos = origin.getWorldPosition();
                        if (results["pos"].isSet)
                            pos = results["pos"].get<Vec3>();
                        ((SimulatedPlayer*)player)->simulateNavigateToLocation(pos, 1.0f);
                        output.success(fmt::format("simulateNavigateToLocation {} -> ({})", player->getName(), pos.toString()));
                    }
                }
                break;
            default:
                break;
        }
    });
    return DynamicCommand::setup(std::move(cmd));
})();

#endif // DEBUG

#pragma region TestPacket

inline Vec3 TEST_VEC3 = {12345.0f, 54321.0f, 123456.0f};
template <typename PacketType>
inline void testPacket() = delete;

template <>
inline void testPacket<ChangeDimensionPacket>()
{
    ChangeDimensionPacket pkt(2, TEST_VEC3, false);
    assert(pkt.mDimensionId == 2);
    assert(pkt.mPosition == TEST_VEC3);
    assert(pkt.mRespawn == false);

    ChangeDimensionPacket pkt2(1, TEST_VEC3, true);
    assert(pkt.mDimensionId == 2);
    assert(pkt.mPosition == TEST_VEC3);
    assert(pkt.mRespawn == false);
}

void test()
{
    testPacket<ChangeDimensionPacket>();
}

#pragma endregion

#pragma region PacketSizeStaticAssert
// copy it to end of includes

// if type T is not complete
template <typename T, size_t size, typename = void>
struct size_or
{
    static constexpr size_t value = size;
};
// if type T is complete
template <typename T, size_t size>
struct size_or<T, size, decltype(void(sizeof(T)))>
{
    static constexpr size_t value = sizeof(T);
};

template <typename T, size_t size>
constexpr size_t size_or_v = size_or<T, size>::value;

// size of T is 48 or size if type T is complete
template <typename T, size_t size>
constexpr bool check_packet_size_v = size_or_v<T, sizeof(Packet)> == size || size_or_v<T, sizeof(Packet)> == sizeof(Packet);

static_assert(check_packet_size_v<class PlayStatusPacket, 0x38>, "size of PlayStatusPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class ServerToClientHandshakePacket, 0x50>, "size of ServerToClientHandshakePacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class ClientToServerHandshakePacket, 0x30>, "size of ClientToServerHandshakePacket should be 48 or 48(default)");
static_assert(check_packet_size_v<class DisconnectPacket, 0x58>, "size of DisconnectPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class ResourcePacksInfoPacket, 0x68>, "size of ResourcePacksInfoPacket should be 104 or 48(default)");
static_assert(check_packet_size_v<class ResourcePackStackPacket, 0x128>, "size of ResourcePackStackPacket should be 296 or 48(default)");
static_assert(check_packet_size_v<class ResourcePackClientResponsePacket, 0x48>, "size of ResourcePackClientResponsePacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class TextPacket, 0xD8>, "size of TextPacket should be 216 or 48(default)");
static_assert(check_packet_size_v<class SetTimePacket, 0x38>, "size of SetTimePacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class StartGamePacket, 0x630>, "size of StartGamePacket should be 1584 or 48(default)");
static_assert(check_packet_size_v<class AddPlayerPacket, 0x2E8>, "size of AddPlayerPacket should be 744 or 48(default)");
static_assert(check_packet_size_v<class AddActorPacket, 0x180>, "size of AddActorPacket should be 384 or 48(default)");
static_assert(check_packet_size_v<class RemoveActorPacket, 0x38>, "size of RemoveActorPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class AddItemActorPacket, 0x100>, "size of AddItemActorPacket should be 256 or 48(default)");
static_assert(check_packet_size_v<class TakeItemActorPacket, 0x40>, "size of TakeItemActorPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class MoveActorAbsolutePacket, 0x50>, "size of MoveActorAbsolutePacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class MovePlayerPacket, 0x70>, "size of MovePlayerPacket should be 112 or 48(default)");
static_assert(check_packet_size_v<class PassengerJumpPacket, 0x38>, "size of PassengerJumpPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class UpdateBlockPacket, 0x48>, "size of UpdateBlockPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class AddPaintingPacket, 0x70>, "size of AddPaintingPacket should be 112 or 48(default)");
static_assert(check_packet_size_v<class TickSyncPacket, 0x40>, "size of TickSyncPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class LevelSoundEventPacketV1, 0x50>, "size of LevelSoundEventPacketV1 should be 80 or 48(default)");
static_assert(check_packet_size_v<class LevelEventPacket, 0x48>, "size of LevelEventPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class BlockEventPacket, 0x48>, "size of BlockEventPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class ActorEventPacket, 0x40>, "size of ActorEventPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class MobEffectPacket, 0x50>, "size of MobEffectPacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class UpdateAttributesPacket, 0x58>, "size of UpdateAttributesPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class InventoryTransactionPacket, 0x60>, "size of InventoryTransactionPacket should be 96 or 48(default)");
//static_assert(check_packet_size_v<class MobEquipmentPacket, 0xC8>, "size of MobEquipmentPacket should be 200 or 48(default)");
static_assert(check_packet_size_v<class MobArmorEquipmentPacket, 0x238>, "size of MobArmorEquipmentPacket should be 568 or 48(default)");
static_assert(check_packet_size_v<class InteractPacket, 0x50>, "size of InteractPacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class BlockPickRequestPacket, 0x40>, "size of BlockPickRequestPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class ActorPickRequestPacket, 0x40>, "size of ActorPickRequestPacket should be 64 or 48(default)");
//static_assert(check_packet_size_v<class PlayerActionPacket, 0x50>, "size of PlayerActionPacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class HurtArmorPacket, 0x40>, "size of HurtArmorPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class SetActorDataPacket, 0x58>, "size of SetActorDataPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class SetActorMotionPacket, 0x48>, "size of SetActorMotionPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class SetActorLinkPacket, 0x50>, "size of SetActorLinkPacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class SetHealthPacket, 0x38>, "size of SetHealthPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class SetSpawnPositionPacket, 0x50>, "size of SetSpawnPositionPacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class AnimatePacket, 0x40>, "size of AnimatePacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class RespawnPacket, 0x48>, "size of RespawnPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class ContainerOpenPacket, 0x48>, "size of ContainerOpenPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class ContainerClosePacket, 0x38>, "size of ContainerClosePacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class PlayerHotbarPacket, 0x38>, "size of PlayerHotbarPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class InventoryContentPacket, 0x50>, "size of InventoryContentPacket should be 80 or 48(default)");
//static_assert(check_packet_size_v<class InventorySlotPacket, 0xB8>, "size of InventorySlotPacket should be 184 or 48(default)");
static_assert(check_packet_size_v<class ContainerSetDataPacket, 0x40>, "size of ContainerSetDataPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class CraftingDataPacket, 0x98>, "size of CraftingDataPacket should be 152 or 48(default)");
static_assert(check_packet_size_v<class CraftingEventPacket, 0x78>, "size of CraftingEventPacket should be 120 or 48(default)");
static_assert(check_packet_size_v<class GuiDataPickItemPacket, 0x78>, "size of GuiDataPickItemPacket should be 120 or 48(default)");
static_assert(check_packet_size_v<class AdventureSettingsPacket, 0x50>, "size of AdventureSettingsPacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class BlockActorDataPacket, 0x58>, "size of BlockActorDataPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class PlayerInputPacket, 0x40>, "size of PlayerInputPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class LevelChunkPacket, 0x88>, "size of LevelChunkPacket should be 136 or 48(default)");
static_assert(check_packet_size_v<class SetCommandsEnabledPacket, 0x38>, "size of SetCommandsEnabledPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class SetDifficultyPacket, 0x38>, "size of SetDifficultyPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class ChangeDimensionPacket, 0x48>, "size of ChangeDimensionPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class SetPlayerGameTypePacket, 0x38>, "size of SetPlayerGameTypePacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class PlayerListPacket, 0x50>, "size of PlayerListPacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class SimpleEventPacket, 0x38>, "size of SimpleEventPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class EventPacket, 0x140>, "size of EventPacket should be 320 or 48(default)");
static_assert(check_packet_size_v<class SpawnExperienceOrbPacket, 0x40>, "size of SpawnExperienceOrbPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class ClientboundMapItemDataPacket, 0xB8>, "size of ClientboundMapItemDataPacket should be 184 or 48(default)");
static_assert(check_packet_size_v<class MapInfoRequestPacket, 0x38>, "size of MapInfoRequestPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class RequestChunkRadiusPacket, 0x38>, "size of RequestChunkRadiusPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class ChunkRadiusUpdatedPacket, 0x38>, "size of ChunkRadiusUpdatedPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class ItemFrameDropItemPacket, 0x40>, "size of ItemFrameDropItemPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class GameRulesChangedPacket, 0x48>, "size of GameRulesChangedPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class CameraPacket, 0x40>, "size of CameraPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class BossEventPacket, 0x80>, "size of BossEventPacket should be 128 or 48(default)");
static_assert(check_packet_size_v<class ShowCreditsPacket, 0x40>, "size of ShowCreditsPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class AvailableCommandsPacket, 0xC0>, "size of AvailableCommandsPacket should be 192 or 48(default)");
static_assert(check_packet_size_v<class CommandRequestPacket, 0x98>, "size of CommandRequestPacket should be 152 or 48(default)");
static_assert(check_packet_size_v<class CommandBlockUpdatePacket, 0xB0>, "size of CommandBlockUpdatePacket should be 176 or 48(default)");
static_assert(check_packet_size_v<class CommandOutputPacket, 0xA0>, "size of CommandOutputPacket should be 160 or 48(default)");
static_assert(check_packet_size_v<class UpdateTradePacket, 0x90>, "size of UpdateTradePacket should be 144 or 48(default)");
static_assert(check_packet_size_v<class UpdateEquipPacket, 0x58>, "size of UpdateEquipPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class ResourcePackDataInfoPacket, 0x88>, "size of ResourcePackDataInfoPacket should be 136 or 48(default)");
static_assert(check_packet_size_v<class ResourcePackChunkDataPacket, 0x78>, "size of ResourcePackChunkDataPacket should be 120 or 48(default)");
static_assert(check_packet_size_v<class ResourcePackChunkRequestPacket, 0x58>, "size of ResourcePackChunkRequestPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class TransferPacket, 0x58>, "size of TransferPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class PlaySoundPacket, 0x68>, "size of PlaySoundPacket should be 104 or 48(default)");
static_assert(check_packet_size_v<class StopSoundPacket, 0x58>, "size of StopSoundPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class SetTitlePacket, 0xA8>, "size of SetTitlePacket should be 168 or 48(default)");
static_assert(check_packet_size_v<class AddBehaviorTreePacket, 0x50>, "size of AddBehaviorTreePacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class StructureBlockUpdatePacket, 0xF0>, "size of StructureBlockUpdatePacket should be 240 or 48(default)");
static_assert(check_packet_size_v<class ShowStoreOfferPacket, 0x78>, "size of ShowStoreOfferPacket should be 120 or 48(default)");
static_assert(check_packet_size_v<class PurchaseReceiptPacket, 0x48>, "size of PurchaseReceiptPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class PlayerSkinPacket, 0x2E8>, "size of PlayerSkinPacket should be 744 or 48(default)");
static_assert(check_packet_size_v<class SubClientLoginPacket, 0x38>, "size of SubClientLoginPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class AutomationClientConnectPacket, 0x50>, "size of AutomationClientConnectPacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class SetLastHurtByPacket, 0x38>, "size of SetLastHurtByPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class BookEditPacket, 0xA0>, "size of BookEditPacket should be 160 or 48(default)");
static_assert(check_packet_size_v<class NpcRequestPacket, 0x88>, "size of NpcRequestPacket should be 136 or 48(default)");
static_assert(check_packet_size_v<class PhotoTransferPacket, 0xC0>, "size of PhotoTransferPacket should be 192 or 48(default)");
static_assert(check_packet_size_v<class ModalFormRequestPacket, 0x58>, "size of ModalFormRequestPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class ModalFormResponsePacket, 0x58>, "size of ModalFormResponsePacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class ServerSettingsRequestPacket, 0x30>, "size of ServerSettingsRequestPacket should be 48 or 48(default)");
static_assert(check_packet_size_v<class ServerSettingsResponsePacket, 0x58>, "size of ServerSettingsResponsePacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class ShowProfilePacket, 0x50>, "size of ShowProfilePacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class SetDefaultGameTypePacket, 0x38>, "size of SetDefaultGameTypePacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class RemoveObjectivePacket, 0x50>, "size of RemoveObjectivePacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class SetDisplayObjectivePacket, 0xB8>, "size of SetDisplayObjectivePacket should be 184 or 48(default)");
static_assert(check_packet_size_v<class SetScorePacket, 0x50>, "size of SetScorePacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class LabTablePacket, 0x48>, "size of LabTablePacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class UpdateBlockSyncedPacket, 0x58>, "size of UpdateBlockSyncedPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class MoveActorDeltaPacket, 0x70>, "size of MoveActorDeltaPacket should be 112 or 48(default)");
static_assert(check_packet_size_v<class SetScoreboardIdentityPacket, 0x50>, "size of SetScoreboardIdentityPacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class SetLocalPlayerAsInitializedPacket, 0x38>, "size of SetLocalPlayerAsInitializedPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class UpdateSoftEnumPacket, 0x70>, "size of UpdateSoftEnumPacket should be 112 or 48(default)");
static_assert(check_packet_size_v<class NetworkStackLatencyPacket, 0x40>, "size of NetworkStackLatencyPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class ScriptCustomEventPacket, 0x60>, "size of ScriptCustomEventPacket should be 96 or 48(default)");
static_assert(check_packet_size_v<class SpawnParticleEffectPacket, 0x70>, "size of SpawnParticleEffectPacket should be 112 or 48(default)");
static_assert(check_packet_size_v<class AvailableActorIdentifiersPacket, 0x48>, "size of AvailableActorIdentifiersPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class LevelSoundEventPacketV2, 0x70>, "size of LevelSoundEventPacketV2 should be 112 or 48(default)");
static_assert(check_packet_size_v<class NetworkChunkPublisherUpdatePacket, 0x40>, "size of NetworkChunkPublisherUpdatePacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class BiomeDefinitionListPacket, 0x48>, "size of BiomeDefinitionListPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class LevelSoundEventPacket, 0x70>, "size of LevelSoundEventPacket should be 112 or 48(default)");
static_assert(check_packet_size_v<class LevelEventGenericPacket, 0x40>, "size of LevelEventGenericPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class LecternUpdatePacket, 0x48>, "size of LecternUpdatePacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class AddEntityPacket, 0x38>, "size of AddEntityPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class RemoveEntityPacket, 0x38>, "size of RemoveEntityPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class ClientCacheStatusPacket, 0x38>, "size of ClientCacheStatusPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class OnScreenTextureAnimationPacket, 0x38>, "size of OnScreenTextureAnimationPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class MapCreateLockedCopyPacket, 0x40>, "size of MapCreateLockedCopyPacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class StructureTemplateDataRequestPacket, 0xC8>, "size of StructureTemplateDataRequestPacket should be 200 or 48(default)");
static_assert(check_packet_size_v<class StructureTemplateDataResponsePacket, 0x60>, "size of StructureTemplateDataResponsePacket should be 96 or 48(default)");
static_assert(check_packet_size_v<class ClientCacheBlobStatusPacket, 0x60>, "size of ClientCacheBlobStatusPacket should be 96 or 48(default)");
static_assert(check_packet_size_v<class ClientCacheMissResponsePacket, 0x88>, "size of ClientCacheMissResponsePacket should be 136 or 48(default)");
static_assert(check_packet_size_v<class EducationSettingsPacket, 0x178>, "size of EducationSettingsPacket should be 376 or 48(default)");
static_assert(check_packet_size_v<class EmotePacket, 0x60>, "size of EmotePacket should be 96 or 48(default)");
static_assert(check_packet_size_v<class MultiplayerSettingsPacket, 0x38>, "size of MultiplayerSettingsPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class SettingsCommandPacket, 0x58>, "size of SettingsCommandPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class AnvilDamagePacket, 0x40>, "size of AnvilDamagePacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class CompletedUsingItemPacket, 0x38>, "size of CompletedUsingItemPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class NetworkSettingsPacket, 0x38>, "size of NetworkSettingsPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class PlayerAuthInputPacket, 0xA8>, "size of PlayerAuthInputPacket should be 168 or 48(default)");
static_assert(check_packet_size_v<class CreativeContentPacket, 0x50>, "size of CreativeContentPacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class PlayerEnchantOptionsPacket, 0x48>, "size of PlayerEnchantOptionsPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class ItemStackRequestPacket, 0x38>, "size of ItemStackRequestPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class ItemStackResponsePacket, 0x48>, "size of ItemStackResponsePacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class PlayerArmorDamagePacket, 0x40>, "size of PlayerArmorDamagePacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class CodeBuilderPacket, 0x58>, "size of CodeBuilderPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class UpdatePlayerGameTypePacket, 0x40>, "size of UpdatePlayerGameTypePacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class EmoteListPacket, 0x50>, "size of EmoteListPacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class PositionTrackingDBServerBroadcastPacket, 0x50>, "size of PositionTrackingDBServerBroadcastPacket should be 80 or 48(default)");
static_assert(check_packet_size_v<class PositionTrackingDBClientRequestPacket, 0x38>, "size of PositionTrackingDBClientRequestPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class DebugInfoPacket, 0x58>, "size of DebugInfoPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class PacketViolationWarningPacket, 0x60>, "size of PacketViolationWarningPacket should be 96 or 48(default)");
static_assert(check_packet_size_v<class MotionPredictionHintsPacket, 0x48>, "size of MotionPredictionHintsPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class AnimateEntityPacket, 0xD8>, "size of AnimateEntityPacket should be 216 or 48(default)");
static_assert(check_packet_size_v<class CameraShakePacket, 0x40>, "size of CameraShakePacket should be 64 or 48(default)");
static_assert(check_packet_size_v<class PlayerFogPacket, 0x48>, "size of PlayerFogPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class CorrectPlayerMovePredictionPacket, 0x58>, "size of CorrectPlayerMovePredictionPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class ItemComponentPacket, 0x48>, "size of ItemComponentPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class FilterTextPacket, 0x58>, "size of FilterTextPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class ClientboundDebugRendererPacket, 0x88>, "size of ClientboundDebugRendererPacket should be 136 or 48(default)");
static_assert(check_packet_size_v<class SyncActorPropertyPacket, 0x48>, "size of SyncActorPropertyPacket should be 72 or 48(default)");
static_assert(check_packet_size_v<class AddVolumeEntityPacket, 0x100>, "size of AddVolumeEntityPacket should be 256 or 48(default)");
static_assert(check_packet_size_v<class RemoveVolumeEntityPacket, 0x38>, "size of RemoveVolumeEntityPacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class SimulationTypePacket, 0x38>, "size of SimulationTypePacket should be 56 or 48(default)");
static_assert(check_packet_size_v<class NpcDialoguePacket, 0xC0>, "size of NpcDialoguePacket should be 192 or 48(default)");
static_assert(check_packet_size_v<class EduUriResourcePacket, 0x70>, "size of EduUriResourcePacket should be 112 or 48(default)");
static_assert(check_packet_size_v<class CreatePhotoPacket, 0x78>, "size of CreatePhotoPacket should be 120 or 48(default)");
static_assert(check_packet_size_v<class UpdateSubChunkBlocksPacket, 0x70>, "size of UpdateSubChunkBlocksPacket should be 112 or 48(default)");
static_assert(check_packet_size_v<class SubChunkPacket, 0x60>, "size of SubChunkPacket should be 96 or 48(default)");
static_assert(check_packet_size_v<class SubChunkRequestPacket, 0x80>, "size of SubChunkRequestPacket should be 128 or 48(default)");
static_assert(check_packet_size_v<class PlayerStartItemCooldownPacket, 0x58>, "size of PlayerStartItemCooldownPacket should be 88 or 48(default)");
static_assert(check_packet_size_v<class ScriptMessagePacket, 0x70>, "size of ScriptMessagePacket should be 112 or 48(default)");
static_assert(check_packet_size_v<class CodeBuilderSourcePacket, 0x58>, "size of CodeBuilderSourcePacket should be 88 or 48(default)");

#pragma endregion
