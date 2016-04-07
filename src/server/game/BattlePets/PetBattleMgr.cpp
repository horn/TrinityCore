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

#include "PetBattleMgr.h"

// TODO: move parts of PetBattle class here
//       make use of ObjectGuid<HighGuid::PetBattle> here probably
//       implement matchmaking (Find Battle)

PetBattleMgr& PetBattleMgr::Instance()
{
    static PetBattleMgr instance;
    return instance;
}

void PetBattleMgr::Update(uint32 diff)
{
    for (auto battle : _battles)
        battle->Update(diff);
}

PetBattle* PetBattleMgr::CreatePetBattle(Player* player, ObjectGuid target, WorldPackets::BattlePet::LocationInfo locationInfo)
{
    PetBattle* battle = new PetBattle(player, target, locationInfo);
    if (!battle)
        return nullptr;

    _battles.push_back(battle);
    return battle;
}

void PetBattleMgr::DestroyPetBattle(PetBattle* battle)
{
    _battles.remove(battle);
    delete battle;
}
