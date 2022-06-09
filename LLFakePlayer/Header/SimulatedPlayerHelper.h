#pragma once
#include <MC/SimulatedPlayer.hpp>

#include <MC/ServerNetworkHandler.hpp>
#include <MC/Level.hpp>
#include <MC/StackResultStorageEntity.hpp>
#include <MC/OwnerStorageEntity.hpp>


inline class SimulatedPlayer* getSimulatedPlayerByUuid(class mce::UUID const& a0)
{
    auto level = Global<Level>;
    if (!Global<Level>)
        return nullptr;
    auto sp = level->getPlayer(a0);
    if (sp && sp->isSimulatedPlayer())
        return static_cast<SimulatedPlayer*>(sp);
    return nullptr;
}

template <>
class OwnerPtrT<struct EntityRefTraits>
{
    char filler[24];

public:
    MCAPI ~OwnerPtrT();
    OwnerPtrT(OwnerPtrT&& right) noexcept
    {
        reinterpret_cast<OwnerStorageEntity*>(this)->OwnerStorageEntity::OwnerStorageEntity(reinterpret_cast<OwnerStorageEntity&&>(right));
    }
    inline OwnerPtrT& operator=(OwnerPtrT&& right) noexcept
    {
        *reinterpret_cast<OwnerStorageEntity*>(this) = reinterpret_cast<OwnerStorageEntity&&>(right);
        return *this;
    }
    inline SimulatedPlayer* tryGetSimulatedPlayer(bool b = false)
    {
        return SimulatedPlayer::tryGetFromEntity(reinterpret_cast<OwnerStorageEntity*>(this)->_getStackRef(), b);
    }
};

namespace SimulatedPlayerHelper
{
// Rewrite SimulatedPlayer::create(std::string const & name,class BlockPos const & bpos,class AutomaticID<class Dimension,int> dimId,class ServerNetworkHandler & handler)
inline SimulatedPlayer* create(std::string const& name, BlockPos* bpos = nullptr, class AutomaticID<class Dimension, int> dimId = 0)
{
    auto handler = Global<ServerNetworkHandler>;
    if (handler == nullptr)
        return nullptr;
    std::string xuid = "";
#if BDS_VER > 11910
    OwnerPtrT<EntityRefTraits> ownerPtr = handler->createSimulatedPlayer(name, dimId, xuid);
#else
    OwnerPtrT<EntityRefTraits> ownerPtr = handler->createSimulatedPlayer(name, dimId);
#endif
    auto player = ownerPtr.tryGetSimulatedPlayer();

    if (player /* && player->isSimulated() */)
    {
        auto unk = *dAccess<void (**)(), 8880>(player);
        player->postLoad(/* isNewPlayer */ false);
        Level& level = player->getLevel();
        level.addUser(std::move(ownerPtr));
        if (bpos)
        {
            auto pos = bpos->bottomCenter();
            pos.y = pos.y + 1.62001f;
            player->setPos(pos);
            player->setRespawnReady(pos);
            player->setSpawnBlockRespawnPosition(*bpos, dimId);
        }
        player->setLocalPlayerAsInitialized();
        player->doInitialSpawn();
    }
    return player;
}
inline SimulatedPlayer* tryGetSimulatedPlayer(Actor* actor)
{
    if (actor && actor->isSimulatedPlayer())
        return static_cast<SimulatedPlayer*>(actor);
    return nullptr;
}
}; // namespace SimulatedPlayerHelper
