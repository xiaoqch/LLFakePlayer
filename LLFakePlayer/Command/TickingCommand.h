#pragma once

#ifdef PLUGIN_IS_BETA

class TickingCommand : public Command
{
    CommandSelector<Player> mTarget;
    CommandPosition mPosition;
    int mDimensionId;
    int mRange;
    bool mTarget_isSet;
    bool mPosition_isSet;
    bool mDimensionId_isSet;
    bool mRange_isSet;

    virtual void execute(class CommandOrigin const& origin, class CommandOutput& output) const override;

public:
    static void setup(CommandRegistry& registry);
};

#endif // PLUGIN_IS_BETA