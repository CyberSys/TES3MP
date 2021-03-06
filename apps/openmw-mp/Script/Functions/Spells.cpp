#include "Spells.hpp"

#include <components/misc/stringops.hpp>
#include <components/openmw-mp/NetworkMessages.hpp>

#include <apps/openmw-mp/Script/ScriptFunctions.hpp>
#include <apps/openmw-mp/Networking.hpp>

using namespace mwmp;

std::vector<ESM::ActiveEffect> storedActiveEffects;

void SpellFunctions::ClearSpellbookChanges(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    player->spellbookChanges.spells.clear();
}

void SpellFunctions::ClearSpellsActiveChanges(unsigned short pid) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, );

    player->spellsActiveChanges.activeSpells.clear();
}

unsigned int SpellFunctions::GetSpellbookChangesSize(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return player->spellbookChanges.spells.size();
}

unsigned int SpellFunctions::GetSpellbookChangesAction(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return player->spellbookChanges.action;
}

unsigned int SpellFunctions::GetSpellsActiveChangesSize(unsigned short pid) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, 0);

    return player->spellsActiveChanges.activeSpells.size();
}

unsigned int SpellFunctions::GetSpellsActiveChangesAction(unsigned short pid) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, 0);

    return player->spellsActiveChanges.action;
}

void SpellFunctions::SetSpellbookChangesAction(unsigned short pid, unsigned char action) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    player->spellbookChanges.action = action;
}

void SpellFunctions::SetSpellsActiveChangesAction(unsigned short pid, unsigned char action) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, );

    player->spellsActiveChanges.action = action;
}

void SpellFunctions::AddSpell(unsigned short pid, const char* spellId) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    ESM::Spell spell;
    spell.mId = spellId;

    player->spellbookChanges.spells.push_back(spell);
}

void SpellFunctions::AddSpellActive(unsigned short pid, const char* spellId, const char* displayName) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, );

    mwmp::ActiveSpell spell;
    spell.id = spellId;
    spell.params.mDisplayName = displayName;
    spell.params.mEffects = storedActiveEffects;

    player->spellsActiveChanges.activeSpells.push_back(spell);

    storedActiveEffects.clear();
}

void SpellFunctions::AddSpellActiveEffect(unsigned short pid, int effectId, double magnitude, double duration, double timeLeft, int arg) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, );

    ESM::ActiveEffect effect;
    effect.mEffectId = effectId;
    effect.mMagnitude = magnitude;
    effect.mDuration = duration;
    effect.mTimeLeft = timeLeft;
    effect.mArg = arg;

    storedActiveEffects.push_back(effect);
}

const char *SpellFunctions::GetSpellId(unsigned short pid, unsigned int index) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, "");

    if (index >= player->spellbookChanges.spells.size())
        return "invalid";

    return player->spellbookChanges.spells.at(index).mId.c_str();
}

const char* SpellFunctions::GetSpellsActiveId(unsigned short pid, unsigned int index) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, "");

    if (index >= player->spellsActiveChanges.activeSpells.size())
        return "invalid";

    return player->spellsActiveChanges.activeSpells.at(index).id.c_str();
}

const char* SpellFunctions::GetSpellsActiveDisplayName(unsigned short pid, unsigned int index) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, "");

    if (index >= player->spellsActiveChanges.activeSpells.size())
        return "invalid";

    return player->spellsActiveChanges.activeSpells.at(index).params.mDisplayName.c_str();
}

unsigned int SpellFunctions::GetSpellsActiveEffectCount(unsigned short pid, unsigned int index) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, 0);

    if (index >= player->spellsActiveChanges.activeSpells.size())
        return 0;

    return player->spellsActiveChanges.activeSpells.at(index).params.mEffects.size();
}

unsigned int SpellFunctions::GetSpellsActiveEffectId(unsigned short pid, unsigned int spellIndex, unsigned int effectIndex) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, 0);

    if (spellIndex >= player->spellsActiveChanges.activeSpells.size())
        return 0;

    return player->spellsActiveChanges.activeSpells.at(spellIndex).params.mEffects.at(effectIndex).mEffectId;
}

int SpellFunctions::GetSpellsActiveEffectArg(unsigned short pid, unsigned int spellIndex, unsigned int effectIndex) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, 0);

    if (spellIndex >= player->spellsActiveChanges.activeSpells.size())
        return 0;

    return player->spellsActiveChanges.activeSpells.at(spellIndex).params.mEffects.at(effectIndex).mArg;
}

double SpellFunctions::GetSpellsActiveEffectMagnitude(unsigned short pid, unsigned int spellIndex, unsigned int effectIndex) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, 0);

    if (spellIndex >= player->spellsActiveChanges.activeSpells.size())
        return 0;

    return player->spellsActiveChanges.activeSpells.at(spellIndex).params.mEffects.at(effectIndex).mMagnitude;
}

double SpellFunctions::GetSpellsActiveEffectDuration(unsigned short pid, unsigned int spellIndex, unsigned int effectIndex) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, 0);

    if (spellIndex >= player->spellsActiveChanges.activeSpells.size())
        return 0;

    return player->spellsActiveChanges.activeSpells.at(spellIndex).params.mEffects.at(effectIndex).mDuration;
}

double SpellFunctions::GetSpellsActiveEffectTimeLeft(unsigned short pid, unsigned int spellIndex, unsigned int effectIndex) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, 0);

    if (spellIndex >= player->spellsActiveChanges.activeSpells.size())
        return 0;

    return player->spellsActiveChanges.activeSpells.at(spellIndex).params.mEffects.at(effectIndex).mTimeLeft;
}

void SpellFunctions::SendSpellbookChanges(unsigned short pid, bool sendToOtherPlayers, bool skipAttachedPlayer) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    mwmp::PlayerPacket *packet = mwmp::Networking::get().getPlayerPacketController()->GetPacket(ID_PLAYER_SPELLBOOK);
    packet->setPlayer(player);

    if (!skipAttachedPlayer)
        packet->Send(false);
    if (sendToOtherPlayers)
        packet->Send(true);
}

void SpellFunctions::SendSpellsActiveChanges(unsigned short pid, bool sendToOtherPlayers, bool skipAttachedPlayer) noexcept
{
    Player* player;
    GET_PLAYER(pid, player, );

    mwmp::PlayerPacket* packet = mwmp::Networking::get().getPlayerPacketController()->GetPacket(ID_PLAYER_SPELLS_ACTIVE);
    packet->setPlayer(player);

    if (!skipAttachedPlayer)
        packet->Send(false);
    if (sendToOtherPlayers)
        packet->Send(true);
}

// All methods below are deprecated versions of methods from above

void SpellFunctions::InitializeSpellbookChanges(unsigned short pid) noexcept
{
    ClearSpellbookChanges(pid);
}
