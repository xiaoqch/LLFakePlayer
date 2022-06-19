#pragma once
#include "Main/TaskSystem.h"

class TaskCommand : public Command
{
public:
    enum class Action
    {
        List,
        Follow,
        Sleep,
        Use,
        Sync,
        Custom,
        Cancel,
    };

private:
    Action mAction;
    CommandSelector<Player> mOperator;
    CommandSelector<Actor> mTarget;
    std::string mTaskName;
    CommandPosition mPosition;
    Json::Value mJson;
    CommandItem mItem;
    bool mSyncPosition, mSyncView;
    bool mOperator_isSet, mTarget_isSet, mPosition_isSet, mJson_isSet, mItem_isSet;
    bool mSyncPosition_isSet, mSyncView_isSet;
    
    template <Action op>
    void executeAction(class CommandOrigin const& origin, class CommandOutput& output, SimulatedPlayer& sp) const {
        output.error("Unimplemented");
    };
    Actor* getUniqueTarget(class CommandOrigin const& origin, class CommandOutput& output) const;

    Task::CallbackFn delaySendMessage(class CommandOrigin const& origin, std::string const& msg = "") const;

    template <Action... ops>
    static CommandParameterData _registerAction(CommandRegistry& registry, char const* name);
    
    virtual void execute(class CommandOrigin const& origin, class CommandOutput& output) const override;

public:
    static void setup(CommandRegistry& registry);
};
