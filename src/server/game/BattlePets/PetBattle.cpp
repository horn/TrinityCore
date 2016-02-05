/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

    PBOID = 3;

    if (target.IsPlayer())
    {
        // CMSG_PET_BATTLE_REQUEST_PVP (battle pet duel) or Find Battle
        if (Player* opponent = ObjectAccessor::FindPlayer(target))
        {
            _participants[1].player = opponent;
            _participants[1].playerUpdate = GetPlayerUpdateInfo(opponent, PBOID);
        }

        _isPvP = true;
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
            init.ForfeitPenalty = 10;
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

WorldPackets::BattlePet::PlayerUpdate PetBattle::GetPlayerUpdateInfo(Player* player, uint8& PBOID)
{
    BattlePetMgr* battlePetMgr = player->GetSession()->GetBattlePetMgr();
    //if (!battlePetMgr) // this shouldn't happen
    //    return;

    WorldPackets::BattlePet::PlayerUpdate update;

    update.Guid = player->GetGUID();
    update.TrapAbilityID = TrapSpells[battlePetMgr->GetTrapLevel()];
    update.TrapStatus = 4; // ?
    update.InputFlags = 6; // ?

    for (auto slot : battlePetMgr->GetSlots())
    {
        if (!slot.Locked && !slot.Pet.Guid.IsEmpty())
        {
            BattlePetMgr::BattlePet* pet = battlePetMgr->GetPet(slot.Pet.Guid);
            if (!pet || pet->JournalInfo.Health == 0) // pet is dead
                continue;

            pet->UpdateInfo.Slot = slot.Index;
            pet->UpdateInfo.JournalInfo = &pet->JournalInfo;

            uint8 slot = 0;
            for (auto abilityId : pet->GetActiveAbilities())
            {
                WorldPackets::BattlePet::BattlePetAbility ability;
                ability.Id = abilityId;
                ability.Slot = slot++;
                ability.PBOID = PBOID;
                pet->UpdateInfo.Abilities.push_back(ability);
            }

            pet->UpdateInfo.States[STATE_STAT_POWER] = pet->JournalInfo.Power;
            pet->UpdateInfo.States[STATE_STAT_STAMINA] = pet->GetBaseStateValue(STATE_STAT_STAMINA); // from max or current health?
            pet->UpdateInfo.States[STATE_STAT_SPEED] = pet->GetBaseStateValue(STATE_STAT_SPEED);
            pet->UpdateInfo.States[STATE_STAT_CRIT_CHANCE] = 5; // 5 seems to be default value
                                                                // probably add proper family check here (pet->GetFamily() != BATTLE_PET_FAMILY_MAX)
            pet->UpdateInfo.States[FamilyStates[pet->GetFamily()]] = 1; // STATE_PASSIVE_FAMILYTYPE
                                                                        // other states (pve enemies)

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

    NotifyParticipants(init.Write());
}

void PetBattle::Update(uint8 frontPet)
{
    /*if (!_round)
    {
        SendFirstRound();
        return;
    }
    if (frontPet)
        SendReplacementsMade();*/
}

void PetBattle::NotifyParticipants(const WorldPacket* packet)
{
    // some checks needed?
    _participants[0].player->GetSession()->SendPacket(packet);

    if (_isPvP)
        _participants[1].player->GetSession()->SendPacket(packet);
}
