#include <pch.h>
#include <MC/SimulatedPlayer.hpp>

// Fake SimulatedPlayer Structure
constexpr size_t ServerPlayerSize = 9584;
constexpr size_t SimulatedPlayerSize = 9832;

using _QWORD = unsigned __int64;
using _DWORD = unsigned int;
using _WORD = unsigned short;
using _BYTE = unsigned char;
struct PlayerMovementSettings
{
    _WORD unk9320 = 1;      // 9320 - size = 104
    _DWORD unk9324 = 0i64;  // 9324
    _DWORD unk9328 = 0i64;  // 9324
    _DWORD unk9332 = 0i64;  // 9332
    _DWORD unk9336 = 0i64;  // 9332
    _QWORD unk9344 = 0i64;  // 9344
    _QWORD unk9352 = 0i64;  // 9352
    _QWORD unk9360 = 0i64;  // 9360
    _QWORD unk9368 = 0i64;  // 9368
    _QWORD unk9376 = 0i64;  // 9376
    _BYTE unk9384 = 0;      // 9384
    _DWORD unk9388 = 0i64;  // 9388
    _DWORD unk9392 = 0i64;  // 9392
    _DWORD unk9396 = 0i64;  // 9396
    _WORD unk9400 = 0x6400; // 9400
    _QWORD unk9408 = 40i64; // 9408
    _BYTE unk9416 = 1;      // 9416
    _DWORD unk9420 = 0;     // 9420
    bool operator==(PlayerMovementSettings const& right) const
    {
        return unk9320 == right.unk9320 && unk9324 == right.unk9324 && unk9328 == right.unk9328 && unk9332 == right.unk9332 && unk9336 == right.unk9336 && unk9344 == right.unk9344 && unk9352 == right.unk9352 && unk9360 == right.unk9360 && unk9368 == right.unk9368 && unk9376 == right.unk9376 && unk9384 == right.unk9384 && unk9388 == right.unk9388 && unk9392 == right.unk9392 && unk9396 == right.unk9396 && unk9400 == right.unk9400 && unk9408 == right.unk9408 && unk9416 == right.unk9416 && unk9420 == right.unk9420;
    };
    bool operator!=(PlayerMovementSettings const& right) const
    {
        return !(*this == right);
    }
};
#ifdef DEBUG
static_assert(sizeof(PlayerMovementSettings) == 104);

template <>
struct fmt::formatter<Vec3>
{
    // Presentation format: 'f' - fixed, 'e' - exponential.
    char presentation = 'f';

    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        // [ctx.begin(), ctx.end()) is a character range that contains a part of
        // the format string starting from the format specifications to be parsed,
        // e.g. in
        //
        //   fmt::format("{:f} - point of interest", point{1, 2});
        //
        // the range will contain "f} - point of interest". The formatter should
        // parse specifiers until '}' or the end of the range. In this example
        // the formatter should parse the 'f' specifier and return an iterator
        // pointing to '}'.

        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

        // Check if reached the end of the range:
        if (it != end && *it != '}') throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const Vec3& p, FormatContext& ctx) -> decltype(ctx.out())
    {
        // ctx.out() is an output iterator to write to.
        return presentation == 'f'
                   ? fmt::v8::format_to(ctx.out(), "({:.1f}, {:.1f}, {:.1f})", p.x, p.y, p.z)
                   : fmt::v8::format_to(ctx.out(), "({:.1e}, {:.1e}, {:.1e})", p.x, p.y, p.z);
    }
};
template <>
struct fmt::formatter<BlockPos>
{

    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        auto it = ctx.begin(), end = ctx.end();
        return it;
    }

    template <typename FormatContext>
    auto format(const BlockPos& p, FormatContext& ctx) -> decltype(ctx.out())
    {
        return fmt::v8::format_to(ctx.out(), "({}, {}, {})", p.x, p.y, p.z);
    }
};
template <>
struct fmt::formatter<Tick>
{
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return fmt::formatter<long long>().parse(ctx);
    }
    template <typename FormatContext>
    auto format(const Tick& p, FormatContext& ctx) -> decltype(ctx.out())
    {
        return fmt::formatter<long long>().format(p.t, ctx);
    }
};
template <typename T>
struct fmt::formatter<std::vector<T>>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        auto it = ctx.begin(), end = ctx.end();
        return it;
    }
    template <typename FormatContext>
    auto format(const std::vector<T>& p, FormatContext& ctx) -> decltype(ctx.out())
    {
        if (p.size() == 0)
            return fmt::v8::format_to(ctx.out(), "[]");
        fmt::v8::format_to(ctx.out(), "[");
        fmt::v8::format_to(ctx.out(), "{}", *p.begin());
        for (size_t index = 1; index < p.size(); ++index)
            fmt::v8::format_to(ctx.out(), ",{}", p[index]);
        return fmt::v8::format_to(ctx.out(), "]");
    }
};

enum class SimulatedMovingType
{
    Local,
    World,
};

#endif // DEBUG
#include <MC/NavigationComponent.hpp>
class FakeSimulatedPlayer
{
    FakeSimulatedPlayer() = delete;

public:
    char player[ServerPlayerSize];

    bool mIsLocalMoving;                           // 9584 _updateMovement
    Vec3 mMovingDirection;                         // 9588 _updateMovement
    bool mIsMoving;                                // 9600 _updateMovement,
    Vec3 mMovingPosition;                          // 9604 _updateMovement
    bool mIsMovingTo;                              // 9616 _updateMovement
    Vec3 mLookingTargetPosition;                   // 9620 _updateMovement
    bool mIsLookingAt;                             // 9632 _updateMovement
    BlockPos mBreakingBlockPos;                    // 9636 simulateStopDestroyingBlock
    bool mIsBreakingBlock;                         // 9648 simulateStopDestroyingBlock
    char filler65[3];                              //
    FaceID mBreakingFace;                          // 9652 aiStep, simulateDestroyBlock
    _BYTE unk69;                                   // 9653 simulateStopDestroyingBlock,
    std::vector<Vec3> mNavigateTargets;            // 9656 simulateNavigateToLocations
    bool mIsToTargets;                             // 9680 simulateNavigateToLocations
    size_t mTargetIndex;                           // 9688 _updateMovement
    std::shared_ptr<void*> mGametestHelper;        // 9696 getGameTestHelper
    /*Tick*/ size_t mLastCooldownTick;             // 9712, cooldown = 20 ticks, attack and use
    PlayerMovementSettings mMovementSettings = {}; // 9720
    float mOldY = -FLT_MAX;                        // 9824 aiStep
    float mInputSpeed;                             // 9828 _getInputSpeed

#ifdef DEBUG
    inline void breakIfStateChanged()
    {
        return;
        constexpr size_t off = sizeof(FakeSimulatedPlayer);
        // return;
#define ListenAndLog(val)                                                             \
    static auto _##val = val;                                                         \
    if (_##val != val) logger.error("[ValueChange] {}: {} -> {}", #val, _##val, val); \
    _##val = val;
#define ListenAndBreak(val)            \
    static auto _##val = val;          \
    if (_##val != val) __debugbreak(); \
    _##val = val;
        ListenAndLog(mIsLocalMoving);
        ListenAndLog(mMovingDirection);
        ListenAndLog(mIsMoving);
        ListenAndLog(mMovingPosition);
        ListenAndLog(mIsMovingTo);
        ListenAndLog(mLookingTargetPosition);
        ListenAndLog(mIsLookingAt);
        ListenAndLog(mBreakingBlockPos);
        ListenAndLog(mIsBreakingBlock);
        ListenAndLog(filler65);
        ListenAndLog(mBreakingFace);
        ListenAndLog(unk69);
        ListenAndLog(mNavigateTargets);
        ListenAndLog(mIsToTargets);
        ListenAndLog(mTargetIndex);
        ListenAndBreak(mGametestHelper);
        ListenAndLog(mLastCooldownTick);
        ListenAndBreak(mMovementSettings);
        ListenAndLog(mOldY);
        ListenAndLog(mInputSpeed);
    }
#endif // DEBUG
    inline NavigationComponent const* getNavigationComponent() const
    {
        return SymCall("??$tryGetComponent@VNavigationComponent@@@Actor@@QEBAPEBVNavigationComponent@@XZ",
                       NavigationComponent const*, const void*)(this);
    }
    inline static FakeSimulatedPlayer* from(Player* player)
    {
        return reinterpret_cast<FakeSimulatedPlayer*>(player);
    }
    inline static FakeSimulatedPlayer& from(Player& player)
    {
        return reinterpret_cast<FakeSimulatedPlayer&>(player);
    }
    inline SimulatedPlayer& to()
    {
        return *reinterpret_cast<SimulatedPlayer*>(player);
    }
};

template<>
struct std::hash<ActorUniqueID>
{
    size_t operator()(ActorUniqueID const& id) const
    {
        return id.id;
    }
};

#include <MC/CommandUtils.hpp>
#include <MC/BlockSource.hpp>

void tickFakeSimulatedPlayer(SimulatedPlayer& sp)
{
    auto& fsp = FakeSimulatedPlayer::from(sp);
#ifdef DEBUG
    if (sp.getUserEntityIdentifierComponent()->isPrimaryClient())
        fsp.breakIfStateChanged();
#endif // DEBUG
    auto currentServerTick = Global<Level>->getCurrentServerTick();
    if (currentServerTick + Config::DefaultMaxCooldownTicks < fsp.mLastCooldownTick)
        fsp.mLastCooldownTick = currentServerTick + Config::DefaultMaxCooldownTicks;

}

static_assert(sizeof(FakeSimulatedPlayer) == SimulatedPlayerSize);
static_assert(offsetof(FakeSimulatedPlayer, mIsLocalMoving) == ServerPlayerSize + 0);
static_assert(offsetof(FakeSimulatedPlayer, mMovingDirection) == ServerPlayerSize + 4);
static_assert(offsetof(FakeSimulatedPlayer, mIsMoving) == ServerPlayerSize + 16);
static_assert(offsetof(FakeSimulatedPlayer, mMovingPosition) == ServerPlayerSize + 20);
static_assert(offsetof(FakeSimulatedPlayer, mIsMovingTo) == ServerPlayerSize + 32);
static_assert(offsetof(FakeSimulatedPlayer, mLookingTargetPosition) == ServerPlayerSize + 36);
static_assert(offsetof(FakeSimulatedPlayer, mIsLookingAt) == ServerPlayerSize + 48);
static_assert(offsetof(FakeSimulatedPlayer, mBreakingBlockPos) == ServerPlayerSize + 52);
static_assert(offsetof(FakeSimulatedPlayer, mIsBreakingBlock) == ServerPlayerSize + 64);
static_assert(offsetof(FakeSimulatedPlayer, filler65) == ServerPlayerSize + 65);
static_assert(offsetof(FakeSimulatedPlayer, mBreakingFace) == ServerPlayerSize + 68);
static_assert(offsetof(FakeSimulatedPlayer, unk69) == ServerPlayerSize + 69);
static_assert(offsetof(FakeSimulatedPlayer, mNavigateTargets) == ServerPlayerSize + 72);
static_assert(offsetof(FakeSimulatedPlayer, mIsToTargets) == ServerPlayerSize + 96);
static_assert(offsetof(FakeSimulatedPlayer, mTargetIndex) == ServerPlayerSize + 104);
static_assert(offsetof(FakeSimulatedPlayer, mGametestHelper) == ServerPlayerSize + 112);
static_assert(offsetof(FakeSimulatedPlayer, mLastCooldownTick) == ServerPlayerSize + 128);
static_assert(offsetof(FakeSimulatedPlayer, mMovementSettings) == ServerPlayerSize + 136);
static_assert(offsetof(FakeSimulatedPlayer, mOldY) == ServerPlayerSize + 240);
static_assert(offsetof(FakeSimulatedPlayer, mInputSpeed) == ServerPlayerSize + 244);

