#pragma once
#include <RegCommandAPI.h>

class TestCommand : public Command
{
    enum class Operation
    {
    } mOperation;
    bool mOperation_isSet;
    string param;
    virtual void execute(class CommandOrigin const&, class CommandOutput&) const override;

public:
    static void setup(CommandRegistry& registry);
};
