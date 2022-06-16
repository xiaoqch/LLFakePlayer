#include "pch.h"
#include "CommandHelper.h"
#include "TaskCommand.h"
#include "Header/FakePlayerManager.h"

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
    bool sync = mSync_isSet ? mSync : true;
    auto player = origin.getPlayer();
    if (!player)
        sync = false;
    if (sync)
    {
        auto& selectedItem = player->getSelectedItem();
        if (!selectedItem.isNull())
            names.insert(selectedItem.getTypeName());
    }
    SyncHelper syncHelper = sync ? SyncHelper(*player) : SyncHelper();
    auto task = std::make_unique<UseTask>(sp.getUniqueID(), delaySendMessage(origin), names, syncHelper);
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
        

void TaskCommand::setup(CommandRegistry& registry)
{
    registry.registerCommand(FULL_COMMAND_NAME, "FakePlayer Task", CommandPermissionLevel::GameMasters, {(CommandFlagValue)0}, {(CommandFlagValue)0x80});
    if (!Config::TaskCommandAlias.empty())
        registry.registerAlias(FULL_COMMAND_NAME, Config::TaskCommandAlias);
    registry.addEnum<Action>("FptFollowAction",
                             {
                                 {"follow", Action::Follow},
                             });
    registry.addEnum<Action>("FptSleepAction",
                             {
                                 {"sleep", Action::Sleep},
                                 {"list", Action::List},
                             });
    registry.addEnum<Action>("FptCustomAction",
                             {
                                 {"custom", Action::Custom},
                                 {"cancel", Action::Cancel},
                             });
    registry.addEnum<Action>("FptUseAction",
                             {
                                 {"use", Action::Use},
                             });

    auto actionFollow = makeMandatory<CommandParameterDataType::ENUM>(&TaskCommand::mAction, "mAction", "FptFollowAction");
    auto actionSleep = makeMandatory<CommandParameterDataType::ENUM>(&TaskCommand::mAction, "mAction", "FptSleepAction");
    auto actionCustom = makeMandatory<CommandParameterDataType::ENUM>(&TaskCommand::mAction, "mAction", "FptCustomAction");
    auto actionUse = makeMandatory<CommandParameterDataType::ENUM>(&TaskCommand::mAction, "mAction", "FptUseAction");
    actionFollow.addOptions(CommandParameterOption::EnumAutocompleteExpansion);
    actionSleep.addOptions(CommandParameterOption::EnumAutocompleteExpansion);

    auto opMand = makeMandatory(&TaskCommand::mOperator, "operator", &TaskCommand::mOperator_isSet);
    auto taskMand = makeMandatory(&TaskCommand::mTaskName, "task");
    auto targetOpt = makeOptional(&TaskCommand::mTarget, "target", &TaskCommand::mTarget_isSet);
    auto posOpt = makeOptional(&TaskCommand::mPosition, "position", &TaskCommand::mPosition_isSet);
    auto syncOpt = makeOptional(&TaskCommand::mSync, "sync", &TaskCommand::mSync_isSet);
    auto jsonOpt = makeOptional(&TaskCommand::mJson, "dimension", &TaskCommand::mJson_isSet);

    registry.registerOverload<TaskCommand>(FULL_COMMAND_NAME, opMand, actionFollow, targetOpt);
    registry.registerOverload<TaskCommand>(FULL_COMMAND_NAME, opMand, actionSleep, posOpt);
    registry.registerOverload<TaskCommand>(FULL_COMMAND_NAME, opMand, actionUse, syncOpt);
    registry.registerOverload<TaskCommand>(FULL_COMMAND_NAME, opMand, actionCustom, taskMand, jsonOpt);
}
