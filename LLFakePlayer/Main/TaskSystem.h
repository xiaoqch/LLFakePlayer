#pragma once
#include <MC/SimulatedPlayer.hpp>
#include <MC/Level.hpp>
#include <MC/NavigationComponent.hpp>
#include <MC/BlockSource.hpp>
#include <MC/CommandUtils.hpp>
#include <MC/BlockActor.hpp>
#include <MC/Path.hpp>
#include <MC/BlockLegacy.hpp>
#include <MC/Block.hpp>
#include <MC/VanillaBlockTypes.hpp>
#include <MC/FishingHook.hpp>
#include <MC/SynchedActorData.hpp>
#include <MC/VanillaBlockTypeIds.hpp>
#include <MC/VanillaItemNames.hpp>
#include <MC/VanillaItemTags.hpp>
#include <MC/ItemRegistry.hpp>
#include <EventAPI.h>
#include <ScheduleAPI.h>

inline void RstripPath(Path& path, BlockSource& region, HashedString const& name = "", size_t len = 1)
{
    for (size_t index = 0; index < len; index++)
    {
        if (path.getSize() == 0)
            return;
        auto endPos = path.getEndPos();
        if (name == "" || region.getBlock(endPos).getName() == name)
            path.mNodes.pop_back();
        else
            return;
    }
}

class Task
{
public:
    enum class Type
    {
        Follow,
        Sleep,
        Use,
        Custom,
    };
    using CallbackFn = std::function<void(Task&)>;

    int mId;
    ActorUniqueID mPlayerUid;
    bool mWaitingRemoval = false;
    bool mSuccess = false;
    CallbackFn mCallback;
    std::string mErrorMessage;

    inline SimulatedPlayer* getPlayer() const
    {
        auto player = Global<Level>->getPlayer(mPlayerUid);
        if (player && player->isSimulatedPlayer())
            return static_cast<SimulatedPlayer*>(player);
        return nullptr;
    };
    static int getNextId()
    {
        static int id = 0;
        return id++;
    }
    template <typename... Args>
    inline void waitRemove(std::string const& format = "", Args&&... args)
    {
        if constexpr (sizeof...(args) > 0)
            mErrorMessage = fmt::format(format, std::forward<Args>(args)...);
        else
            mErrorMessage = format;
        mWaitingRemoval = true;
    }

public:
    virtual ~Task(){};
    virtual bool canUse()
    {
        if (mWaitingRemoval)
            return false;
        if (!getPlayer())
        {
            waitRemove("Player not found.");
            return false;
        }
        return true;
    }
    inline NavigationComponent const* getNavigationComponent() const
    {
        return SymCall("??$tryGetComponent@VNavigationComponent@@@Actor@@QEBAPEBVNavigationComponent@@XZ",
                       NavigationComponent const*, Actor*)(getPlayer());
    }
    virtual void start(){};
    virtual void tick() = 0;
    virtual Type getType() const = 0;
    virtual void stop(){};
    virtual void onPlayerChangeDimension(class Player*, class AutomaticID<class Dimension, int>){};
    virtual std::string toString() const
    {
        auto player = getPlayer();
        auto name = player ? player->getName() : std::string("<Invaild>");
        return fmt::format("Task<{}> - {} - {}", getName(), name, mId);
    };

    // virtual std::string toJson() const;
    // virtual void fromJson(const std::string& json);

    Task(ActorUniqueID actorId, CallbackFn callback = nullptr)
        : mPlayerUid(actorId)
        , mCallback(callback)
    {
        mId = getNextId();
    };
    std::string getName() const
    {
        return fmt::format("{}Task", magic_enum::enum_name(getType()));
    }


    template <typename T, typename... Args>
    static std::unique_ptr<Task> create(ActorUniqueID actorId, Args&&... args)
    {
        return std::make_unique<T>(actorId, std::forward<Args>(args)...);
    }
    Task(Task const&) = delete;
};

class FollowTask : public Task
{
    ActorUniqueID mTargetUid;
    float mSpeed = 1.0f;
    BlockPos mLastPos = BlockPos::MIN;
    std::vector<BlockPos> mWaitPositions;
    inline Actor* getTarget() const
    {
        return Global<Level>->fetchEntity(mTargetUid, false);
    }

public:
    virtual bool canUse()
    {
        if (!__super::canUse())
            return false;
        if (!getTarget())
        {
            waitRemove("Target not found.");
            return false;
        }
        return true;
    }
    virtual void tick() override
    {
        auto& sp = *getPlayer();
        auto& target = *getTarget();
        // if (!sp.isOnGround())
        //{
        //     if (sp.isInWater())
        //     {
        //         if (!sp.isSwimming())
        //             sp.startSwimming();
        //     }
        //     else if (sp.isSwimming())
        //         sp.stopSwimming();
        // }
        // else if (sp.isSwimming())
        //     sp.stopSwimming();

        auto naviComp = getNavigationComponent();

        if (!naviComp)
            return waitRemove("Can't get navigation component");
        if (!naviComp->isDone() && !naviComp->isStuck(100))
            return;
         else
             DEBUGL("{}: done: {}, stuck: {}",
                 CommandUtils::getActorName(sp), naviComp->isDone(), naviComp->isStuck(100));


        auto maxDist = naviComp->getMaxDistance(sp);
        auto start = sp.getPosition();
        auto end = target.getPosition();
        if (start.distanceToSqr(end) < maxDist * 0.25 * maxDist * 0.25)
            return;
        // LOG_VAR(maxDist);
        // auto _result = sp.simulateNavigateToEntity(target, 1.f);
        // if (!_result.mPath.empty())
        //{
        //      sendMessage("{} move to {}", CommandUtils::getActorName(sp), _result.mPath.back().toString());
        //      return;
        // }
        // sendMessage("You are too far away from {}", CommandUtils::getActorName(sp));

        auto nextPos = getNextPosition(start, end, maxDist * 0.5f);
        bool canToTarget = nextPos != end;
        if (canToTarget)
        {
            auto& region = sp.getRegion();
            nextPos.y = region.getHeightmap(nextPos);
        }
        sp.setSprinting(!canToTarget);
        auto result = sp.simulateNavigateToLocation(nextPos, 1.f);
        if (!result.mPath.empty())
        {
#ifdef DEBUG
            DEBUGL("{} move to {}", CommandUtils::getActorName(sp), result.mPath.back().toString());
#endif // DEBUG
            return;
        }
        waitRemove("{} can not reach {}, stop following", CommandUtils::getActorName(sp), CommandUtils::getActorName(target));
    }
    virtual Type getType() const override
    {
        return Type::Follow;
    }

    inline Vec3 getNextPosition(Vec3 const& start, Vec3 const& end, float distance)
    {
        auto dir = end - start;
        auto length = dir.length();
        if (length < distance)
            return end;
        auto off = dir * (distance / length);
        return start + off;
    }
    template <typename... Args>
    void sendMessage(std::string const& format, Args&&... args)
    {
        std::string msg;
        if constexpr (sizeof...(args) > 0)
            msg = fmt::format(format, std::forward<Args>(args)...);
        else
            msg = format;
        auto target = getTarget();
        if (target && target->isPlayer(false))
            static_cast<Player*>(target)->sendText(msg);
        else
            logger.warn(msg);
    }

    FollowTask(ActorUniqueID actorId, ActorUniqueID targetUid, CallbackFn callback = nullptr)
        : Task(actorId, callback)
        , mTargetUid(targetUid){};
};

class TradeTask : public Task
{
};

class BreakTask : public Task
{
};

class AttackTask : public Task
{
};

class CraftTask : public Task
{
};

#include <MC/Container.hpp>
#include <MC/ItemStack.hpp>
#include <MC/HitResult.hpp>
#include <MC/Item.hpp>

inline HitResult getBlockFromViewVectorEx(Actor& actor)
{
    float maxDistance = 5.25f;
    auto& bs = actor.getRegion();
    auto pos = actor.getCameraPos();
    auto viewVec = actor.getViewVector(1.0f);
    auto viewPos = pos + (viewVec * maxDistance);
    auto player = actor.isPlayer() ? (Player*)&actor : nullptr;
    int maxDisManhattan = (int)((maxDistance + 1) * 2);
    return bs.clip(pos, viewPos, true, false, maxDisManhattan, true, false, nullptr, BlockSource::ClipParameters::CHECK_ALL_BLOCKS);
}

class SyncHelper
{
    enum class State : unsigned char
    {
        Starting,
        Navigating,
        WaitMove,
        Moving,
        Looking,
        Stopped,
    };

    State mState = State::Starting;
    bool mNeedSyncPos = false;
    bool mNeedSyncView = false;
    bool mNeedSneak = false;
    Vec3 mStandingPos = Vec3::MIN;
    Vec3 mViewVector = Vec3::MIN;
    Vec3 mLastVec = Vec3::MIN;
    bool mHasError = false;
    std::string mErrorMessage = "";
    int mSleepingTicks = 0;

    template<typename ...Args>
    inline bool waitRemove(std::string const& format = "", Args&&... args)
    {
        if constexpr (sizeof...(args) > 0)
            mErrorMessage = fmt::format(format, std::forward<Args>(args)...);
        else
            mErrorMessage = format;
        mHasError = true;
        return false;
    }

    inline NavigationComponent const* getNavigationComponent(SimulatedPlayer& sp) const
    {
        return SymCall("??$tryGetComponent@VNavigationComponent@@@Actor@@QEBAPEBVNavigationComponent@@XZ",
                       NavigationComponent const*, Actor&)(sp);
    }

public:
    bool needSync()
    {
        return mNeedSyncPos && mNeedSyncView;
    }
    bool hasError()
    {
        return mHasError;
    }
    std::string const& getErrorMessage()
    {
        return mErrorMessage;
    }
    bool syncTick(SimulatedPlayer& sp)
    {
        if (mSleepingTicks)
        {
            --mSleepingTicks;
            return false;
        }
        switch (mState)
        {
            case SyncHelper::State::Starting:
                if (mNeedSyncPos)
                {
                    auto naviComp = getNavigationComponent(sp);
                    if (!naviComp)
                        return waitRemove("Can't get navigation component");
                    auto start = sp.getPosition();
                    auto end = mStandingPos;
                    auto maxDist = naviComp->getMaxDistance(sp);
                    auto result = sp.simulateNavigateToLocation(end, 1.f);
                    if (!result.mPath.empty())
                    {
                        DEBUGL("{} move to {}", CommandUtils::getActorName(sp), result.mPath.back().toString());
                        mState = State::Navigating;
                        return false;
                    }
                    else
                        return waitRemove("{} can not navigate to {}, stop syncing",
                                          CommandUtils::getActorName(sp), mStandingPos.toString());
                    return false;
                }
                else if (mNeedSyncView)
                {
                    auto& pos = sp.getPosition();
                    sp.simulateLookAt(pos + (mViewVector * 1024));
                    mLastVec = Vec3::MIN;
                    mState = State::Looking;
                    return false;
                }
                else
                {
                    mState = State::Stopped;
                    return true;
                }
                break;
            case SyncHelper::State::Navigating:
            {

                auto naviComp = getNavigationComponent(sp);
                if (!naviComp)
                    return waitRemove("Can't get navigation component");
                if (!naviComp->isDone() && !naviComp->isStuck(100))
                    return false;
                else
                    DEBUGL("{}: done: {}, stuck: {}",
                           CommandUtils::getActorName(sp), naviComp->isDone(), naviComp->isStuck(100));
                auto feetPos = CommandUtils::getFeetPos(&sp);
                DEBUGW("navigating stop, distance: {}", feetPos.distanceTo(mStandingPos));
                if (feetPos.distanceToSqr(mStandingPos) < 2.0f)
                {
                    sp.simulateStopMoving();
                    mState = State::WaitMove;
                    mSleepingTicks = 5;
                    return false;
                }
                else
                    return waitRemove("{} is too far from {}, stop syncing",
                                      CommandUtils::getActorName(sp), mStandingPos.toString());
                break;
            }
            case SyncHelper::State::WaitMove:
                DEBUGL("{} start move to {}, speed: {}", CommandUtils::getActorName(sp), mViewVector.toString(), sp.getSpeed());
                sp.simulateMoveToLocation(mStandingPos + Vec3{0.f, 1.6f, 0.f}, .01f);
                mLastVec = sp.getPos();
                mState = State::Moving;
                return false;
                break;
            case SyncHelper::State::Moving:
            {

                auto& pos = sp.getPosition();
                if (mLastVec != pos)
                {
                    mLastVec = pos;
                    return false;
                }
                auto feetPos = CommandUtils::getFeetPos(&sp);

                DEBUGW("moving stop, distance: {}", feetPos.distanceTo(mStandingPos));
                if (feetPos.distanceToSqr(mStandingPos) < 1)
                {
                    sp.simulateStopSneaking();
                    sp.simulateStopMoving();
                    if (mNeedSyncView)
                    {
                        mState = State::Looking;
                        sp.simulateLookAt(pos + (mViewVector * 1024));
                        mLastVec = Vec3::MIN;
                        return false;
                    }
                    mState = State::Stopped;
                    return true;
                }
                if (feetPos.y < mStandingPos.y)
                {
                    sp.simulateJump();
                    return false;
                }
                //sp.simulateMoveToLocation(mStandingPos, .01f);
                //return false;
                return waitRemove("{} can't move to {}, stop syncing",
                                  CommandUtils::getActorName(sp), mStandingPos.toString());
                break;
            }
            case SyncHelper::State::Looking:
            {
                auto viewVector = sp.getViewVector(1.f);
                if (mLastVec != viewVector)
                {
                    mLastVec = viewVector;
                    return false;
                }
                if (viewVector.dot(mViewVector) > 0.9f)
                {
                    mState = State::Stopped;
                    DEBUGW("{} look at {}", CommandUtils::getActorName(sp), mViewVector.toString());
                    return true;
                }
                else
                    return waitRemove("{} is not looking at {}, stop syncing", CommandUtils::getActorName(sp), mViewVector.toString());
                break;
            }
            case SyncHelper::State::Stopped:
                return true;
                break;
            default:
                break;
        }
        __debugbreak();
    }
    bool updateTarget(Actor& actor)
    {
        if (mNeedSyncPos)
            mStandingPos = CommandUtils::getFeetPos(&actor);
        if (mNeedSyncView) {
            mViewVector = actor.getViewVector(.1f);
            mNeedSneak = actor.isSneaking();
        }
        return true;
    }
    SyncHelper(Actor& actor, bool syncPos = true, bool syncView = true)
        : mNeedSyncPos(syncPos)
        , mNeedSyncView(syncView)
    {
        updateTarget(actor);
    }
    SyncHelper(){};
};

class UseTask : public Task
{
    bool mNeedSync = true;
    SyncHelper mSyncHelper;
    std::set<HashedString> mItemNames;
    std::set<int> mSlots;
    int mWaitUseingDuration = 0;
    int mCooldown = static_cast<int>(std::min(Config::DefaultMaxCooldownTicks, 10ull));
    int mSleepingTicks = 0;
    size_t mUseTimes = 0;
    float mLastFishingAngle= 0.0f;
    virtual Type getType() const override
    {
        return Type::Use;
    }
    virtual bool canUse() override
    {
        if (!__super::canUse())
            return false;
        if (mItemNames.empty())
        {
            waitRemove("No item to use");
            return false;
        }
        return true;
    }
    virtual void tick() override
    {
        if (mSleepingTicks) {
            mSleepingTicks--;
            return;
        }
        auto& sp = *getPlayer();
        if (mNeedSync)
        {
            mNeedSync = !mSyncHelper.syncTick(sp);
            if (mSyncHelper.hasError())
                return waitRemove(mSyncHelper.getErrorMessage());
            return;
        }
        DEBUGL("isUsingItem: {}", sp.isUsingItem());
        if (mWaitUseingDuration)
        {
            DEBUGW("using: {}/{}", sp.getItemUseDuration(), mWaitUseingDuration);
            if (mWaitUseingDuration == 72000) {
                if (mWaitUseingDuration - sp.getItemUseDuration() > 20) {
                    sp.releaseUsingItem();
                    mWaitUseingDuration = 0;
                    return;
                }
            }
            else if (sp.getItemUseDuration() == 0)
            {
                sp.simulateStopUsingItem();
                mWaitUseingDuration = 0;
                DEBUGL("{} stop using", CommandUtils::getActorName(sp));
            }
            return;
        }
        auto& inventory = sp.getInventory();
        if (mSlots.empty())
        {
            for (int i = 0; i < inventory.getContainerSize(); i++)
            {
                auto& item = inventory.getItem(i);
                if (item.isNull())
                    continue;
                auto name = item.getTypeName();
                if (mItemNames.find(name) != mItemNames.end())
                    mSlots.insert(i);
            }
            if (mSlots.empty())
                return waitRemove("{} has no item to use, used times: {}",
                    CommandUtils::getActorName(sp), mUseTimes);
        }
        auto slot = *mSlots.begin();
        auto& item = inventory.getItem(slot);
        if (item.isNull() || mItemNames.find(item.getTypeName()) == mItemNames.end())
        {
            mSlots.erase(slot);
            return;
        }
        if (item.getFullNameHash() == VanillaItemNames::FishingRod)
        {
            return tickFishing(sp, slot);
        }
        bool res;
        mWaitUseingDuration = item.getMaxUseDuration();
        if (item.getItem()->isUseable() || mWaitUseingDuration)
        {
            DEBUGL("{} is usable", item.getName());
            res = sp.simulateUseItemInSlot(slot);
        }
        else
        {
            DEBUGL("{} is unusable", item.getName());
            auto hitResult = getBlockFromViewVectorEx(sp);
            if (hitResult.isHit()||hitResult.isHitLiquid()) {
                res = sp.simulateUseItemInSlotOnBlock(slot, hitResult.getLiquidPos(), (ScriptFacing)hitResult.getFacing(), dAccess<Vec3>(&hitResult, 96));
                DEBUGL("Use on liquid: {}", res);
                res = res || sp.simulateUseItemInSlotOnBlock(slot, hitResult.getBlockPos(), (ScriptFacing)hitResult.getFacing(), hitResult.getPos());
            }
            else
            {
                return waitRemove("{} can not use {}, used times: {}",
                    CommandUtils::getActorName(sp), item.getName(), mUseTimes);
            }
        }
        DEBUGL("{} use item in slot {}, result: {}, duration: {}, used times: {}",
               CommandUtils::getActorName(sp), slot, res, mWaitUseingDuration, mUseTimes);
        if (res)
        {
            mUseTimes++;
            mSleepingTicks = mCooldown;
        }
        else
        {
            return waitRemove("Can't use item, used times: {}", mUseTimes);
        }

    }
    virtual void stop()
    {
        auto sp = getPlayer();
        if (sp)
        {
            sp->simulateStopUsingItem();
            sp->simulateStopMoving();
        }
    }
    
    void tickFishing(SimulatedPlayer& sp, int slot)
    {
        DEBUGL("Using fishing rod");
        auto res = false;
        if (sp.hasFishingHook())
        {
            auto hook = sp.fetchFishingHook();
            if (!hook)
                return;
            auto targetId = hook->getTargetId();
            if (targetId != 0 && targetId != -1)
            {
                res = sp.simulateUseItemInSlot(slot);
            }
            else if (hook->_getWaterPercentage() > 0.f)
            {
                auto fishAngle = hook->getEntityData().getFloat(ActorDataKeys::FISH_ANGLE);
#ifdef DEBUG
                auto unk1808 = dAccess<int>(hook, 1808); // left time
                auto unk1812 = dAccess<int>(hook, 1812); // wait time
                auto unk1816 = dAccess<int>(hook, 1816);
                auto unk1820 = dAccess<int>(hook, 1820);
                DEBUGL("{} fish angle: {}, unk1808: {}, unk1812: {}, unk1816: {}, unk1820: {}",
                       CommandUtils::getActorName(sp), fishAngle, unk1808, unk1812, unk1816, unk1820);
#endif // DEBUG=
                if (fishAngle == mLastFishingAngle)
                {
                    if (fishAngle != 0.f)
                    {
                        // ASSERT(unk1808 <= 0 && unk1816 > 0);
                        // fish is hooked
                        res = sp.simulateUseItemInSlot(slot);
                    }
                    else
                    {
                        // wait for the fish to appear
                        mSleepingTicks = 20;
                        return;
                    }
                }
                else
                {
                    // wait for the fish to take the bait
                    mLastFishingAngle = fishAngle;
                    mSleepingTicks = 5;
                    return;
                }
            }
            else if (!hook->isMoving())
            {
                res = sp.simulateUseItemInSlot(slot);
            }
            else
            {
                mSleepingTicks = 5;
                return;
            }
        }
        else
        {
            res = sp.simulateUseItemInSlot(slot);
        }
        if (res)
        {
            mUseTimes++;
            mSleepingTicks = mCooldown;
        }
        else
        {
            return waitRemove("Can't use item, used times: {}", mUseTimes);
        }
        return;
    }

public:
    UseTask(ActorUniqueID actorId, CallbackFn callback = nullptr, std::set<std::string> const& itemNames = {}, SyncHelper syncHelper = {})
        : Task(actorId, callback)
        , mSyncHelper(syncHelper)
    {
        for (auto& name : itemNames)
        {
            auto fullName = name.find(":") == std::string::npos ? "minecraft:" + name : name;
            mItemNames.insert(fullName);
        }
        if (mItemNames.empty()) {
            auto player = getPlayer();
            if (player) {
                auto& item = player->getSelectedItem();
                if (!item.isNull()) {
                    mItemNames.insert(item.getFullNameHash());
                }
            }
        }
        mNeedSync = syncHelper.needSync();
    };
};

class NavigateTask : public Task
{
};

enum BlockActorType;
#include <Utils/PlayerMap.h>
class SleepTask : public Task
{
    std::vector<BlockPos> blockPosList;
    bool sorted = false;
    int mTryingIndex = -1;
    Vec3 mTryingPos = Vec3::MIN;
    bool mMoving = false;
    bool mLooking = false;
    Vec3 mLastViewVec = Vec3::ZERO;

public:

    virtual void tick() override
    {
        auto naviComp = getNavigationComponent();
        if (!naviComp)
            return waitRemove("Can't get navigation component");
        auto sp = getPlayer();
        auto maxDist = naviComp->getMaxDistance(*sp);
        auto& pos = sp->getPosition();
        if (blockPosList.empty())
        {
            auto aa = pos - maxDist;
            auto bb = pos + maxDist;

            auto bas = sp->getRegion().fetchBlockEntities((enum BlockActorType)27, AABB{aa, bb});
            if (bas.empty())
                return waitRemove("Can't found bed");
            for (auto ba : bas)
            {
                blockPosList.push_back(ba->getPosition());
            }
            return;
        }
        if (!sorted)
        {
            std::sort(blockPosList.begin(), blockPosList.end(), [&](BlockPos const& _Left, BlockPos const& _Right) {
                return pos.distanceToSqr(_Right.center()) - pos.distanceToSqr(_Left.center());
            });
            sorted = true;
            return;
        }
        if (!naviComp->isDone())
            return;
        if (!mMoving && !mLooking)
        {
            ++mTryingIndex;
            if (mTryingIndex >= blockPosList.size())
                return waitRemove("can't reach bed");
            mTryingPos = blockPosList[mTryingIndex].center();
            auto res = sp->simulateNavigateToLocation(mTryingPos, 1.0f);
            auto path = naviComp->getPath();
            if (path)
                RstripPath(*path, sp->getRegion(), VanillaBlockTypeIds::Bed, 2);
            mMoving = true;
            DEBUGL("start move to {}", mTryingPos.toString());
        }
        else if (mMoving)
        {
            mMoving = false;
            sp->simulateLookAt(mTryingPos);
            mLooking = true;
            DEBUGL("start look at {}", mTryingPos.toString());
        }
        else if (mLooking)
        {
            auto viewVec = sp->getViewVector(1.0f);
            if (mLastViewVec != viewVec)
            {
                mLastViewVec = viewVec;
                return;
            }
            mLooking = false;
            auto res = sp->simulateInteract();
            DEBUGL("simulateInteract -> {}", res);
            if (sp->isSleeping())
            {
                DEBUGW("now, player is sleeping");
                mSuccess = true;
                waitRemove();
            }
            else
                DEBUGW("but is not sleeping");
        }
    }
    virtual Type getType() const override
    {
        return Task::Type::Sleep;
    }
    SleepTask(ActorUniqueID actorId, CallbackFn callback = nullptr)
        : Task(actorId, std::move(callback)){};
};

class TaskSystem
{
public:
    std::vector<std::unique_ptr<Task>> mTasks;
    std::vector<std::unique_ptr<Task>> mPendingTasks;

    int mGlobalTaskId = 0;
    Tick mLastSleepTick = 0;
    std::optional<ScheduleTask> mScheduleTask = std::nullopt;

    int addTask(std::unique_ptr<Task>&& task)
    {
        if (!mScheduleTask)
        {
            mScheduleTask = Schedule::repeat([&]() { tick(); }, 1);
        }
        auto id = mGlobalTaskId++;
        task->mId = id;
        mPendingTasks.push_back(std::move(task));
        return id;
    }
    void removeTask(int id)
    {
        for (auto it = mTasks.begin(); it != mTasks.end(); ++it)
        {
            if ((*it)->mId == id)
            {
                mTasks.erase(it);
                return;
            }
        }
    }
    void removeTask(Task* task)
    {
        removeTask(task->mId);
    }
    void tick()
    {
        mTasks.insert(mTasks.end(), std::make_move_iterator(mPendingTasks.begin()), std::make_move_iterator(mPendingTasks.end()));
        mPendingTasks.clear();
        mTasks.erase(std::remove_if(mTasks.begin(), mTasks.end(), [](auto& task) {
                         if (!task->mWaitingRemoval && task->canUse())
                             return false;
                         if (task->mCallback)
                             task->mCallback(*task);
                         return true;
                     }),
                     mTasks.end());
        for (auto& task : mTasks)
            task->tick();
        mTasks.erase(std::remove_if(mTasks.begin(), mTasks.end(), [](auto& task) {
                         if (!task->mWaitingRemoval)
                             return false;
                         task->stop();
                         if (task->mCallback)
                             task->mCallback(*task);
                         return true;
                     }),
                     mTasks.end());
    }
    void onPlayerChangeDimension(Player* player, int toDimension)
    {
        for (auto& task : mTasks)
            task->onPlayerChangeDimension(player, toDimension);
    }
    std::vector<Task*> findTasks(Player& sp)
    {
        std::vector<Task*> tasks;
        auto uid = sp.getUniqueID();
        for (auto& task : mTasks)
        {
            if (task->mPlayerUid == uid)
                tasks.push_back(task.get());
        }
        return tasks;
    }
    std::vector<Task*> findTasks(Player& sp, Task::Type type)
    {
        std::vector<Task*> tasks;
        auto uid = sp.getUniqueID();
        for (auto& task : mTasks)
        {
            if (task->getType() == type && task->mPlayerUid == uid)
                tasks.push_back(task.get());
        }
        return tasks;
    }

    void onPlayerSleep(Player* player)
    {
        if (player && player->isSimulatedPlayer())
            return;
        auto currentTick = Global<Level>->getCurrentServerTick();
        if (currentTick - mLastSleepTick > 4ull /** 60 * 20*/)
        {
            Global<Level>->forEachPlayer([&](Player& player) {
                if (player.isSimulatedPlayer() && player.getDimensionId() == 0 && findTasks(player).empty())
                    addTask(std::make_unique<SleepTask>(player.getUniqueID(), [](Task& task) {
                        if (!task.mSuccess)
                        {
                            auto sp = Global<Level>->getPlayer(task.mPlayerUid);
                            if (sp)
                                sp->talkAs(fmt::format("I can't sleep. {}", task.mErrorMessage));
                        }
                    }));
                return true;
            });
            mLastSleepTick = currentTick;
        }
    }
    static TaskSystem& getInstance()
    {
        static TaskSystem taskSystem;
        return taskSystem;
    }
};
