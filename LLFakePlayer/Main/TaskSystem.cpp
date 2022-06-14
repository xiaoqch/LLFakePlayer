#include "pch.h"
#include "TaskSystem.h"

TInstanceHook(enum BedSleepingResult, "?startSleepInBed@Player@@UEAA?AW4BedSleepingResult@@AEBVBlockPos@@@Z",
              Player, class BlockPos const& pos)
{
    if (Config::AutoSleep)
        TaskSystem::getInstance().onPlayerSleep(this);
    return original(this, pos);
}

#ifdef DEBUG
using GoalFactoryFn = std::function<std::unique_ptr<class Goal>(class Mob&, struct GoalDefinition const&)>;
static std::unordered_map<std::string, GoalFactoryFn>& mGoalMap = *static_cast<std::add_pointer_t<decltype(mGoalMap)>>(dlsym("?mGoalMap@GoalDefinition@@2V?$unordered_map@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$function@$$A6A?AV?$unique_ptr@VGoal@@U?$default_delete@VGoal@@@std@@@std@@AEAVMob@@AEBUGoalDefinition@@@Z@2@U?$hash@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@U?$equal_to@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@V?$allocator@U?$pair@$$CBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$function@$$A6A?AV?$unique_ptr@VGoal@@U?$default_delete@VGoal@@@std@@@std@@AEAVMob@@AEBUGoalDefinition@@@Z@2@@std@@@2@@std@@A"));
#endif // DEBUG
