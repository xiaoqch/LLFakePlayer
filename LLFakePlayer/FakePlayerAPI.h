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


namespace FakePlayerAPI
{
struct FakePlayerState
{
    std::string name;
    std::string uuid;
    bool online;
};

FPAPI std::vector<int> getVersion();
FPAPI std::string getVersionString();

FPAPI std::vector<SimulatedPlayer*> getOnlineList();
FPAPI std::vector<FakePlayerState> getAllStates();
FPAPI std::string getAllStatesJson();

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
