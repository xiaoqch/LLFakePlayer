#include "pch.h"
#include "FakePlayerAPI.h"
#include <RemoteCallAPI.h>
#include <MC/SimulatedPlayer.hpp>
#include <FakePlayerManager.h>

#define ExportRemoteCallApi(name) RemoteCall::exportAs<decltype(FakePlayerAPI::name)>("FakePlayerAPI", #name, FakePlayerAPI::name)
#define ExportRemoteCallApiAs(name, func) RemoteCall::exportAs<func>("FakePlayerAPI", #name, (func)&FakePlayerAPI::name)

namespace FakePlayerAPI
{

std::string StateToJson(FakePlayerState const& state, nlohmann::json& json)
{
    json["name"] = state.name;
    json["xuid"] = state.xuid;
    json["uuid"] = state.uuid.asString();
    json["skinId"] = state.skinId;
    json["online"] = state.online;
    return json.dump();
}
FakePlayerState StateFromFakePlayer(FakePlayer const& fp)
{
    return {fp.getRealName(), fp.getXuid(), fp.getUuid(), "", fp.isOnline()};
}


std::string FakePlayerState::toJson()
{
    nlohmann::json json;
    return StateToJson(*this, json);
}

std::vector<int> getVersion()
{
    DEBUGL(__FUNCTION__);
    return {PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_REVISION, PLUGIN_VERSION_BUILD};
}
std::string getVersionString()
{
    DEBUGL(__FUNCTION__);
    return PLUGIN_FILE_VERSION_STRING;
}

std::vector<SimulatedPlayer*> getOnlineList()
{
    DEBUGL(__FUNCTION__);
    std::vector<SimulatedPlayer*> list;
    for (auto& fp : FakePlayerManager::getManager().getFakePlayerList())
    {
        if (fp->isOnline())
            list.emplace_back(fp->getPlayer());
    }
    return list;
}

FPAPI FakePlayerState getState(std::string const& name)
{
    DEBUGL(__FUNCTION__);
    auto fp = FakePlayerManager::getManager().tryGetFakePlayer(name);
    if (fp)
        return StateFromFakePlayer(*fp);
    return {};
}

FPAPI std::string getStateJson(std::string const& name)
{
    DEBUGL(__FUNCTION__);
    auto state = getState(name);
    return state.toJson();
}

std::vector<FakePlayerState> getAllStates()
{
    DEBUGL(__FUNCTION__);
    std::vector<FakePlayerState> list;
    for (auto& fp : FakePlayerManager::getManager().getFakePlayerList()) {
        list.emplace_back(StateFromFakePlayer(*fp));
    }
    return list;
}

std::string getAllStateJson()
{
    DEBUGL(__FUNCTION__);
    nlohmann::json json;
    for (auto& state : getAllStates()) {
        StateToJson(state, json[state.name]);
    }
    return json.dump();
}

std::vector<std::string> list()
{
    DEBUGL(__FUNCTION__);
    return FakePlayerManager::getManager().getSortedNames();
}
SimulatedPlayer* login(std::string const& name)
{
    DEBUGL(__FUNCTION__);
    return FakePlayerManager::getManager().login(name);
}
bool logout(std::string const& name)
{
    DEBUGL(__FUNCTION__);
    return FakePlayerManager::getManager().logout(name);
}
bool create(std::string const& name)
{
    DEBUGL(__FUNCTION__);
    return FakePlayerManager::getManager().create(name);
}
bool createWithData(std::string const& name, CompoundTag* data)
{
    DEBUGL(__FUNCTION__);
    return FakePlayerManager::getManager().create(name, data->clone());
}
bool createAt(std::string const& name, BlockPos pos, int dimid)
{
    DEBUGL(__FUNCTION__);
    return FakePlayerManager::getManager().create(name);
}
bool remove(std::string const& name)
{
    DEBUGL(__FUNCTION__);
    return FakePlayerManager::getManager().remove(name);
}
std::vector<std::string> loginAll()
{
    DEBUGL(__FUNCTION__);
    return std::vector<std::string>();
}
std::vector<std::string> logoutAll()
{
    DEBUGL(__FUNCTION__);
    return std::vector<std::string>();
}
std::vector<std::string> removeAll()
{
    DEBUGL(__FUNCTION__);
    return std::vector<std::string>();
}
bool importDDFFakePlayer(std::string const& name)
{
    DEBUGL(__FUNCTION__);
    return FakePlayerManager::getManager().importData_DDF(name);
}

bool exportRemoteCallApis()
{
    bool res = true;
    res = res && ExportRemoteCallApi(getVersion);
    res = res && ExportRemoteCallApi(getVersionString);
    res = res && ExportRemoteCallApi(getOnlineList);
    res = res && ExportRemoteCallApi(getStateJson);
    res = res && ExportRemoteCallApi(getAllStateJson);
    res = res && ExportRemoteCallApi(list);
    res = res && ExportRemoteCallApi(login);
    res = res && ExportRemoteCallApi(logout);
    res = res && ExportRemoteCallApi(create);
    res = res && ExportRemoteCallApi(createAt);
    res = res && ExportRemoteCallApi(createWithData);
    res = res && ExportRemoteCallApi(remove);
    res = res && ExportRemoteCallApi(loginAll);
    res = res && ExportRemoteCallApi(logoutAll);
    res = res && ExportRemoteCallApi(removeAll);
    res = res && ExportRemoteCallApi(importDDFFakePlayer);


#ifdef DEBUG
    auto list = ImportFakePlayerAPI(list);
    TestFuncTime(list);
    TestFuncTime(list);
    TestFuncTime(list);
    auto test = ImportFakePlayerAPI(getVersionString);
    using Type = decltype(FakePlayerAPI::getVersionString);
    auto test2 = RemoteCall::importAs<decltype(FakePlayerAPI::getVersionString)>("FakePlayerAPI", "getVersionString");
    logger.info(test2());
    assert(test() == PLUGIN_FILE_VERSION_STRING);
#endif // DEBUG

    return res;
}

} // namespace FakePlayerAPI