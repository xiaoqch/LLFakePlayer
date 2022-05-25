#include "pch.h"
#if false

#include "SkinHelper.h"
#include <EventAPI.h>
#include <ScheduleAPI.h>
#include <MC/SimulatedPlayer.hpp>
#include <MC/VanillaActorRendererId.hpp>
#include <MC/BinaryStream.hpp>
inline class std::unordered_map<class mce::UUID, class PlayerListEntry, struct std::hash<class mce::UUID>, struct std::equal_to<class mce::UUID>, class std::allocator<struct std::pair<class mce::UUID const, class PlayerListEntry>>>& getPlayerList()
{
    class std::unordered_map<class mce::UUID, class PlayerListEntry, struct std::hash<class mce::UUID>, struct std::equal_to<class mce::UUID>, class std::allocator<struct std::pair<class mce::UUID const, class PlayerListEntry>>>& (Level::*rv)();
    *((void**)&rv) = dlsym("?getPlayerList@Level@@UEAAAEAV?$unordered_map@VUUID@mce@@VPlayerListEntry@@U?$hash@VUUID@mce@@@std@@U?$equal_to@VUUID@mce@@@5@V?$allocator@U?$pair@$$CBVUUID@mce@@VPlayerListEntry@@@std@@@5@@std@@XZ");
    return (Global<Level>->*rv)();
};

bool SkinHelper::init()
{
#ifdef DEBUG
    Event::PlayerJoinEvent::subscribe_ref([](Event::PlayerJoinEvent& ev) -> bool {
        logger.warn("PlayerSkinID: {}", ev.mPlayer->getSkinID());
        logger.warn("ActorRendererId: {}", ev.mPlayer->getActorRendererId().getString());
        static SerializedSkin skin;
        if (ev.mPlayer->isSimulatedPlayer())
            updatePlayerSkin(*ev.mPlayer, skin);
        else
            skin = ev.mPlayer->getSkin();
        return true;
    });
#endif // DEBUG

    return true;
}
bool SkinHelper::updatePlayerSkin(Player& player, SerializedSkin const& skin)
{
    auto sid = player.getSkinID();
    auto skinCopy = SerializedSkin();
    BinaryStream bs;
    skin.write(bs);
    skinCopy.read(bs);
    player.updateSkin(skinCopy, 0);
    logger.warn("ActorRendererId: {}", player.getActorRendererId().getString());
    auto playerUUID = mce::UUID::fromString(player.getUuid());
    auto& list = getPlayerList();
    for (auto& [uuid, entry] : list)
        if (uuid == playerUUID)
        {
            entry.skin = skinCopy;
            return true;
        }
    auto pkt = PlayerSkinPacket::fromPlayer(player);
    Level::sendPacketForAllPlayer(*pkt);
    return false;
}
#include <MC/CommandUtils.hpp>
TInstanceHook(void, "?updateSkin@Player@@QEAAXAEBVSerializedSkin@@H@Z",
              Player, class SerializedSkin const& skin, int id)
{
    logger.warn("{} updateSkin - {}, skin id: {}", CommandUtils::getActorName(*this), id, skin.getName());
    return original(this, skin, id);
}


#ifdef DEBUG
auto id = Schedule::nextTick([]() {
    logger.info("{}: {}", "_map", VanillaActorRendererId::_map.getString());
    logger.info("{}: {}", "_photo", VanillaActorRendererId::_photo.getString());
    logger.info("{}: {}", "_query", VanillaActorRendererId::_query.getString());
    logger.info("{}: {}", "elderGuardian", VanillaActorRendererId::elderGuardian.getString());
    logger.info("{}: {}", "elderGuardianGhost", VanillaActorRendererId::elderGuardianGhost.getString());
    logger.info("{}: {}", "guardian", VanillaActorRendererId::guardian.getString());
    logger.info("{}: {}", "itemEntity", VanillaActorRendererId::itemEntity.getString());
    logger.info("{}: {}", "lingeringPotion", VanillaActorRendererId::lingeringPotion.getString());
    logger.info("{}: {}", "npc", VanillaActorRendererId::npc.getString());
    logger.info("{}: {}", "player", VanillaActorRendererId::player.getString());
    logger.info("{}: {}", "shield", VanillaActorRendererId::shield.getString());
    logger.info("{}: {}", "thrownPotion", VanillaActorRendererId::thrownPotion.getString());
    logger.info("{}: {}", "trident", VanillaActorRendererId::trident.getString());
});
#endif // DEBUG


#endif // false