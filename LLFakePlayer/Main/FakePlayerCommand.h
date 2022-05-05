#pragma once
class SimulatedPlayer;

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
        Swap,
        GUI,
        Import,
    } operation;
    CommandPosition commandPos;
    int dimensionId;
    bool name_isSet, commandPos_isSet, dimensionId_isSet;

    virtual void execute(class CommandOrigin const& origin, class CommandOutput& output) const override;

public:
    static void setup(CommandRegistry& registry);
};

#ifdef PLUGIN_IS_BETA

// =============== Test ===============
class TickingCommand : public Command
{
    CommandSelector<Player> selector;
    CommandPosition commandPos;
    int dimensionId;
    int range;
    bool selector_isSet;
    bool commandPos_isSet;
    bool dimensionId_isSet;
    bool range_isSet;

    virtual void execute(class CommandOrigin const& origin, class CommandOutput& output) const override;

public:
    static void setup(CommandRegistry& registry);
};

#endif // PLUGIN_IS_BETA