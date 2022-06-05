#include "pch.h"
#include "FakePlayerAPI.h"
#include <RemoteCallAPI.h>
#include <MC/SimulatedPlayer.hpp>
#include <FakePlayerManager.h>

#define FAKE_PALYER_API_DEBUG
#if defined(FAKE_PALYER_API_DEBUG) && !defined(DEBUG)
#undef DEBUGL
#define DEBUGL(...) logger.info(__VA_ARGS__)
#endif // FAKE_PALYER_API_DEBUG



#define ExportRemoteCallApi(name) RemoteCall::exportAs<decltype(FakePlayerAPI::name)>(LLFAKEPLAYER_NAMESPACE, #name, FakePlayerAPI::name)
#define ExportRemoteCallApiAs(name, func) RemoteCall::exportAs<func>(LLFAKEPLAYER_NAMESPACE, #name, (func)&FakePlayerAPI::name)

namespace FakePlayerAPI
{

std::string StateToJson(FakePlayerState const& state, nlohmann::json& json)
{
    json["name"] = state.name;
    json["xuid"] = state.xuid;
    json["uuid"] = state.uuid.asString();
    json["skinId"] = state.skinId;
    json["online"] = state.online;
    json["autoLogin"] = state.autoLogin;
    json["lastUpdateTime"] = state.lastUpdateTime;
    return json.dump();
}
FakePlayerState StateFromFakePlayer(FakePlayer const& fp)
{
    return {fp.getRealName(), fp.getXuid(), fp.getUuid(), "", fp.isOnline(), fp.isAutoLogin(), fp.getLastUpdateTime()};
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
    for (auto& fp : FakePlayerManager::getManager().getFakePlayerList())
    {
        list.emplace_back(StateFromFakePlayer(*fp));
    }
    return list;
}

std::string getAllStateJson()
{
    DEBUGL(__FUNCTION__);
    nlohmann::json json;
    for (auto& state : getAllStates())
    {
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
    std::vector<std::string> list;
    FakePlayerManager::getManager().forEachFakePlayer([&](std::string_view name, FakePlayer& fp) {
        if (!fp.isOnline())
            if (fp.login())
                list.emplace_back(name);
    });
    return list;
}
std::vector<std::string> logoutAll()
{
    DEBUGL(__FUNCTION__);
    std::vector<std::string> list;
    FakePlayerManager::getManager().forEachFakePlayer([&](std::string_view name, FakePlayer& fp) {
        if (fp.isOnline())
            if (fp.logout())
                list.emplace_back(name);
    });
    return list;
}
std::vector<std::string> removeAll()
{
    DEBUGL(__FUNCTION__);
    std::vector<std::string> list;
    auto names = FakePlayerManager::getManager().getSortedNames();
    for (auto& name : names)
    {
        if (FakePlayerManager::getManager().remove(name))
            list.emplace_back(name);
    }
    return list;
}
bool importDDFFakePlayer(std::string const& name)
{
    DEBUGL(__FUNCTION__);
    return FakePlayerManager::getManager().importData_DDF(name);
}

size_t subscribeEvent(std::function<void(Event&)> const& handler)
{
    DEBUGL(__FUNCTION__);
    return FakePlayerManager::getManager().subscribeEvent(handler);
}

bool unsubscribeEvent(size_t id)
{
    DEBUGL(__FUNCTION__);
    return FakePlayerManager::getManager().unsubscribeEvent(id);
}

using RemoteCallCallbackFn = bool(std::string const& type, std::string const& name);
static std::vector<std::tuple<std::string, std::string, std::function<RemoteCallCallbackFn>>> callbacks;
bool remoteCallSubscribe(std::string const& callbackNamespace, std::string const& callbackName)
{
    DEBUGL(__FUNCTION__);
    if (std::find_if(callbacks.begin(), callbacks.end(), [&](auto const& cb) {
            return std::get<0>(cb) == callbackNamespace && std::get<1>(cb) == callbackName;
        }) != callbacks.end())
        return false;
    callbacks.emplace_back(callbackNamespace, callbackName, RemoteCall::importAs<RemoteCallCallbackFn>(callbackNamespace, callbackName));
    return true;
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
    
    FakePlayerManager::getManager().subscribeEvent([](Event& ev) {
        DEBUGL(__FUNCTION__);
        bool needClean = false;
        static std::unordered_map<Event::EventType, std::string> eventNameMap{
            {Event::EventType::Add, "add"},
            {Event::EventType::Remove, "remove"},
            {Event::EventType::Login, "login"},
            {Event::EventType::Logout, "logout"},
            {Event::EventType::Change, "change"},
        };

        DEBUGL("Event: {}, player: {}", eventNameMap[ev.mEvent], ev.mPlayer.getRealName());
        for (auto& [ns, name, func] : callbacks)
        {
            if (RemoteCall::hasFunc(ns, name))
                func(eventNameMap[ev.mEvent], ev.mPlayer.getRealName());
            else
                needClean = true;
        }
        if (needClean)
        {
            callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(), [](auto& t) {
                return !RemoteCall::hasFunc(std::get<0>(t), std::get<1>(t));
            }), callbacks.end());
        }
    });
    res = res && RemoteCall::exportAs(LLFAKEPLAYER_NAMESPACE, "subscribeEvent", remoteCallSubscribe);
    

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
