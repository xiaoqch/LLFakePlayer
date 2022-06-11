#pragma once

class SimulatedPlayer;

//#define COMMAND_AS_SUB_COMMAND
extern void UpdateLLFakePlayerSoftEnum();

class FakePlayerCommand : public Command
{
    static std::vector<std::string> mList;
    std::string name;
    enum class Operation
    {
        Help,
        List,
        Create,
        Remove,
        Login,
        Logout,
#ifdef COMMAND_AS_SUB_COMMAND
        As,
#endif // COMMAND_AS_SUB_COMMAND
        Import,
    } operation;
    CommandPosition commandPos;
    int dimensionId;
    bool name_isSet, commandPos_isSet, dimensionId_isSet;

    virtual void execute(class CommandOrigin const& origin, class CommandOutput& output) const override;

public:
    static void setup(CommandRegistry& registry);
};
