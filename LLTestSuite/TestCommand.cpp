#include "pch.h"
#include "TestCommand.h"

using namespace RegisterCommandHelper;

void TestCommand::execute(class CommandOrigin const& ori, class CommandOutput& outp) const
{
    if (mOperation_isSet)
    {
        switch (mOperation)
        {
            case (Operation)0:
                break;
            default:
                break;
        }
    }
    else
    {
    }
}

void TestCommand::setup(CommandRegistry& registry)
{
    registry.registerCommand("lltest", "LiteLoader Test", CommandPermissionLevel::Any, {(CommandFlagValue)0}, {(CommandFlagValue)0x80});

    registry.addEnum<Operation>("Template_Action", {/* ... */});
    auto action = makeMandatory<CommandParameterDataType::ENUM>(
        &TestCommand::mOperation, "action", "Template_Action", &TestCommand::mOperation_isSet);

    registry.registerOverload<TestCommand>("template");
    registry.registerOverload<TestCommand>("template", action);
}