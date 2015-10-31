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
#include "BattlePetMgr.h"
#include "BattlePetPackets.h"
#include "Player.h"

void WorldSession::HandleBattlePetRequestJournal(WorldPackets::BattlePet::BattlePetRequestJournal& /*battlePetRequestJournal*/)
{
    // TODO: Move this to BattlePetMgr::SendJournal() just to have all packets in one file
    WorldPackets::BattlePet::BattlePetJournal battlePetJournal;
    battlePetJournal.Trap = GetBattlePetMgr()->GetTrapLevel();

    for (auto itr : GetBattlePetMgr()->GetLearnedPets())
        battlePetJournal.Pets.push_back(itr.JournalInfo);

    battlePetJournal.Slots = GetBattlePetMgr()->GetSlots();
    SendPacket(battlePetJournal.Write());
}

void WorldSession::HandleBattlePetSetBattleSlot(WorldPackets::BattlePet::BattlePetSetBattleSlot& battlePetSetBattleSlot)
{
    if (BattlePetMgr::BattlePet* pet = GetBattlePetMgr()->GetPet(battlePetSetBattleSlot.PetGuid))
        GetBattlePetMgr()->GetSlot(battlePetSetBattleSlot.Slot)->Pet = pet->JournalInfo;
}

void WorldSession::HandleBattlePetModifyName(WorldPackets::BattlePet::BattlePetModifyName& battlePetModifyName)
{
    if (BattlePetMgr::BattlePet* pet = GetBattlePetMgr()->GetPet(battlePetModifyName.PetGuid))
    {
        pet->JournalInfo.Name = battlePetModifyName.Name;

        if (pet->SaveInfo != BATTLE_PET_NEW)
            pet->SaveInfo = BATTLE_PET_CHANGED;
    }
}

void WorldSession::HandleBattlePetDeletePet(WorldPackets::BattlePet::BattlePetDeletePet& battlePetDeletePet)
{
    GetBattlePetMgr()->RemovePet(battlePetDeletePet.PetGuid);
}

void WorldSession::HandleBattlePetSetFlags(WorldPackets::BattlePet::BattlePetSetFlags& battlePetSetFlags)
{
    if (BattlePetMgr::BattlePet* pet = GetBattlePetMgr()->GetPet(battlePetSetFlags.PetGuid))
    {
        if (battlePetSetFlags.ControlType == 2) // 2 - apply
            pet->JournalInfo.Flags |= battlePetSetFlags.Flags;
        else                                    // 3 - remove
            pet->JournalInfo.Flags &= ~battlePetSetFlags.Flags;

        if (pet->SaveInfo != BATTLE_PET_NEW)
            pet->SaveInfo = BATTLE_PET_CHANGED;
    }
}

void WorldSession::HandleCageBattlePet(WorldPackets::BattlePet::CageBattlePet& cageBattlePet)
{
    GetBattlePetMgr()->CageBattlePet(cageBattlePet.PetGuid);
}

void WorldSession::HandleBattlePetSummon(WorldPackets::BattlePet::BattlePetSummon& battlePetSummon)
{
    GetBattlePetMgr()->SummonPet(battlePetSummon.PetGuid);
}

void WorldSession::HandlePetBattleRequestPvp(WorldPackets::BattlePet::PetBattleRequestPvp& petBattleRequestPvp)
{
    // TODO: handle locations properly
    WorldPackets::BattlePet::PetBattleFinalizeLocation finalLoc;
    finalLoc.LocationInfo = petBattleRequestPvp.LocationInfo;

    SendPacket(finalLoc.Write());

    GetBattlePetMgr()->InitializePetBattle(petBattleRequestPvp.TargetGuid);
}

void WorldSession::HandlePetBattleRequestWild(WorldPackets::BattlePet::PetBattleRequestWild& petBattleRequestWild)
{
    // TODO: handle locations properly
    WorldPackets::BattlePet::PetBattleFinalizeLocation finalLoc;
    finalLoc.LocationInfo = petBattleRequestWild.LocationInfo;

    SendPacket(finalLoc.Write());

    GetBattlePetMgr()->InitializePetBattle(petBattleRequestWild.TargetGuid);
}

void WorldSession::HandlePetBattleFinalNotify(WorldPackets::BattlePet::PetBattleFinalNotify& petBattleFinalNotify)
{
    // SMSG_PET_BATTLE_FINISHED
    WorldPackets::BattlePet::PetBattleFinished finished;
    SendPacket(finished.Write());
}

void WorldSession::HandlePetBattleInput(WorldPackets::BattlePet::PetBattleInput& petBattleInput)
{
    // SMSG_PET_BATTLE_ROUND_RESULT when MoveType = 1
    // SMSG_PET_BATTLE_FINAL_ROUND when MoveType = 4, maybe
}

void WorldSession::HandlePetBattleQuitNotify(WorldPackets::BattlePet::PetBattleQuitNotify& petBattleQuitNotify)
{

}

void WorldSession::HandlePetBattleReplaceFrontPet(WorldPackets::BattlePet::PetBattleReplaceFrontPet& petBattleReplaceFrontPet)
{
    // SMSG_PET_BATTLE_FIRST_ROUND or SMSG_PET_BATTLE_REPLACEMENTS_MADE
    // PetBattleEffectType: 4
    // Effect.Petx = petBattleReplaceFrontPet.FrontPet

    if (PetBattle* battle = GetBattlePetMgr()->GetPetBattle())
        battle->Update(petBattleReplaceFrontPet.FrontPet);
}

void WorldSession::HandlePetBattleRequestUpdate(WorldPackets::BattlePet::PetBattleRequestUpdate& petBattleRequestPvp)
{
    // not sure for what is it used
}
