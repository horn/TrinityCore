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

#include "PetBattle.h"
#include "BattlePetMgr.h"
#include "ObjectAccessor.h"

void PetBattle::Initialize()
{
    std::cout << "created instance for player " << _invoker->GetName() << std::endl;

    WorldPackets::BattlePet::PetBattleInitialUpdate init;
    init.PlayerUpdate[0] = _invoker->GetSession()->GetBattlePetMgr()->GetPlayerUpdateInfo();

    if (_target.IsPlayer())
    {
        // CMSG_PET_BATTLE_REQUEST_PVP (battle pet duel) or Find Battle
        // TODO: send packet to the opponent too
        if (Player* opponent = ObjectAccessor::FindPlayer(_target))
        {
            init.PlayerUpdate[1] = opponent->GetSession()->GetBattlePetMgr()->GetPlayerUpdateInfo();
            opponent->GetSession()->GetBattlePetMgr()->InitializePetBattle(_invoker->GetGUID());
        }
        init.IsPvp = true;
    }
    else
    {
        // CMSG_PET_BATTLE_REQUEST_WILD or spell casts (from spellclick - menagerie, from gossip - tamers, Kura etc.)
        // fake player - NYI
        // generate pet teams (prepared or random) - next big sql awaits, yay!
        // find out what to do with BattlePetNPCTeamMember.db2
        init.IsPvp = false;
        init.InitialWildPetGuid = _target;
        init.ForfeitPenalty = 10;
        if (Creature* wildPet = ObjectAccessor::GetCreature(*_invoker, _target))
            init.PlayerUpdate[1] = _invoker->GetSession()->GetBattlePetMgr()->GetWildPetUpdateInfo(wildPet);
    }

    // TODO: send enviros and set other fields properly

    init.WaitingForFrontPetsMaxSecs = 30;
    init.PvpMaxRoundTime = 30;
    init.CurrentPetBattleState = 1; // ?
    init.CurrentRound = 0;

    _invoker->GetSession()->SendPacket(init.Write());
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
