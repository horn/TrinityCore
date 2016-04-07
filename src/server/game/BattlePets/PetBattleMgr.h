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

#ifndef PetBattleMgr_h__
#define PetBattleMgr_h__

#include "PetBattle.h"

class PetBattleMgr
{
public:
    static PetBattleMgr& Instance();

    void Update(uint32 diff);

    PetBattle* CreatePetBattle(Player* player, ObjectGuid target, WorldPackets::BattlePet::LocationInfo locationInfo);
    void DestroyPetBattle(PetBattle* battle);

private:
    std::list<PetBattle*> _battles;
};

#define sPetBattleMgr PetBattleMgr::Instance()

#endif // PetBattleMgr_h__
