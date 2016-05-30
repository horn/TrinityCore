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

#include "WorldSession.h"
#include "BattlePetJournal.h"
#include "BattlePetPackets.h"
#include "PetBattleMgr.h"
#include "Player.h"
#include "ObjectMgr.h"

void WorldSession::HandleBattlePetRequestJournal(WorldPackets::BattlePet::BattlePetRequestJournal& /*battlePetRequestJournal*/)
{
    // TODO: Move this to BattlePetMgr::SendJournal() just to have all packets in one file
    WorldPackets::BattlePet::BattlePetJournal battlePetJournal;
    battlePetJournal.Trap = GetBattlePetJournal()->GetTrapLevel();

    for (auto itr : GetBattlePetJournal()->GetLearnedPets())
        battlePetJournal.Pets.push_back(itr.JournalInfo);

    battlePetJournal.Slots = GetBattlePetJournal()->GetSlots();
    SendPacket(battlePetJournal.Write());
}

void WorldSession::HandleBattlePetSetBattleSlot(WorldPackets::BattlePet::BattlePetSetBattleSlot& battlePetSetBattleSlot)
{
    WorldPackets::BattlePet::BattlePetSlot* targetSlot = GetBattlePetJournal()->GetSlot(battlePetSetBattleSlot.Slot);
    if (BattlePetJournal::BattlePet* pet = GetBattlePetJournal()->GetPet(battlePetSetBattleSlot.PetGuid))
    {
        if (targetSlot->Pet)
            for (auto& slot : GetBattlePetJournal()->GetSlots())
                if (slot.Pet && slot.Pet->Guid == battlePetSetBattleSlot.PetGuid)
                {
                    GetBattlePetJournal()->GetSlot(slot.Index)->Pet = targetSlot->Pet;
                    break;
                }

        targetSlot->Pet = &pet->JournalInfo;
    }
    else if (battlePetSetBattleSlot.PetGuid.IsEmpty()) // we can set a slot to be empty using macro
        targetSlot->Pet = nullptr;
}

void WorldSession::HandleBattlePetModifyName(WorldPackets::BattlePet::BattlePetModifyName& battlePetModifyName)
{
    if (BattlePetJournal::BattlePet* pet = GetBattlePetJournal()->GetPet(battlePetModifyName.PetGuid))
    {
        pet->JournalInfo.Name = battlePetModifyName.Name;

        if (pet->SaveInfo != BATTLE_PET_NEW)
            pet->SaveInfo = BATTLE_PET_CHANGED;
    }
}

void WorldSession::HandleBattlePetDeletePet(WorldPackets::BattlePet::BattlePetDeletePet& battlePetDeletePet)
{
    GetBattlePetJournal()->RemovePet(battlePetDeletePet.PetGuid);
}

void WorldSession::HandleBattlePetSetFlags(WorldPackets::BattlePet::BattlePetSetFlags& battlePetSetFlags)
{
    if (BattlePetJournal::BattlePet* pet = GetBattlePetJournal()->GetPet(battlePetSetFlags.PetGuid))
    {
        if (battlePetSetFlags.ControlType == FLAGS_CONTROL_TYPE_APPLY)
            pet->JournalInfo.Flags |= battlePetSetFlags.Flags;
        else // FLAGS_CONTROL_TYPE_REMOVE
            pet->JournalInfo.Flags &= ~battlePetSetFlags.Flags;

        if (pet->SaveInfo != BATTLE_PET_NEW)
            pet->SaveInfo = BATTLE_PET_CHANGED;
    }
}

void WorldSession::HandleCageBattlePet(WorldPackets::BattlePet::CageBattlePet& cageBattlePet)
{
    GetBattlePetJournal()->CageBattlePet(cageBattlePet.PetGuid);
}

void WorldSession::HandleBattlePetSummon(WorldPackets::BattlePet::BattlePetSummon& battlePetSummon)
{
    GetBattlePetJournal()->SummonPet(battlePetSummon.PetGuid);
}

void WorldSession::HandlePetBattleRequestPvp(WorldPackets::BattlePet::PetBattleRequestPvp& petBattleRequestPvp)
{
    if (GetBattlePetJournal()->GetPetBattle())
        return;

    Player* target = ObjectAccessor::FindPlayer(petBattleRequestPvp.TargetGuid);
    if (!target || target->GetSession()->GetBattlePetJournal()->GetPetBattle()) // add "have battle pets unlocked" check
        return;

    PetBattle* battle = sPetBattleMgr.CreatePetBattle(_player, petBattleRequestPvp.TargetGuid, petBattleRequestPvp.LocationInfo);
    if (!battle)
        return;

    WorldPackets::BattlePet::PetBattlePvpChallenge petBattlePvpChallenge;
    petBattlePvpChallenge.ChallengerGuid = _player->GetGUID();
    petBattlePvpChallenge.LocationInfo = petBattleRequestPvp.LocationInfo;

    target->GetSession()->SendPacket(petBattlePvpChallenge.Write());
}

void WorldSession::HandlePetBattleRequestUpdate(WorldPackets::BattlePet::PetBattleRequestUpdate& petBattleRequestUpdate)
{
    PetBattle* battle = GetBattlePetJournal()->GetPetBattle();
    if (!battle)
        return;

    if (petBattleRequestUpdate.Canceled)
    {
        sPetBattleMgr.DestroyPetBattle(battle);

        if (Player* challenger = ObjectAccessor::FindPlayer(petBattleRequestUpdate.TargetGUID))
        {
            WorldPackets::BattlePet::PetBattleRequestFailed petBattleRequestFailed;
            petBattleRequestFailed.Reason = 12; // TODO: enum
            challenger->GetSession()->SendPacket(petBattleRequestFailed.Write());
        }
    }
    else
    {
        battle->StartBattle();
        /*if (Player* challenger = ObjectAccessor::FindPlayer(petBattleRequestUpdate.TargetGUID))
            challenger->SetRooted(true);
        _player->SetRooted(true);*/
    }
}

void WorldSession::HandlePetBattleRequestWild(WorldPackets::BattlePet::PetBattleRequestWild& petBattleRequestWild)
{
    if (GetBattlePetJournal()->GetPetBattle())
        return;

    PetBattle* battle = sPetBattleMgr.CreatePetBattle(_player, petBattleRequestWild.TargetGuid, petBattleRequestWild.LocationInfo);
    if (!battle)
        return;

    battle->StartBattle();
    //_player->SetRooted(true);
}

void WorldSession::HandlePetBattleFinalNotify(WorldPackets::BattlePet::PetBattleFinalNotify& /*petBattleFinalNotify*/)
{
    // SMSG_PET_BATTLE_FINISHED
    WorldPackets::BattlePet::PetBattleFinished finished;
    SendPacket(finished.Write());
    //_player->SetRooted(false);
}

void WorldSession::HandlePetBattleInput(WorldPackets::BattlePet::PetBattleInput& petBattleInput)
{
    PetBattle* battle = GetBattlePetJournal()->GetPetBattle();
    if (!battle)
        return;

    switch (PetBattleMoveType(petBattleInput.MoveType))
    {
        case PETBATTLE_MOVE_PET_SWAP:
            battle->SwapPet(_player, uint8(petBattleInput.NewFrontPet));
            break;
        case PETBATTLE_MOVE_REQUEST_LEAVE:
            battle->ForfeitBattle(_player);
            break;
        case PETBATTLE_MOVE_LEAVE_BATTLE: // how to react on this?
            break;
        case PETBATTLE_MOVE_ABILITY:
            battle->UseAbility(_player, petBattleInput.AbilityID);
            break;
        case PETBATTLE_MOVE_CAGE:
            battle->UseAbility(_player, TrapSpells[GetBattlePetJournal()->GetTrapLevel()]);
            break;
        default:
            break;
    }
}

void WorldSession::HandlePetBattleQuitNotify(WorldPackets::BattlePet::PetBattleQuitNotify& /*petBattleQuitNotify*/)
{
    PetBattle* battle = GetBattlePetJournal()->GetPetBattle();
    if (!battle)
        return;

    WorldPackets::BattlePet::PetBattleFinished finished;
    battle->NotifyParticipants(finished.Write());
    
   /*for (auto& slot : GetBattlePetMgr()->GetSlots())
        slot.Pet.Health = uint32(ceil(slot.Pet.Health / battle->GetForfeitPenalty()));*/

    sPetBattleMgr.DestroyPetBattle(battle);
}

void WorldSession::HandlePetBattleReplaceFrontPet(WorldPackets::BattlePet::PetBattleReplaceFrontPet& petBattleReplaceFrontPet)
{
    if (PetBattle* battle = GetBattlePetJournal()->GetPetBattle())
        battle->SwapPet(_player, petBattleReplaceFrontPet.FrontPet);
}
