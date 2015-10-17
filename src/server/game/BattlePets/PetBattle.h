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

#ifndef PetBattle_h__
#define PetBattle_h__

#include "Player.h"

class PetBattle
{
public:
    PetBattle(Player* invoker, ObjectGuid target) : _invoker(invoker), _target(target) { }

    Player* GetBattleInvoker() const { return _invoker; }

    void Update(uint8 frontPet);

    void SendFirstRound();
    void SendReplacementsMade();

private:
    Player* _invoker;
    ObjectGuid _target;

    bool _isPvP = false;
    uint8 _round = 0;
};

#endif // PetBattle_h__
