#pragma once
#include <Global.h>
//#define SKIN_HELPER

#ifdef SKIN_HELPER
//#include <MC/SerializedSkin.hpp>
//#include <MC/AnimatedImageData.hpp>
//#include <MC/SemVersion.hpp>
//#include <MC/SerializedPersonaPieceHandle.hpp>
#include <MC/persona.hpp>
#include <MC/JsonHelpers.hpp>
#include <MC/JsonUtil.hpp>
#include <MC/Util.hpp>
#include <lodepng.h>
class SemVersion
{

#define AFTER_EXTRA
    // Add Member There
public:
    enum MatchType;
    enum ParseOption;

    struct any_version_constructor
    {
        any_version_constructor() = delete;
        any_version_constructor(any_version_constructor const&) = delete;
        any_version_constructor(any_version_constructor const&&) = delete;
    };
    unsigned short major; // 0
    unsigned short minor; // 2
    unsigned short patch; // 2
    std::string unk8;     // 8
    std::string unk40;    // 40
    std::string unk72;    // 72
    bool valid;           // 104
    bool anyVersion;      // 105

#undef AFTER_EXTRA

public:
    MCAPI SemVersion(class SemVersion const&);
    MCAPI SemVersion(unsigned short, unsigned short, unsigned short, std::string const&, std::string const&);
    MCAPI SemVersion(struct SemVersion::any_version_constructor);
    MCAPI SemVersion();
    MCAPI std::string const& asString() const;
    MCAPI unsigned short getMajor() const;
    MCAPI unsigned short getMinor() const;
    MCAPI unsigned short getPatch() const;
    MCAPI bool isAnyVersion() const;
    MCAPI bool isValid() const;
    MCAPI bool operator<(class SemVersion const&) const;
    MCAPI bool operator<=(class SemVersion const&) const;
    MCAPI class SemVersion& operator=(class SemVersion const&);
    MCAPI bool operator==(class SemVersion const&) const;
    MCAPI bool operator>(class SemVersion const&) const;
    MCAPI bool operator>=(class SemVersion const&) const;
    MCAPI bool satisfies(class SemVersion const&) const;
    MCAPI ~SemVersion();
    MCAPI static struct any_version_constructor const AnyVersionConstructor;
    MCAPI static enum SemVersion::MatchType fromJson(class Json::Value const&, class SemVersion&, enum SemVersion::ParseOption);
    MCAPI static enum SemVersion::MatchType fromJsonArray(class Json::Value const&, class SemVersion&);
    MCAPI static enum SemVersion::MatchType fromString(std::string const&, class SemVersion&, enum SemVersion::ParseOption);

protected:
private:
    MCAPI void _parseVersionToString();
};
namespace mce
{
 class Blob
{
 public:
     void (*deleter)(unsigned char*) = &defaultDeleter;
     unsigned char* data = nullptr;
     size_t size = 0;

     MCAPI Blob(class mce::Blob&&);
     MCAPI Blob();
     MCAPI Blob(unsigned __int64);
     MCAPI ~Blob();
     MCAPI class Blob& operator=(class mce::Blob&&);
     MCAPI unsigned char const* cend(void) const;
     //MCAPI unsigned char const* cbegin(void) const;
     MCAPI static void defaultDeleter(unsigned char*);
     constexpr unsigned char* begin(void) const {
         return data;
     };
     constexpr unsigned char* end(void) const
     {
         return data + size;
     }
 };
namespace BlobHelper
{
MCAPI class mce::Blob clone(class mce::Blob const&);
MCAPI void copySpanIntoBlob(class mce::Blob&, gsl::cstring_span<-1>);
MCAPI void copyStringIntoBlob(class mce::Blob&, std::string const&);
inline void copyVectorIntoBlob(class mce::Blob& blob, std::vector<unsigned char> const& data)
{
    std::string str((char*)data.data(), data.size());
    copyStringIntoBlob(blob, str);
}
inline std::vector<unsigned char> getVectorFromBlob(class mce::Blob const& blob)
{
    std::vector<unsigned char> data{};
    data.reserve(blob.size());
    for (auto& c : const_cast<Blob&>(blob))
    {
        data.push_back(c);
    }
    return data;
}
} // namespace BlobHelper

 struct Image
{
     ImageFormat mFormat;  // 0
     unsigned int mWidth;  // 4
     unsigned int mHeight; // 8
     ImageUsage mUsage;    // 12
     Blob mBlob;           // 16

     inline bool savePngTo(std::string const& filePath) const
     {
         auto buffer = mce::BlobHelper::getVectorFromBlob(mBlob);
         auto err = lodepng::encode(filePath, buffer, mWidth, mHeight);
         return err == 0;
     }

     inline static Image readPngFrom(std::string const& filePath)
     {
         return Image(0,0,(mce::ImageFormat)0,(mce::ImageUsage)0);
     }

     MCAPI ~Image();
     MCAPI Image(unsigned int, unsigned int, enum mce::ImageFormat, enum mce::ImageUsage);
     MCAPI struct mce::Image clone(void) const;
     MCAPI bool isEmpty(void) const;
     MCAPI void resizeImageBytesToFitImageDescription(void);

 };
} // namespace mce
class TintMapColor
{
    mce::Color color;
};
class AnimatedImageData
{
public:
    enum /*persona::*/ AnimatedTextureType type;
    enum /*persona::*/ AnimationExpression expression;
    mce::Image image;
    float unk48;

    MCAPI AnimatedImageData(class AnimatedImageData const&);
    // MCAPI AnimatedImageData(enum persona::AnimatedTextureType, enum persona::AnimationExpression, struct mce::Image const&, float);
};
class SerializedPersonaPieceHandle
{
public:
    std::string unk0;        // 0
    persona::PieceType type; // 32
    mce::UUID uuid;          // 40
    bool unk56;              // 56
    std::string unk64;       // 64
    MCAPI SerializedPersonaPieceHandle(std::string const&, enum persona::PieceType, class mce::UUID, bool, std::string const&);
};

namespace persona
{
enum PieceType : unsigned int
{
    Unknown = 0x00, // or DefaultPieceNullType
    Skeleton = 0x01,
    Body = 0x02,
    Skin = 0x03,
    Bottom = 0x04,
    Feet = 0x05,
    Dress = 0x06,
    Top = 0x07,
    HighPants = 0x08,
    Hand = 0x09,
    Outerwear = 0x0A,
    FacialHair = 0x0B,
    Mouth = 0x0C,
    Eyes = 0x0D,
    Hair = 0x0E,
    Hood = 0x0F,
    Back = 0x10,
    FaceAccessory = 0x11,
    Head = 0x12,
    Legs = 0x13,
    LeftLeg = 0x14,
    RightLeg = 0x15,
    Arms = 0x16,
    LeftArm = 0x17,
    RightArm = 0x18,
    Capes = 0x19,
    ClassicSkin = 0x1A,
    Emote = 0x1B,
    Unsupported = 0x1C,
};
}
#ifdef DEBUG
inline void genPieceTypeEnum()
{
    logger.warn("True");
    for (unsigned int type = 0; type <= 0x1Cu; ++type)
    {
        auto str = persona::stringFromPieceType((persona::PieceType)type, true);
        logger.info("{} = 0x{:X},", str, type);
    }
    logger.warn("False");
    for (unsigned int type = 0; type <= 0x1Cu; ++type)
    {
        auto str = persona::stringFromPieceType((persona::PieceType)type, false);
        logger.info("{} = 0x{:X},", Util::toCamelCase(str, '_'), type);
    }
    __debugbreak();
}
// inline bool f = ([]() { genPieceTypeEnum(); return true; })();
#endif // DEBUG
class SerializedSkin
{
    // by dreamguxiang
    std::string mId, mPlayFabId, mFullId, mResourcePatch, mDefaultGeometryName;
    mce::Image mSkinImage, mCapeImage;
    std::vector<AnimatedImageData> mSkinAnimatedImages;
    Json::Value mGeometryData;
    SemVersion mGeometryDataEngineVersion;
    Json::Value mGeometryDataMutable;
    std::string mAnimationData, mCapeId;
    std::vector<SerializedPersonaPieceHandle> mPersonaPieces;
    std::string mArmSize;
    std::unordered_map<persona::PieceType, class TintMapColor> mPieceTintColors;
    mce::Color mSkinColor;
    enum TrustedSkinFlag mTrustedFlag;
    bool mIsPremium, mIsPersona, mIsPersonaCapeOnClassicSkin, misPrimaryUser;

public:
    inline std::string getGeometryDataString() const
    {
        return JsonHelpers::serialize(mGeometryData);
    }
    inline std::string getmGeometryDataMutableString() const
    {
        return JsonHelpers::serialize(mGeometryDataMutable);
    }
    inline void test() const
    {
        std::string geo = getGeometryDataString();
        std::string geom = getmGeometryDataMutableString();
        auto size = mPieceTintColors.size();
        auto& ver = mGeometryDataEngineVersion.asString();
        std::filesystem::path const skinPath = std::filesystem::path("Skins").append(mFullId);
        std::filesystem::create_directories(skinPath);

        mSkinImage.savePngTo(std::filesystem::path(skinPath).append("skin.png").u8string());
        mCapeImage.savePngTo(std::filesystem::path(skinPath).append("cape.png").u8string());
        for (size_t index = 0; index < mSkinAnimatedImages.size(); ++index)
        {
            std::string filename = fmt::format("animate-{}.png", index);
            mSkinAnimatedImages[index].image.savePngTo(std::filesystem::path(skinPath).append(filename).u8string());
        }
        WriteAllFile(std::filesystem::path(skinPath).append("geometry.geo.json").u8string(), geo, false);
        WriteAllFile(std::filesystem::path(skinPath).append("resource_patch.json").u8string(), mResourcePatch, false);
        __debugbreak();
    }
    MCAPI SerializedSkin(class SerializedSkin&&);
    MCAPI SerializedSkin();
    MCAPI SerializedSkin(class SerializedSkin const&);
    MCAPI SerializedSkin(class ConnectionRequest const&);
    MCAPI SerializedSkin(class SubClientConnectionRequest const&);
    MCAPI std::string const& getName() const;
    MCAPI bool isTrustedSkin() const;
    MCAPI class SerializedSkin& operator=(class SerializedSkin const&);
    MCAPI bool read(class ReadOnlyBinaryStream&);
    MCAPI void setIsTrustedSkin(bool);
    MCAPI void updateGeometryName();
    MCAPI void write(class BinaryStream&) const;
    MCAPI ~SerializedSkin();
};
static_assert(sizeof(SerializedSkin) == 616);
#include <MC/Packet.hpp>

class PlayerSkinPacket : public Packet
{

    mce::UUID mUUID;
    SerializedSkin mSkin;
    std::string mNewSkinName = "";
    std::string mOldSkinName = "";

public:
    inline static std::unique_ptr<PlayerSkinPacket> fromPlayer(class Player& player)
    {
        auto pkt = std::make_unique<PlayerSkinPacket>();
        pkt->mUUID = mce::UUID::fromString(player.getUuid());
        pkt->mSkin = player.getSkin();
        return std::move(pkt);
    }

    /*0*/ virtual ~PlayerSkinPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*4*/ virtual struct ExtendedStreamReadResult readExtended(class ReadOnlyBinaryStream&);
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);

    MCAPI PlayerSkinPacket();
};


class PlayerListEntry
{

public:
    ActorUniqueID uid;
    mce::UUID uuid;
    std::string name, xuid, platform_online_id;
    enum BuildPlatform platform;
    SerializedSkin skin;
    bool teacher, host;

public:
    MCAPI PlayerListEntry(class Player const&);
    MCAPI class PlayerListEntry clone() const;
    MCAPI bool read(class ReadOnlyBinaryStream&);
    MCAPI void write(class BinaryStream&) const;
    MCAPI ~PlayerListEntry();
};

enum class PlayerListPacketType
{
    Add,
    Remove
};


class PlayerListPacket : public Packet
{

public:
    std::vector<class PlayerListEntry> entries;
    enum PlayerListPacketType type;

public:
    /*0*/ virtual ~PlayerListPacket();
    /*1*/ virtual enum MinecraftPacketIds getId() const;
    /*2*/ virtual std::string getName() const;
    /*3*/ virtual void write(class BinaryStream&) const;
    /*4*/ virtual struct ExtendedStreamReadResult readExtended(class ReadOnlyBinaryStream&);
    /*6*/ virtual enum StreamReadResult _read(class ReadOnlyBinaryStream&);

    MCAPI PlayerListPacket(class mce::UUID const&);
    MCAPI PlayerListPacket();
    MCAPI void emplace(class PlayerListEntry&&);
};

namespace SkinHelper
{
bool init();
bool updatePlayerSkin(Player& player, SerializedSkin const& skin);
}; // namespace SkinHelper
#endif // SKIN_HELPER