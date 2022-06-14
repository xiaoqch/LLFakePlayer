#pragma once

class SimulatedPlayer;

extern void UpdateLLFakePlayerSoftEnum();

class FakePlayerCommand : public Command
{
    std::string mName;
    enum class Action
    {
        Help,
        List,
        Add,
        Remove,
        Login,
        Logout,
#ifdef COMMAND_AS_ACTION
        As,
#endif // COMMAND_AS_ACTION
        Import,
    } mAction;
    CommandPosition mPosition;
    int mDimensionId;
    bool name_isSet, commandPos_isSet, dimensionId_isSet;
    template <Action op>
    void executeAction(class CommandOrigin const& origin, class CommandOutput& output) const = delete;
    
    virtual void execute(class CommandOrigin const& origin, class CommandOutput& output) const override;

public:
    static void setup(CommandRegistry& registry);
};
