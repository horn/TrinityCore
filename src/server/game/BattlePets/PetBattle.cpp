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
#include "PetBattleAbility.h"
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
    update.InputFlags = 8; // ? 8 allows you to choose which battle pet goes first

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

    for (uint8 i = 0; i < (_isPvP ? 2 : 1); ++i)
    {
        _participants[i].player->Dismount();
        // set pacified and disable move flags
        _participants[i].player->MonsterMoveWithSpeed(_locationInfo.PlayerPositions[i].GetPositionX(), _locationInfo.PlayerPositions[i].GetPositionY(),
                                                      _locationInfo.PlayerPositions[i].GetPositionZ(), _participants[i].player->GetSpeed(MOVE_RUN));
        // need to be set AFTER the movement is done.. or make 2 points to run behind the battle position and return here
        _participants[i].player->SetOrientation(_locationInfo.PlayerPositions[i].GetOrientation());
        init.PlayerUpdate[i] = _participants[i].playerUpdate;
    }

    // TODO: send enviros and set other fields properly

    init.WaitingForFrontPetsMaxSecs = 30;
    init.PvpMaxRoundTime = 30;
    init.CurrentPetBattleState = 1; // ?
    init.CurrentRound = 0;
    init.ForfeitPenalty = _forfeitPenalty;

    NotifyParticipants(init.Write());

    /*_participants[0].player->SetRooted(true); // inaccessible, must be called from WorldSession
    if (_isPvP)
        _participants[1].player->SetRooted(true);*/
}

// don't call this on battle interrupt (we should only unroot players, send SMSG_PET_BATTLE_FINISHED and delete battle in this case)
void PetBattle::EndBattle(uint8 winner, bool forfeit)
{
    WorldPackets::BattlePet::PetBattleFinalRound finalRound;
    finalRound.Abandoned = forfeit;
    finalRound.PvpBattle = _isPvP;
    finalRound.Winners[winner] = true;

    if (!_isPvP)
        finalRound.NpcCreatureID[1] = _participants[1].creature->GetEntry(); // really? why NpcCreatureID[0] would be here?

    for (uint8 i = 0; i < 2; ++i)
    {
        uint8 PBOID = 0;

        for (auto pet : _participants[i].playerUpdate.Pets)
        {
            WorldPackets::BattlePet::FinalRoundPetInfo petInfo;
            petInfo.Guid = pet.JournalInfo->Guid;
            petInfo.Level = pet.JournalInfo->Level; // this should probably change with xp gain
            petInfo.Xp = pet.JournalInfo->Exp; // awarded, initial or new value (intial + awarded)?
            petInfo.Health = pet.JournalInfo->Health; // in case of pvp no health is lost (even if the pet dies during the battle)
            petInfo.MaxHealth = pet.JournalInfo->MaxHealth; // max or initial?
            petInfo.InitialLevel = pet.JournalInfo->Level;
            petInfo.PBOID = PBOID + (i ? PBOID_P1_PET_0 : PBOID_P0_PET_0);
            petInfo.Caged = false; // TODO
            petInfo.Captured = false; // TODO
            petInfo.SeenAction = false; // TODO
            petInfo.AwardedXP = false; // in case of find battle or pve battle, only if pet did something during the battle
            finalRound.Pets.push_back(petInfo);
        }
    }

    NotifyParticipants(finalRound.Write());

    /*_participants[0].player->SetRooted(false);
    if (_isPvP)
        _participants[1].player->SetRooted(false);*/
}

void PetBattle::RegisterMove(uint8 playerId)
{
    _participants[playerId].roundCompleted = true;

    if (!_participants[0].roundCompleted || !_participants[1].roundCompleted)
        return;

    _roundResult.RoundTimeSecs[0] = 30;
    _roundResult.RoundTimeSecs[1] = 30;
    _roundResult.CurRound = _round;

    if (!_round)
    {
        WorldPackets::BattlePet::PetBattleFirstRound petBattleFirstRound;
        petBattleFirstRound.RoundResult = _roundResult;
        NotifyParticipants(petBattleFirstRound.Write());
    }
    else // TODO: handle last round
    {
        WorldPackets::BattlePet::PetBattleRoundResult petBattleRoundResult;
        petBattleRoundResult.RoundResult = _roundResult;
        NotifyParticipants(petBattleRoundResult.Write());
    }

    _roundResult = WorldPackets::BattlePet::RoundResult(); // reset round result
    _participants[0].roundCompleted = false;
    _participants[1].roundCompleted = false;
    _round++;
}

void PetBattle::SwapPet(Player* player, uint8 frontPet)
{
    uint8 playerId = (player == _participants[0].player) ? 0 : 1;

    if (_participants[playerId].roundCompleted)
        return;

    _roundResult.NextPetBattleState = 2;

    WorldPackets::BattlePet::PetBattleEffect eff;
    eff.PetBattleEffectType = PETBATTLE_EFFECT_TYPE_PET_SWAP;
    WorldPackets::BattlePet::PetBattleEffectTarget tar;
    tar.Type = 0; // TODO: enum
    tar.Petx = (playerId ? PBOID_P1_PET_0 : PBOID_P0_PET_0) + frontPet; // "second" player's pboid starts at 3
    eff.Targets.push_back(tar);
    eff.CasterPBOID = _round ? _participants[playerId].playerUpdate.FrontPet : tar.Petx; // on first round, caster is the same as target

    if (_round && tar.Petx == _participants[playerId].playerUpdate.FrontPet) // pass round
        eff.Flags = 1;

    _participants[playerId].playerUpdate.FrontPet = tar.Petx; // not sure if we can store it here
    _roundResult.Effects.push_back(eff);

    RegisterMove(playerId);
}

void PetBattle::ForfeitBattle(Player* player)
{
    uint8 playerId = (player == _participants[0].player) ? 0 : 1;

    if (_participants[playerId].roundCompleted)
        return; // TODO: in this case we should probably remember that player wants to forfeit battle and do it in next round instead of ignoring it

    RegisterMove(playerId);
}

void PetBattle::UseAbility(Player* player, uint32 ability)
{

}

void PetBattle::NotifyParticipants(const WorldPacket* packet)
{
    // some checks needed?
    _participants[0].player->GetSession()->SendPacket(packet);

    if (_isPvP)
        _participants[1].player->GetSession()->SendPacket(packet);
}
