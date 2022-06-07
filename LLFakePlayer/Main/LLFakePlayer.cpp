#include <pch.h>
#include <EventAPI.h>
#include "FakePlayerManager.h"
#include "FakePlayerCommand.h"
#include "../SkinHelper.h"
#include <MC/NetworkIdentifier.hpp>

extern bool ExportRemoteCallApis();

void entry()
{
    ExportRemoteCallApis();
    // SkinHelper::init();
    Event::RegCmdEvent::subscribe([](Event::RegCmdEvent ev) -> bool {
#ifdef PLUGIN_IS_BETA
        TickingCommand::setup(*ev.mCommandRegistry);
#endif // PLUGIN_IS_BETA
        FakePlayerCommand::setup(*ev.mCommandRegistry);
        return true;
    });
    // ========== Test ==========
#ifdef PLUGIN_IS_DEV
    auto listener = Event::PlayerJoinEvent::subscribe([](Event::PlayerJoinEvent const& ev) -> bool {
        DEBUGW("NetworkIdentifier: {}", ev.mPlayer->getNetworkIdentifier()->toString());
        return true;
    });
    Event::ServerStartedEvent::subscribe([](Event::ServerStartedEvent ev) -> bool {
#ifdef MODIFY_SIMULATED_PLAYER_VTABLE
        static void* fakeSimulatedPlayerVftbl[445];
        void** vftbl_simulated = (void**)dlsym_real("??_7SimulatedPlayer@@6B@");
        void** vftbl_server = (void**)dlsym_real("??_7ServerPlayer@@6B@");
        memcpy(fakeSimulatedPlayerVftbl, vftbl_server, sizeof(fakeSimulatedPlayerVftbl));
#define RestoreVFunc(index) fakeSimulatedPlayerVftbl[index] = vftbl_simulated[index]
        RestoreVFunc(14);
        RestoreVFunc(292);
        RestoreVFunc(10);
        // RestoreVFunc(339);
        // RestoreVFunc(438);
        RestoreVFunc(439);
        RestoreVFunc(440);
        RestoreVFunc(443);
#undef RestoreVFunc
#endif // MODIFY_SIMULATED_PLAYER_VTABLE
       // FakePlayerManager::getManager();
        return true;
    });
#endif // PLUGIN_IS_BETA
}
