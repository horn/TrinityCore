/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "BattlePetMgr.h"
#include "Creature.h"
#include "ObjectAccessor.h"
#include "PetBattle.h"
#include "Player.h"

 // maybe more different ctors would be better (Player vs. Player, Player vs. Creature etc.)
PetBattle::PetBattle(Player* player, ObjectGuid target, WorldPackets::BattlePet::LocationInfo locationInfo)
{
    uint8 PBOID = 0;

    _locationInfo = locationInfo; // TODO: verify location, cancel battle in case of invalid location etc.

    _participants[0].player = player;
    _participants[0].playerUpdate = GetPlayerUpdateInfo(player, PBOID); // TODO: each player must have at least one battle pet in slot
    player->GetSession()->GetBattlePetMgr()->SetPetBattle(this);

    PBOID = 3;

    if (target.IsPlayer())
    {
        // CMSG_PET_BATTLE_REQUEST_PVP (battle pet duel) or Find Battle
        if (Player* opponent = ObjectAccessor::FindPlayer(target))
        {
            _participants[1].player = opponent;
            _participants[1].playerUpdate = GetPlayerUpdateInfo(opponent, PBOID);
            opponent->GetSession()->GetBattlePetMgr()->SetPetBattle(this);
        }

        _isPvP = true;
        _forfeitPenalty = 0; // TODO: make this configurable
    }
    /*else
    {
        if (BattlePetSpeciesEntry const* species = sDB2Manager.GetBattlePetSpeciesByCreatureId(target.GetEntry()))
        {
            // CMSG_PET_BATTLE_REQUEST_WILD or spell casts (from spellclick - menagerie, from gossip - tamers, Kura etc.)
            // fake player - NYI
            // generate pet teams (prepared or random) - next big sql awaits, yay!
            // find out what to do with BattlePetNPCTeamMember.db2
            WorldPackets::BattlePet::PlayerUpdate update;
            init.InitialWildPetGuid = target;
            init.ForfeitPenalty = 10; // TODO: make this configurable
            if (Creature* wildPet = ObjectAccessor::GetCreature(*player, target))
            {
                // TODO: make teams
                WorldPackets::BattlePet::PetBattlePetUpdateInfo pet;
                pet.JournalInfo->Species = species->ID;
                pet.JournalInfo->CreatureID = wildPet->GetDisplayId(); // creature id or display id? figure out this mess
                pet.JournalInfo->CollarID = 0; // unknown
                pet.JournalInfo->Level = wildPet->GetUInt32Value(UNIT_FIELD_WILD_BATTLEPET_LEVEL); // TODO: add this field to wild pets
                update.Pets.push_back(pet);
            }
        }
    }*/

    PBOID = 6;
}

PetBattle::~PetBattle()
{
    _participants[0].player->GetSession()->GetBattlePetMgr()->SetPetBattle(nullptr);

    if (_isPvP)
        _participants[1].player->GetSession()->GetBattlePetMgr()->SetPetBattle(nullptr);
}

WorldPackets::BattlePet::PlayerUpdate PetBattle::GetPlayerUpdateInfo(Player* player, uint8& PBOID)
{
    BattlePetMgr* battlePetMgr = player->GetSession()->GetBattlePetMgr();

    WorldPackets::BattlePet::PlayerUpdate update;

    update.Guid = player->GetGUID();
    update.TrapAbilityID = TrapSpells[battlePetMgr->GetTrapLevel()];
    update.TrapStatus = 6; // ?
    update.RoundTimeSecs = 30;
    update.InputFlags = 8; // ?

    uint8 slotId = 0;
    for (auto slot : battlePetMgr->GetSlots())
    {
        if (!slot.Locked && !slot.Pet.Guid.IsEmpty())
        {
            BattlePetMgr::BattlePet* pet = battlePetMgr->GetPet(slot.Pet.Guid);
            if (!pet || pet->JournalInfo.Health == 0) // pet is dead
                continue;

            pet->UpdateInfo = WorldPackets::BattlePet::PetBattlePetUpdateInfo(); // re-initialize update info to remove old values (TODO: consider other ways)
            pet->UpdateInfo.Slot = slotId++;
            pet->UpdateInfo.JournalInfo = &pet->JournalInfo;

            uint8 abilitySlotId = 0;
            for (auto abilityId : pet->GetActiveAbilities())
            {
                WorldPackets::BattlePet::BattlePetAbility ability;
                ability.Id = abilityId;
                ability.Slot = abilitySlotId++;
                ability.PBOID = PBOID;
                pet->UpdateInfo.Abilities.push_back(ability);
            }

            pet->UpdateInfo.States[STATE_STAT_POWER] = pet->JournalInfo.Power;
            pet->UpdateInfo.States[STATE_STAT_STAMINA] = pet->GetBaseStateValue(STATE_STAT_STAMINA); // from max or current health?
            pet->UpdateInfo.States[STATE_STAT_SPEED] = pet->GetBaseStateValue(STATE_STAT_SPEED);
            pet->UpdateInfo.States[STATE_STAT_CRIT_CHANCE] = 5; // 5 seems to be default value
            // probably add proper family check here (pet->GetFamily() != BATTLE_PET_FAMILY_MAX)
            pet->UpdateInfo.States[FamilyStates[pet->GetFamily()]] = 1; // STATE_PASSIVE_FAMILYTYPE
            // add other states (pve enemies)
            // fill auras and other stuff
            update.Pets.push_back(pet->UpdateInfo);
            PBOID++;
        }
    }

    return update;
}

void PetBattle::StartBattle()
{
    WorldPackets::BattlePet::PetBattleFinalizeLocation finalLoc;
    finalLoc.LocationInfo = _locationInfo;
    NotifyParticipants(finalLoc.Write());

    WorldPackets::BattlePet::PetBattleInitialUpdate init;

    init.PlayerUpdate[0] = _participants[0].playerUpdate;
    init.PlayerUpdate[1] = _participants[1].playerUpdate;

    // TODO: send enviros and set other fields properly

    init.WaitingForFrontPetsMaxSecs = 30;
    init.PvpMaxRoundTime = 30;
    init.CurrentPetBattleState = 1; // ?
    init.CurrentRound = 0;
    init.ForfeitPenalty = _forfeitPenalty;

    NotifyParticipants(init.Write());
}

void PetBattle::Update(Player* player, uint8 frontPet)
{
    uint8 playerId = (player == _participants[0].player) ? 0 : 1;

    if (_participants[playerId].roundCompleted)
        return;

    _roundResult.NextPetBattleState = 2;

    WorldPackets::BattlePet::PetBattleEffect eff;
    eff.PetBattleEffectType = 4;
    eff.CasterPBOID = frontPet + (playerId ? 3 : 0); // "second" player's pboid starts at 3
    WorldPackets::BattlePet::PetBattleEffectTarget tar;
    tar.Petx = eff.CasterPBOID;
    eff.Targets.push_back(tar);
    _roundResult.Effects.push_back(eff);

    _participants[playerId].roundCompleted = true;

    if (!_participants[0].roundCompleted || !_participants[1].roundCompleted)
        return;

    if (!_round)
    {
        _roundResult.RoundTimeSecs[0] = 30;
        _roundResult.RoundTimeSecs[1] = 30;

        WorldPackets::BattlePet::PetBattleFirstRound petBattleFirstRound;
        petBattleFirstRound.RoundResult = _roundResult;
        NotifyParticipants(petBattleFirstRound.Write());
        //return;
    }
    else
        return;

    _participants[0].roundCompleted = false;
    _participants[1].roundCompleted = false;
    _round++;
}

void PetBattle::NotifyParticipants(const WorldPacket* packet)
{
    // some checks needed?
    _participants[0].player->GetSession()->SendPacket(packet);

    if (_isPvP)
        _participants[1].player->GetSession()->SendPacket(packet);
}
