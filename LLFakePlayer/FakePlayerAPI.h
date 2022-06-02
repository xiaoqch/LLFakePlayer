#pragma once
#ifndef FPAPI
#ifdef LLFAKEPLAYER_EXPORTS
#define FPAPI __declspec(dllexport)
#else
#define FPAPI __declspec(dllimport)
#endif
#endif // !FPAPI

// example getVersionString = ImportFakePlayerAPI
#define ImportFakePlayerAPI(name) RemoteCall::importAs<decltype(FakePlayerAPI::name)>("FakePlayerAPI", #name)

class SimulatedPlayer;
class FakePlayer;


namespace FakePlayerAPI
{
struct FakePlayerState
{
    std::string name;
    std::string xuid;
    mce::UUID uuid;
    std::string skinId = "";
    bool online =false;
    
    FPAPI std::string toJson();
};

FPAPI std::vector<int> getVersion();
FPAPI std::string getVersionString();

FPAPI std::vector<SimulatedPlayer*> getOnlineList();
FPAPI FakePlayerState getState(std::string const& name);
FPAPI std::string getStateJson(std::string const& name);
FPAPI std::vector<FakePlayerState> getAllStates();
FPAPI std::string getAllStateJson();

FPAPI std::vector<std::string> list();
FPAPI SimulatedPlayer* login(std::string const& name);
FPAPI bool logout(std::string const& name);
FPAPI bool create(std::string const& name);
FPAPI bool createWithData(std::string const& name, CompoundTag* data);
FPAPI bool createAt(std::string const& name, BlockPos pos, int dimid);
FPAPI bool remove(std::string const& name);
FPAPI std::vector<std::string> loginAll();
FPAPI std::vector<std::string> logoutAll();
FPAPI std::vector<std::string> removeAll();
FPAPI bool importDDFFakePlayer(std::string const& name);

};
