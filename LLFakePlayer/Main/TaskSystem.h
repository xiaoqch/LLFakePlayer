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
#include <MC/VanillaBlockTypeIds.hpp>
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
        Custom,
        Cancel,
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
    virtual void tick() = 0;
    virtual Type getType() const = 0;
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
            return false;
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
        // auto _result = sp.simulateNavigateToEntity(target, 99.0f);
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
        auto result = sp.simulateNavigateToLocation(nextPos, 99.0f);
        if (!result.mPath.empty())
        {
#ifdef DEBUG
            sendMessage("{} move to {}", CommandUtils::getActorName(sp), result.mPath.back().toString());
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

class NavigateTask : public Task
{
};
enum BlockActorType;
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
