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
        AutoLogin,
#ifdef COMMAND_AS_ACTION
        As,
#endif // COMMAND_AS_ACTION
        Import,
    } mAction;
    CommandPosition mPosition;
    int mDimensionId;
    bool mAutoLogin = false;
    bool mName_isSet, mPosition_isSet, mDimensionId_isSet, mAutoLogin_isSet;
    template <Action op>
    void executeAction(class CommandOrigin const& origin, class CommandOutput& output) const = delete;

    template <Action... ops>
    static CommandParameterData _registerAction(CommandRegistry& registry, char const* name);

    virtual void execute(class CommandOrigin const& origin, class CommandOutput& output) const override;

public:
    static void setup(CommandRegistry& registry);
};
