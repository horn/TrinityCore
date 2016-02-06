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
#include "ObjectMgr.h"

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
    WorldPackets::BattlePet::BattlePetSlot* targetSlot = GetBattlePetMgr()->GetSlot(battlePetSetBattleSlot.Slot);
    if (BattlePetMgr::BattlePet* pet = GetBattlePetMgr()->GetPet(battlePetSetBattleSlot.PetGuid))
    {
        if (!targetSlot->Pet.Guid.IsEmpty())
            for (auto& slot : GetBattlePetMgr()->GetSlots())
                if (slot.Pet.Guid == battlePetSetBattleSlot.PetGuid)
                {
                    GetBattlePetMgr()->GetSlot(slot.Index)->Pet = targetSlot->Pet;
                    break;
                }

        targetSlot->Pet = pet->JournalInfo;
    }
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
    if (GetBattlePetMgr()->GetPetBattle())
        return;

    Player* target = ObjectAccessor::FindPlayer(petBattleRequestPvp.TargetGuid);
    if (!target || target->GetSession()->GetBattlePetMgr()->GetPetBattle()) // add "have battle pets unlocked" check
        return;

    PetBattle* battle = new PetBattle(_player, petBattleRequestPvp.TargetGuid, petBattleRequestPvp.LocationInfo, 0);
    GetBattlePetMgr()->SetPetBattle(battle);

    WorldPackets::BattlePet::PetBattlePvpChallenge petBattlePvpChallenge;
    petBattlePvpChallenge.ChallengerGuid = _player->GetGUID();
    petBattlePvpChallenge.LocationInfo = petBattleRequestPvp.LocationInfo;

    target->GetSession()->GetBattlePetMgr()->SetPetBattle(battle);
    target->GetSession()->SendPacket(petBattlePvpChallenge.Write());
}

void WorldSession::HandlePetBattleRequestUpdate(WorldPackets::BattlePet::PetBattleRequestUpdate& petBattleRequestUpdate)
{
    if (petBattleRequestUpdate.Canceled)
    {
        if (!GetBattlePetMgr()->GetPetBattle())
            return;

        delete GetBattlePetMgr()->GetPetBattle();
        GetBattlePetMgr()->SetPetBattle(nullptr);

        if (Player* challenger = ObjectAccessor::FindPlayer(petBattleRequestUpdate.TargetGUID))
        {
            challenger->GetSession()->GetBattlePetMgr()->SetPetBattle(nullptr);

            WorldPackets::BattlePet::PetBattleRequestFailed petBattleRequestFailed;
            petBattleRequestFailed.Reason = 12; // TODO: enum
            challenger->GetSession()->SendPacket(petBattleRequestFailed.Write());
        }
    }
    else
    {
        if (PetBattle* battle = GetBattlePetMgr()->GetPetBattle())
            battle->StartBattle();
    }
}

void WorldSession::HandlePetBattleRequestWild(WorldPackets::BattlePet::PetBattleRequestWild& petBattleRequestWild)
{
    if (GetBattlePetMgr()->GetPetBattle())
        return;

    PetBattle* battle = new PetBattle(_player, petBattleRequestWild.TargetGuid, petBattleRequestWild.LocationInfo, 10);
    GetBattlePetMgr()->SetPetBattle(battle);
    battle->StartBattle();
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

void WorldSession::HandlePetBattleQuitNotify(WorldPackets::BattlePet::PetBattleQuitNotify& /*petBattleQuitNotify*/)
{
    PetBattle* battle = GetBattlePetMgr()->GetPetBattle();
    if (!battle)
        return;

    WorldPackets::BattlePet::PetBattleFinished finished;
    battle->NotifyParticipants(finished.Write());
    // pet should lost 10% hp
    battle->DestroyBattle();
}

void WorldSession::HandlePetBattleReplaceFrontPet(WorldPackets::BattlePet::PetBattleReplaceFrontPet& petBattleReplaceFrontPet)
{
    // SMSG_PET_BATTLE_FIRST_ROUND or SMSG_PET_BATTLE_REPLACEMENTS_MADE
    // PetBattleEffectType: 4
    // Effect.Petx = petBattleReplaceFrontPet.FrontPet

    if (PetBattle* battle = GetBattlePetMgr()->GetPetBattle())
        battle->Update(petBattleReplaceFrontPet.FrontPet);
}
