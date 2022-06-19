#include "pch.h"
#include "CommandHelper.h"
#include "TaskCommand.h"
#include "Header/FakePlayerManager.h"
#include <MC/ItemRegistry.hpp>
#include <MC/Item.hpp>

#define FULL_COMMAND_NAME "llfakeplayertask"
using Action = TaskCommand::Action;
template<>
void TaskCommand::executeAction<Action::Follow>(CommandOrigin const& origin, CommandOutput& output, SimulatedPlayer& sp) const
{
    auto& taskSystem = TaskSystem::getInstance();
    auto tasks = taskSystem.findTasks(sp, Task::Type::Follow);
    for (auto& task : tasks) {
        task->waitRemove("Follow task for {} auto cancel by command", sp.getName());
    }
    if (tasks.size())
        return output.success("Cancel exists task");
    auto target = getUniqueTarget(origin, output);
    if (!target)
        return;
    if (sp.getUniqueID() == target->getUniqueID())
        return output.error("Can't follow self");
    auto task = std::make_unique<FollowTask>(sp.getUniqueID(), target->getUniqueID(), delaySendMessage(origin));
    taskSystem.addTask(std::move(task));
    output.success("follow task added");
}

template<>
void TaskCommand::executeAction<Action::Sleep>(CommandOrigin const& origin, CommandOutput& output, SimulatedPlayer& sp) const
{
    auto& taskSystem = TaskSystem::getInstance();
    auto tasks = taskSystem.findTasks(sp, Task::Type::Sleep);
    for (auto& task : tasks)
    {
        task->waitRemove("Sleep task for {} auto cancel by command", sp.getName());
    }
    if (tasks.size())
        return output.success("Cancel exists task");
    auto task = std::make_unique<SleepTask>(sp.getUniqueID(), delaySendMessage(origin));
    taskSystem.addTask(std::move(task));
    output.success("Sleep task added");
}

template<>
void TaskCommand::executeAction<Action::Use>(CommandOrigin const& origin, CommandOutput& output, SimulatedPlayer& sp) const
{
    auto& taskSystem = TaskSystem::getInstance();
    auto tasks = taskSystem.findTasks(sp, Task::Type::Use);
    for (auto& task : tasks)
    {
        task->waitRemove("Use task for {} auto cancel by command", sp.getName());
    }
    if (tasks.size())
        return output.success("Cancel exists task");
    std::set<std::string> names;
    if (mItem_isSet)
    {
        auto id = mItem.getId();
        auto item = ItemRegistry::getItem(id);
        if (item)
            names.insert(item.get()->getFullNameHash().getString());
    }
    auto task = std::make_unique<UseTask>(sp.getUniqueID(), delaySendMessage(origin), names);
    taskSystem.addTask(std::move(task));
    output.success("Use task added");
}

template<>
void TaskCommand::executeAction<Action::Sync>(CommandOrigin const& origin, CommandOutput& output, SimulatedPlayer& sp) const
{
    auto& taskSystem = TaskSystem::getInstance();
    auto tasks = taskSystem.findTasks(sp, Task::Type::Sync);
    for (auto& task : tasks)
    {
        task->waitRemove("Sync task for {} auto cancel by command", sp.getName());
    }
    if (tasks.size())
        return output.success("Cancel exists task");
    bool syncPos = mSyncPosition_isSet ? mSyncPosition : true;
    bool syncView = mSyncView_isSet ? mSyncView : true;
    auto actor = origin.getEntity();
    if (!actor)
        return output.error("Sync command action must be executed by entity or player");
    auto task = std::make_unique<SyncTask>(sp.getUniqueID(), delaySendMessage(origin), *actor, syncPos, syncView);
    taskSystem.addTask(std::move(task));
    output.success("Use task added");
}

template<>
void TaskCommand::executeAction<Action::List>(CommandOrigin const& origin, CommandOutput& output, SimulatedPlayer& sp) const
{
    auto& taskSystem = TaskSystem::getInstance();
    auto tasks = taskSystem.findTasks(sp);
    if (tasks.empty())
        return output.errorf("Player {} has not any task", sp.getName());
    output.successf("player {} has {} task(s):", sp.getName(), tasks.size());
    for (auto& task : tasks) {
        output.success(task->toString());
    }
}

template<>
void TaskCommand::executeAction<Action::Cancel>(CommandOrigin const& origin, CommandOutput& output, SimulatedPlayer& sp) const
{
    auto& taskSystem = TaskSystem::getInstance();
    auto tasks = taskSystem.findTasks(sp);
    if (tasks.empty())
        return output.errorf("Player {} has not any task", sp.getName());
    output.successf("player {} has {} tasks", sp.getName(), tasks.size());
    for (auto& task : tasks) {
        output.success(task->toString());
    }
}

Actor* TaskCommand::getUniqueTarget(CommandOrigin const& origin, CommandOutput& output) const
{
    if (mTarget_isSet)
    {
        auto result = mTarget.results(origin);
        if (result.empty())
        {
            output.error("No target found");
            return nullptr;
        }
        if (result.count() > 1)
        {
            output.error("Too many target");
            return nullptr;
        }
        return *result.begin();
    }
    else
    {
        auto entity = origin.getEntity();
        if (!entity)
            output.error("No target found");
        return entity;
    }
}

Task::CallbackFn TaskCommand::delaySendMessage(CommandOrigin const& origin, std::string const& msg) const
{
    auto ori = origin.getPlayer();
    auto uid = ori ? ori->getUniqueID() : ActorUniqueID(-1);
    return [uid](Task& task) {
        if (!task.mSuccess)
        {
            if (auto player = Global<Level>->getPlayer(uid))
                player->sendText(task.mErrorMessage);
            else
                logger.warn(task.mErrorMessage);
        }
    };
}
#include <MC/VanillaBlocks.hpp>
void TaskCommand::execute(CommandOrigin const& origin, CommandOutput& output) const
{
#if 0
    int y = 66;
    auto& region = origin.getEntity()->getRegion();
    auto& stillWaterBlock = VanillaBlocks::mStillWater;
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            BlockPos bpos(x*2, y, z*2);
            auto flags = x + 16 * z;
            region.setExtraBlock(bpos, *VanillaBlocks::mStillWater, flags);
        }
    }
    return;
#endif // 0

    std::vector<SimulatedPlayer*> result;
    for (auto& player : mOperator.results(origin))
        if (player->isSimulatedPlayer())
            result.push_back(static_cast<SimulatedPlayer*>(player));
    if (result.empty())
        return output.error("No players found");
    for (auto& player : result)
    {
#define CaseAndBreak(action)                                                 \
    case TaskCommand::Action::action:                                        \
        executeAction<TaskCommand::Action::action>(origin, output, *player); \
        break
        switch (mAction)
        {
            CaseAndBreak(List);
            CaseAndBreak(Follow);
            CaseAndBreak(Sleep);
            CaseAndBreak(Use);
            CaseAndBreak(Custom);
            CaseAndBreak(Cancel);
            default:
                DEBUGBREAK();
                output.error("Unknown action");
                break;
        }
    }
}

template <Action... ops>
inline CommandParameterData TaskCommand::_registerAction(CommandRegistry& registry, char const* name)
{
    registry.addEnum<Action>(name, {{magic_enum::lower_enum_name<ops>().data(), ops}...});
    auto action = makeMandatory<CommandParameterDataType::ENUM>(&TaskCommand::mAction, "action", name);
    action.addOptions(CommandParameterOption::EnumAutocompleteExpansion);
    return action;
}


void TaskCommand::setup(CommandRegistry& registry)
{
    registry.registerCommand(FULL_COMMAND_NAME, "FakePlayer Task", CommandPermissionLevel::GameMasters, {(CommandFlagValue)0}, {(CommandFlagValue)0x80});
    if (!Config::TaskCommandAlias.empty())
        registry.registerAlias(FULL_COMMAND_NAME, Config::TaskCommandAlias);
    
    auto actionFollow = _registerAction<Action::Follow>(registry, "FptFollowAction");
    auto actionNoParam = _registerAction<Action::Sleep, Action::List>(registry, "FptActionNoParam");
    auto actionCustom = _registerAction<Action::Custom>(registry, "FptCustomAction");
    auto actionCancel = _registerAction<Action::Cancel>(registry, "FptCancelAction");
    auto actionUse = _registerAction<Action::Use>(registry, "FptUseAction");
    auto actionSync = _registerAction<Action::Sync>(registry, "FptSyncAction");

    auto opMand = makeMandatory(&TaskCommand::mOperator, "operator", &TaskCommand::mOperator_isSet);
    auto taskMand = makeMandatory(&TaskCommand::mTaskName, "task");
    auto targetOpt = makeOptional(&TaskCommand::mTarget, "target", &TaskCommand::mTarget_isSet);
    auto posOpt = makeOptional(&TaskCommand::mPosition, "position", &TaskCommand::mPosition_isSet);
    auto itemOpt = makeOptional(&TaskCommand::mItem, "item", &TaskCommand::mItem_isSet);
    auto syncPosOpt = makeOptional(&TaskCommand::mSyncPosition, "forPos", &TaskCommand::mSyncPosition_isSet);
    auto syncViewOpt = makeOptional(&TaskCommand::mSyncView, "forView", &TaskCommand::mSyncView_isSet);
    auto jsonOpt = makeOptional(&TaskCommand::mJson, "data", &TaskCommand::mJson_isSet);

    registry.registerOverload<TaskCommand>(FULL_COMMAND_NAME, opMand, actionFollow, targetOpt);
    registry.registerOverload<TaskCommand>(FULL_COMMAND_NAME, opMand, actionNoParam);
    registry.registerOverload<TaskCommand>(FULL_COMMAND_NAME, opMand, actionUse, itemOpt);
    registry.registerOverload<TaskCommand>(FULL_COMMAND_NAME, opMand, actionSync, syncPosOpt, syncViewOpt);
    registry.registerOverload<TaskCommand>(FULL_COMMAND_NAME, opMand, actionCustom, taskMand, jsonOpt);
    registry.registerOverload<TaskCommand>(FULL_COMMAND_NAME, opMand, actionCancel, taskMand);
}
