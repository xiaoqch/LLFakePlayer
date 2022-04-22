#include "pch.h"
#include "TestCommand.h"
#include <EventAPI.h>
#include <MC/ServerPlayer.hpp>
#include <MC/ServerNetworkHandler.hpp>
#include <PlayerInfoAPI.h>
#include <SimulatedPlayerHelper.h>
SimulatedPlayer* testPlayer;

#ifdef DEBUG
#pragma lib("./x64/Release/LLFakePlayer.lib")
#else
#pragma lib("./x64/Debug/LLFakePlayer.lib")
#endif // DEBUG


void entry()
{
    Event::RegCmdEvent::subscribe(
        [](Event::RegCmdEvent ev) {
            TestCommand::setup(*ev.mCommandRegistry);
            return true;
        });
    Event::ServerStartedEvent::subscribe_ref([](Event::ServerStartedEvent& ev) ->bool{
        testPlayer = SimulatedPlayerHelper::create("test");
        return true;
        });
}
