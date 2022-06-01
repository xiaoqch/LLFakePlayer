#pragma once
#include <Global.h>
#include <MC/BinaryStream.hpp>
#include <MC/Block.hpp>
#include <MC/Level.hpp>
#include <MC/MinecraftEventing.hpp>
#include <MC/NetworkIdentifier.hpp>
#include <MC/NetworkItemStackDescriptor.hpp>
#include <MC/Player.hpp>
#include <MC/ServerNetworkHandler.hpp>
#include <MC/SynchedActorData.hpp>
#include <magic_enum/magic_enum.hpp>

//#include <MC/Packet.hpp>
class ReadOnlyBinaryStream;
class BinaryStream;
class ServerPlayer;
class NetworkIdentifier;
enum StreamReadResult;
enum class PacketReliability
{
    Relible,
    RelibleOrdered
};


class Packet
{
public:
    unsigned unk2 = 2;                                                     // 8
    PacketReliability reliableOrdered = PacketReliability::RelibleOrdered; // 12
    unsigned char clientSubId = 0;                                         // 16
    uint64_t unk24 = 0;                                                    // 24
    uint64_t unk40 = 0;                                                    // 32
    uint32_t incompressible = 0;                                           // 40

    inline Packet(unsigned compress)
        : incompressible(!compress)
    {
    }
    inline Packet()
    {
    }
    class Packet& operator=(class Packet const&) = delete;
    Packet(class Packet const&) = delete;

    inline ServerPlayer* getPlayerFromPacket(ServerNetworkHandler* handler, NetworkIdentifier* netId)
    {
        return handler->getServerPlayer(*netId, dAccess<char>(this, 16));
    }
    inline enum StreamReadResult read(class ReadOnlyBinaryStream& binaryStream)
    {
        return _read(binaryStream);
    }

    std::string toDebugString()
    {
        return fmt::format("{}({})->{}", getName(), getId(), clientSubId);
    }


public:
    /*0*/ virtual ~Packet();
    /*1*/ virtual enum MinecraftPacketIds getId() const = 0;
    /*2*/ virtual std::string getName() const = 0;
    /*3*/ virtual void write(class BinaryStream&) const = 0;
    /*4*/ virtual struct ExtendedStreamReadResult readExtended(class ReadOnlyBinaryStream&);
    /*5*/ virtual bool disallowBatching() const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&) = 0;
};
inline std::string getPacketIdAndName(Packet const& pkt)
{
    return fmt::format("{}({})", pkt.getName(), pkt.getId());
}


#pragma region ToDebugString

template <typename _Traits>
std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, Vec3 const& _Val)
{
    return _Ostr << fmt::format("{:.2f},{:.2f},{:.2f}", _Val.x, _Val.y, _Val.z);
}
template <typename _Traits>
std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, BlockPos const& _Val)
{
    return _Ostr << _Val.toString();
}
template <typename _Traits>
std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, ActorRuntimeID const& _Val)
{
    return _Ostr << (int64_t)_Val;
}
template <typename _Traits>
std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, ActorUniqueID const& _Val)
{
    return _Ostr << (int64_t)_Val;
}
template <typename _Traits>
std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, Tick const& _Val)
{
    return _Ostr << _Val.t;
}
template <typename _Traits>
std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, mce::UUID const& _Val)
{
    return _Ostr << _Val.asString();
}
template <typename T>
inline std::enable_if_t<std::is_same_v<T, unsigned char> || std::is_same_v<T, char>, int>
    charFilter(T const& _Val)
{
    return (int)_Val;
}
template <typename T>
inline std::enable_if_t<!std::is_same_v<T, unsigned char> && !std::is_same_v<T, char>, T const&>
    charFilter(T const& _Val)
{
    return _Val;
}
template <typename _Traits, typename _Ty>
inline std::enable_if_t<std::is_enum_v<_Ty>, std::basic_ostream<char, _Traits>&>
    operator<<(std::basic_ostream<char, _Traits>& _Ostr, _Ty const& _Val)
{
    return _Ostr << magic_enum::enum_name(_Val) << '(' << (int64_t)_Val << ')';
}
#define KeyAndVal(var) ", "## #var##": " << charFilter(var)

#pragma endregion

// ChangeDimensionPacket
class ChangeDimensionPacket : public Packet
{
public:
    AutomaticID<Dimension, int> mDimensionId;
    Vec3 mPosition;
    bool mRespawn;
    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);
        oss << KeyAndVal(mDimensionId) << KeyAndVal(mPosition) << KeyAndVal(mRespawn);
        return oss.str();
    }

public:
    /*0*/ virtual ~ChangeDimensionPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);
    MCAPI ChangeDimensionPacket(class AutomaticID<class Dimension, int>, class Vec3, bool);
    MCAPI ChangeDimensionPacket();
};

// ChunkRadiusUpdatedPacket
class ChunkRadiusUpdatedPacket : public Packet
{
public:
    /*0*/ virtual ~ChunkRadiusUpdatedPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);

    MCAPI ChunkRadiusUpdatedPacket(int);
    MCAPI ChunkRadiusUpdatedPacket();
};

// CraftingEventPacket
static_assert(sizeof(NetworkItemStackDescriptor) == 152);

class CraftingEventPacket : public Packet
{
public:
    unsigned char mScreenID;                              // 48
    unsigned int mCraftingType;                           // 52
    mce::UUID mRecipeUUID;                                // 56
    std::vector<NetworkItemStackDescriptor> mInputItems;  // 72
    std::vector<NetworkItemStackDescriptor> mOutputItems; // 96

    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);
        oss << KeyAndVal(mScreenID) << KeyAndVal(mCraftingType) << KeyAndVal(mRecipeUUID) << KeyAndVal(mInputItems.size()) << KeyAndVal(mOutputItems.size());
        return oss.str();
    }

public:
    /*0*/ virtual ~CraftingEventPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);
    MCAPI CraftingEventPacket();
};

// EventPacket
enum EventType : int
{
    ACHIEVEMENT_AWARDED,
    ENTITY_INTERACT,
    PORTAL_BUILT,
    PORTAL_USED,
    MOB_KILLED,
    CAULDRON_USED,
    PLAYER_DEATH,
    BOSS_KILLED,
    AGENT_COMMAND,
    AGENT_CREATED,
    PATTERN_REMOVED,
    SLASH_COMMAND_EXECUTED,
    FISH_BUCKETED,
    MOB_BORN,
    PET_DIED,
    CAULDRON_BLOCK_USED,
    COMPOSTER_BLOCK_USED,
    BELL_BLOCK_USED,
};

class EventPacket : public Packet
{
public:
    struct Data
    {
        EventType mType;
        unsigned char mUsePlayerId; // whether uniqueId is set
        int mInt_8;                 // fromDimensionID in PORTAL_USED Event
        int mInt_12;                // toDimensionID in PORTAL_USED Event
        char filler16[40 - 16];
        std::string unk40;
        std::string unk72;
        std::string unk104;
        std::string unk136;
        std::string unk168;
        std::string unk200;
        std::string unk232;
    }; // size: 264
    enum AgentResult;
    ActorUniqueID mUniqueId;
    Data mData;
    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);
        oss << KeyAndVal(mUniqueId) << KeyAndVal(mData.mType) << std::endl;
        return oss.str();
    }

public:
    /*0*/ virtual ~EventPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);

    MCAPI EventPacket(class Player const&, std::string const&);
    MCAPI EventPacket(class Player const&, std::string const&, int);
    MCAPI EventPacket(class Player const&, enum MovementEventType, float, float, float, float, float);
    MCAPI EventPacket(class Player const*, struct EventPacket::Data const&);
    MCAPI EventPacket(class Player const*, std::string const&, int, int, std::string const&);
    MCAPI EventPacket(class Player const*, class Raid const&, bool);
    MCAPI EventPacket(class Player const*, short, unsigned int, short);
    MCAPI EventPacket(class Player const*, int, int, int, int, int);
    MCAPI EventPacket(class Player const*, int, int, enum ActorDamageCause, bool);
    MCAPI EventPacket(class Player const*, int, class gsl::not_null<class Actor const*>);
    MCAPI EventPacket(class Player const*, class Actor const*, class gsl::not_null<class Mob const*>, enum ActorDamageCause, std::string, int, enum ActorType);
    MCAPI EventPacket(class Player const*, class Actor const*, class gsl::not_null<class Mob const*>, enum ActorDamageCause, enum ActorType);
    MCAPI EventPacket(class Player const*, class AutomaticID<class Dimension, int>, class AutomaticID<class Dimension, int>);
    MCAPI EventPacket(class Player const*, class AutomaticID<class Dimension, int>);
    MCAPI EventPacket(class Player const*, class gsl::not_null<class Actor const*>, enum MinecraftEventing::InteractionType);
    MCAPI EventPacket(class Player const*, class gsl::not_null<class Mob const*>);
    MCAPI EventPacket(class Player const*, enum MinecraftEventing::AchievementIds, bool);
    MCAPI EventPacket(class Player const*, enum EventPacket::AgentResult, std::string const&, std::string const&, std::string const&);
    MCAPI EventPacket(class Player const*, enum EventPacket::AgentResult, std::string const&, std::string const&, int);
    MCAPI EventPacket(class Player const*, bool, class Actor const*, class gsl::not_null<class Mob const*>, enum ActorDamageCause);
    MCAPI EventPacket();
    MCAPI class EventPacket& operator=(class EventPacket&&);
};

// ItemStackRequestPacket
class ItemStackRequestPacket : public Packet
{
public:
    /*0*/ virtual ~ItemStackRequestPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);
    MCAPI ItemStackRequestPacket();
    MCAPI class ItemStackRequestBatch const& getRequestBatch() const;
};

// ModalFormRequestPacket
class ModalFormRequestPacket : public Packet
{
public:
    int mFormId;
    std::string mData;

    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);
        oss << KeyAndVal(mFormId) << KeyAndVal(mData);
        return oss.str();
    }

public:
    /*0*/ virtual ~ModalFormRequestPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);
    MCAPI ModalFormRequestPacket();
};

// ModalFormResponsePacket
class ModalFormResponsePacket : public Packet
{
public:
    int mFormId;
    std::string mData;

    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);
        oss << KeyAndVal(mFormId) << KeyAndVal(mData);
        return oss.str();
    }

public:
    /*0*/ virtual ~ModalFormResponsePacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);
    MCAPI ModalFormResponsePacket();
};

// MovePlayerPacket
enum TeleportationCause
{
    UNKNOWN,
    PROJECTILE,
    CHORUS_FRUIT,
    COMMAND,
    BEHAVIOR,
};

class MovePlayerPacket : public Packet
{
public:
    ActorRuntimeID mRuntimeId;
    Vec3 mPosition;
    float mPitch;
    float mYaw;
    float mHeadYaw;
    Player::PositionMode mMode;
    bool mOnGround;
    ActorRuntimeID mRidingRuntimeId;
    TeleportationCause mTeleportationCause;
    int mEntityType;
    long mTick;

    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);
        oss << KeyAndVal(mRuntimeId) << KeyAndVal(mPosition)
            << KeyAndVal(mPitch) << KeyAndVal(mYaw) << KeyAndVal(mHeadYaw)
            << KeyAndVal(mMode) << KeyAndVal(mOnGround) << KeyAndVal(mRidingRuntimeId)
            << KeyAndVal(mTeleportationCause) << KeyAndVal(mEntityType) << KeyAndVal(mTick);
        return oss.str();
    }

public:
    /*0*/ virtual ~MovePlayerPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);
    MCAPI MovePlayerPacket(class Player const&, class Vec3 const&);
    MCAPI MovePlayerPacket(class Player const&, enum Player::PositionMode, int, int);
    MCAPI MovePlayerPacket();
};

// Refer to https://github.com/LiteLDev/BEProtocolGolang/blob/master/minecraft/protocol/player.go
enum PlayerActionType
{
    StartBreak,
    AbortBreak,
    StopBreak,
    GetUpdatedBlock,
    DropItem,
    StartSleeping,
    StopSleeping,
    Respawn,
    Jump,
    StartSprint,
    StopSprint,
    StartSneak,
    StopSneak,
    CreativePlayerDestroyBlock,
    DimensionChangeDone,
    StartGlide,
    StopGlide,
    BuildDenied,
    CrackBreak,
    ChangeSkin,
    SetEnchantmentSeed,
    StartSwimming,
    StopSwimming,
    StartSpinAttack,
    StopSpinAttack,
    StartBuildingBlock,
    PredictDestroyBlock,
    ContinueDestroyBlock,
};

class PlayerActionPacket : public Packet
{
public:
    BlockPos mPosition;           // 48
    #if BDS_VER > 11910
    BlockPos mPosition2;          // 60
    #endif
    FaceID mBlockFace;            // 72
    PlayerActionType mActionType; // 76
    ActorRuntimeID mRuntimeID;    // 80
    
    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);
        oss << KeyAndVal(mPosition) << KeyAndVal(mBlockFace)
            << KeyAndVal(mActionType) << KeyAndVal(mRuntimeID);
        return oss.str();
    }

public:
    /*0*/ virtual ~PlayerActionPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);
    MCAPI PlayerActionPacket(enum PlayerActionType, class BlockPos const&, unsigned char, class ActorRuntimeID);
    MCAPI PlayerActionPacket(enum PlayerActionType, class BlockPos const&, int, class ActorRuntimeID);
    MCAPI PlayerActionPacket(enum PlayerActionType, class ActorRuntimeID);
    MCAPI PlayerActionPacket();
};

// PlayStatusPacket
enum PlayStatus : int
{
    LoginSuccess,
    LoginFailedClient,
    LoginFailedServer,
    PlayerSpawn,
    LoginFailedInvalidTenant,
    LoginFailedVanillaEdu,
    LoginFailedEduVanilla,
    LoginFailedServerFull,
};

class PlayStatusPacket : public Packet
{
public:
    PlayStatus status;

    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);
        oss << KeyAndVal(status);
        return oss.str();
    }

public:
    /*0*/ virtual ~PlayStatusPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);
    MCAPI PlayStatusPacket(enum PlayStatus);
    MCAPI PlayStatusPacket();
};

// RespawnPacket
enum PlayerRespawnState : char
{
    SERVER_SEARCHING = 0,
    SERVER_READY,
    CLIENT_READY,
};

class RespawnPacket : public Packet
{
public:
    Vec3 mRespawnPosition;
    PlayerRespawnState mRespawnState;
    ActorRuntimeID mRuntimeId;

    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);
        oss << KeyAndVal(mRespawnPosition) << KeyAndVal(mRespawnState) << KeyAndVal(mRuntimeId);
        return oss.str();
    }

public:
    /*0*/ virtual ~RespawnPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);
    MCAPI RespawnPacket(class Vec3 const&, enum PlayerRespawnState const&);
    MCAPI RespawnPacket();
};

// ShowCreditsPacket
class ShowCreditsPacket : public Packet
{
public:
    enum CreditsState : int
    {
        START_CREDITS,
        END_CREDITS,
    };
    ActorRuntimeID mRuntimeId;
    CreditsState mState;

    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);
        oss << KeyAndVal(mRuntimeId) << KeyAndVal(mState);
        return oss.str();
    }

public:
    /*0*/ virtual ~ShowCreditsPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);
    MCAPI ShowCreditsPacket(class ActorRuntimeID, enum ShowCreditsPacket::CreditsState);
    MCAPI ShowCreditsPacket();
};

// StartGamePacket
class StartGamePacket : public Packet
{
public:
    /*0*/ virtual ~StartGamePacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);
    MCAPI StartGamePacket(class LevelSettings const&, struct ActorUniqueID, class ActorRuntimeID, enum GameType, bool, class Vec3 const&, class Vec2 const&, std::string const&, std::string const&, class ContentIdentity const&, class BlockPalette const&, std::string const&, class BlockDefinitionGroup const&, bool, struct PlayerMovementSettings const&, std::string const&, unsigned __int64, int, unsigned __int64);
    MCAPI StartGamePacket();
};

// InventorySlotPacket
class InventorySlotPacket : public Packet
{
public:
    ContainerID mContainerId;               // 48
    int mSlot;                              // 52
    NetworkItemStackDescriptor mDescriptor; // 56

    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);
        oss << KeyAndVal(mContainerId) << KeyAndVal(mSlot);
        return oss.str();
    }

public:
    /*0*/ virtual ~InventorySlotPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);

    MCAPI InventorySlotPacket(enum ContainerID, unsigned int, class ItemStack const&);
    MCAPI InventorySlotPacket();
};

// MobEquipmentPacket
class MobEquipmentPacket : public Packet
{
public:
    ActorRuntimeID mRuntimeId;              // 48
    NetworkItemStackDescriptor mDescriptor; // 56
    int unk184;                             // 184 = mInventorySlot
    int unk188;                             // 188 = HotBarSlot
    ContainerID unk192;                     // 192 = mContainerId
    unsigned char mInventorySlot;           // 193
    unsigned char HotBarSlot;               // 194
    ContainerID mContainerId;               // 195

    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);

        oss << KeyAndVal(mRuntimeId) << KeyAndVal(mInventorySlot) << KeyAndVal(HotBarSlot) << KeyAndVal(mContainerId);
        return oss.str();
    }

public:
    /*0*/ virtual ~MobEquipmentPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);

    MCAPI MobEquipmentPacket(class ActorRuntimeID, class ItemStack const&, int, int, enum ContainerID);
    MCAPI MobEquipmentPacket(class ActorRuntimeID, class NetworkItemStackDescriptor const&, int, int, enum ContainerID);
    MCAPI MobEquipmentPacket();
};


// SetActorDataPacket
class SetActorDataPacket : public Packet
{
public:
    ActorRuntimeID mRuntimeId;
    Tick mTick;
    std::vector<std::unique_ptr<DataItem>> mDataItems; // 64

    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);

        oss << KeyAndVal(mRuntimeId) << KeyAndVal(mTick) << KeyAndVal(mDataItems.size());
        return oss.str();
    }

public:
    /*0*/ virtual ~SetActorDataPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*4*/ virtual bool disallowBatching() const;
    /*5*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);
    MCAPI SetActorDataPacket(class ActorRuntimeID, class SynchedActorData&, bool);
    MCAPI SetActorDataPacket();
};

#include <MC/ColorFormat.hpp>
class TextPacket : public Packet
{
    TextType mType;                       // 48
    std::string mSourceName;              // 56
    std::string mMessage;                 // 88
    std::vector<std::string> mParameters; // 120
    bool mNeedsTranslation;               // 144
    std::string mXUID;                    // 152
    std::string mPlatformChatID;          // 184

public:
    inline std::string toDebugString() const
    {
        std::ostringstream oss;
        oss << getPacketIdAndName(*this);

        oss << KeyAndVal(mType) << KeyAndVal(mSourceName) << KeyAndVal(mMessage)
            << KeyAndVal(mParameters.size()) << KeyAndVal(mNeedsTranslation) 
            << KeyAndVal(mXUID) << KeyAndVal(mPlatformChatID);
        return ColorFormat::convertToColsole(oss.str());
    }

    /*0*/ virtual ~TextPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);

    MCAPI TextPacket();
    MCAPI static class TextPacket createAnnouncement(std::string const&, std::string const&, std::string const&, std::string const&);
    MCAPI static class TextPacket createChat(std::string const&, std::string const&, std::string const&, std::string const&);
    MCAPI static class TextPacket createJukeboxPopup(std::string const&, std::vector<std::string> const&);
    MCAPI static class TextPacket createSystemMessage(std::string const&);
    MCAPI static class TextPacket createTextObjectMessage(class ResolvedTextObject const&, std::string, std::string);
    MCAPI static class TextPacket createTextObjectWhisperMessage(class ResolvedTextObject const&, std::string const&, std::string const&);
    MCAPI static class TextPacket createTranslated(std::string const&, std::vector<std::string> const&);
    MCAPI static class TextPacket createTranslatedAnnouncement(std::string const&, std::string const&, std::string const&, std::string const&);
    MCAPI static class TextPacket createWhisper(std::string const&, std::string const&, std::string const&, std::string const&);

};
static_assert(sizeof(TextPacket) == 168 + 48);

static_assert(offsetof(EventPacket, mData.mType) == 56);
static_assert(offsetof(ShowCreditsPacket, mState) == 56);
#if BDS_VER > 11910
static_assert(offsetof(PlayerActionPacket, mActionType) == 76);
#endif

void test();
