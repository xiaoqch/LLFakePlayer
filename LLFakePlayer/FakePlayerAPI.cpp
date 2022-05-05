#include "pch.h"
#include "FakePlayerAPI.h"
#include <RemoteCallAPI.h>
#include <MC/SimulatedPlayer.hpp>
#include <FakePlayerManager.h>

#define ExportRemoteCallApi(name) RemoteCall::exportAs<decltype(FakePlayerAPI::name)>("FakePlayerAPI", #name, FakePlayerAPI::name)
#define ExportRemoteCallApiAs(name, func) RemoteCall::exportAs<func>("FakePlayerAPI", #name, (func)&FakePlayerAPI::name)

namespace FakePlayerAPI
{
std::vector<int> getVersion()
{
    return {PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_REVISION, PLUGIN_VERSION_BUILD};
}
std::string getVersionString() {
    return PLUGIN_FILE_VERSION_STRING;
}

std::vector<SimulatedPlayer*> getOnlineList(){
    std::vector<SimulatedPlayer*> list;
    for (auto& player: Level::getAllPlayers()) {
        if (player->isSimulatedPlayer())
            list.emplace_back(static_cast<SimulatedPlayer*>(player));
    }
    return list;
}
std::vector<FakePlayerState> getAllStates()
{
    return {};
}

std::string getAllStatesJson()
{
    return "";
}

std::vector<std::string> list()
{
    return FakePlayerManager::getManager().getSortedNames();
}
SimulatedPlayer* login(std::string const& name)
{
    return FakePlayerManager::getManager().login(name);
}
bool logout(std::string const& name)
{
    return FakePlayerManager::getManager().logout(name);
}
bool create(std::string const& name)
{
    return FakePlayerManager::getManager().create(name);
}
bool createWithData(std::string const& name, CompoundTag* data)
{
    return FakePlayerManager::getManager().create(name, data->clone());
}
bool createAt(std::string const& name, BlockPos pos, int dimid)
{
    return FakePlayerManager::getManager().create(name);
}
bool remove(std::string const& name)
{
    return FakePlayerManager::getManager().remove(name);
}
std::vector<std::string> loginAll()
{
    return std::vector<std::string>();
}
std::vector<std::string> logoutAll()
{
    return std::vector<std::string>();
}
std::vector<std::string> removeAll()
{
    return std::vector<std::string>();
}
bool importDDFFakePlayer(std::string const& name)
{
    return FakePlayerManager::getManager().importData_DDF(name);
}

bool exportRemoteCallApis()
{
    bool res = true;
    res = res && ExportRemoteCallApi(getVersion);
    res = res && ExportRemoteCallApi(getVersionString);
    res = res && ExportRemoteCallApi(getOnlineList);
    res = res && ExportRemoteCallApi(getAllStatesJson);
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
    auto test = ImportFakePlayerAPI(getVersionString);
    using Type = decltype(FakePlayerAPI::getVersionString);
    auto test2 = RemoteCall::importAs<decltype(FakePlayerAPI::getVersionString)>("FakePlayerAPI", "getVersionString");
    logger.info(test2());
    assert(test() == PLUGIN_FILE_VERSION_STRING);
#endif // DEBUG

    return res;
}

} // namespace FakePlayerAPI