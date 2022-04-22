#pragma once
#include <MC/SimulatedPlayer.hpp>

#include <MC/ServerNetworkHandler.hpp>
#include <MC/Level.hpp>
#include <MC/StackResultStorageEntity.hpp>
#include <MC/OwnerStorageEntity.hpp>
namespace SimulatedPlayerHelper
{
SimulatedPlayer* create(std::string const& name);
}
template <>
class OwnerPtrT<struct EntityRefTraits>
{
    char filler[24];

public:
    MCAPI ~OwnerPtrT();
    // inline ~OwnerPtrT()
    //{
    //     void (OwnerPtrT::*rv)() const;
    //     *((void**)&rv) = dlsym("??1?$OwnerPtrT@UEntityRefTraits@@@@QEAA@XZ");
    //     (this->*rv)();
    // }

    inline OwnerPtrT(OwnerPtrT&& right) noexcept
    {
        void (OwnerPtrT::*rv)(OwnerPtrT && right);
        *((void**)&rv) = dlsym("??0OwnerStorageEntity@@IEAA@$$QEAV0@@Z");
        (this->*rv)(std::move(right));
    }
    inline OwnerPtrT& operator=(OwnerPtrT&& right) noexcept
    {
        void (OwnerPtrT::*rv)(OwnerPtrT && right);
        *((void**)&rv) = dlsym("??4OwnerStorageEntity@@IEAAAEAV0@$$QEAV0@@Z");
        (this->*rv)(std::move(right));
    }

    inline SimulatedPlayer* tryGetSimulatedPlayer(bool b = false)
    {
        auto& context = dAccess<StackResultStorageEntity, 0>(this).getStackRef();
        auto player = SimulatedPlayer::tryGetFromEntity(context, b);
        if (player)
        {
            return player;
        }
        return nullptr;
    }

    inline bool hasValue() const
    {
        if (!this)
            return false;
        return dAccess<bool, 16>(this);
    }
    // inline bool isValid()
};

inline void addUser(Level& level, class OwnerPtrT<struct EntityRefTraits> a0)
{
    void (Level::*rv)(class OwnerPtrT<struct EntityRefTraits>) = nullptr;
    *((void**)&rv) = dlsym("?addUser@Level@@UEAAXV?$OwnerPtrT@UEntityRefTraits@@@@@Z");
    return (level.*rv)(std::forward<class OwnerPtrT<struct EntityRefTraits>>(a0));
}

namespace SimulatedPlayerHelper
{
// Rewrite SimulatedPlayer::create(std::string const & name,class BlockPos const & bpos,class AutomaticID<class Dimension,int> dimId,class ServerNetworkHandler & handler)
inline SimulatedPlayer* create(std::string const& name)
{
    OwnerPtrT<EntityRefTraits> ownerPtr = Global<ServerNetworkHandler>->createSimulatedPlayer(name, 0);
    auto player = ownerPtr.tryGetSimulatedPlayer();

    if (player /* && player->isSimulatedPlayer() */)
    {
        // dAccess<AutomaticID<Dimension, int>>(player, 57) = dimId;
        player->postLoad(/* isNewPlayer */ false);
        Level& level = player->getLevel();
        addUser(level, std::move(ownerPtr));
        // auto pos = bpos.bottomCenter();
        // pos.y = pos.y + 1.62001;
        // player->setPos(pos);
        // player->setRespawnReady(pos);
        // player->setSpawnBlockRespawnPosition(bpos, dimId);
        player->setLocalPlayerAsInitialized();
    }
    return player;
}
inline SimulatedPlayer* tryGetSimulatedPlayer(Actor* actor)
{
    try
    {
        if (actor && actor->isSimulatedPlayer())
            return (SimulatedPlayer*)actor;
        return nullptr;
    }
    catch (...)
    {
    }
    return nullptr;
}
}; // namespace SimulatedPlayerHelper
