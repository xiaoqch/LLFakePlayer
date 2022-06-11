#pragma once
#include <MC/SimulatedPlayer.hpp>
#include <MC/ServerNetworkHandler.hpp>
#include <MC/StackResultStorageEntity.hpp>
#include <MC/OwnerStorageEntity.hpp>
#define BDS_1_18

template <>
class OwnerPtrT<struct EntityRefTraits> : public OwnerStorageEntity
{
    char filler[24];

public:
    MCAPI ~OwnerPtrT();
    OwnerPtrT(OwnerPtrT&& right) noexcept
        : OwnerStorageEntity(std::move(right))
    {
    }
    inline OwnerPtrT& operator=(OwnerPtrT&& right) noexcept
    {
        *this = std::move(right);
        return *this;
    }
    inline SimulatedPlayer* tryGetSimulatedPlayer(bool b = false)
    {
        return SimulatedPlayer::tryGetFromEntity(this->_getStackRef(), b);
    }
};

#ifdef BDS_1_18
#include "Utils/FakeLevel.h"
#else
#include <MC/Level.hpp>
#endif

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
        player->postLoad(/* isNewPlayer */ false);
        Level& level = player->getLevel();
        level.addUser(std::move(ownerPtr));
        //execute @a ~~~ tp ~~~
        if (bpos)
        {
            auto pos = bpos->bottomCenter();
            pos.y = pos.y + 1.62001f;
            player->setPos(pos);
            player->setRespawnReady(pos);
            player->setSpawnBlockRespawnPosition(*bpos, dimId);
            player->setLocalPlayerAsInitialized();
#ifndef BDS_1_18
            player->doInitialSpawn();
#endif
        }
        else
        {
            auto pos = player->getPos();
            player->setPos(pos);
            player->setRespawnReady(pos);
            player->setLocalPlayerAsInitialized();
#ifndef BDS_1_18
            player->doInitialSpawn();
#endif
        }
        auto pos3 = player->getPos();
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
