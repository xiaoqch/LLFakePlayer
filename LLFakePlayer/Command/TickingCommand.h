#pragma once

#ifdef PLUGIN_IS_BETA

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