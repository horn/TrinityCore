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

#include "BattlePetPackets.h"

class PetBattle
{
public:
    struct Participant
    {
        union
        {
            Player* player;
            Creature* creature;
        };

        WorldPackets::BattlePet::PlayerUpdate playerUpdate;
    };

    PetBattle(Player* player, ObjectGuid target, WorldPackets::BattlePet::LocationInfo locationInfo);

    void StartBattle();
    void Update(uint8 frontPet);

    WorldPackets::BattlePet::LocationInfo GetLocationInfo() const { return _locationInfo; }

    void NotifyParticipants(const WorldPacket* packet);

private:
    Participant _participants[2];
    WorldPackets::BattlePet::LocationInfo _locationInfo;

    WorldPackets::BattlePet::PlayerUpdate GetPlayerUpdateInfo(Player* player, uint8& PBOID);

    /* PBOID
       1 - player1, pet1
       2 - player1, pet2
       3 - player1, pet3
       4 - player2, pet1
       5 - player2, pet2
       6 - player2, pet3
       7 - ? (some kind of opponent team object - for example aura 1377)
       8 - weather
       9 - ? (effects 13 and 14)
       ... any others?
    */

    bool _isPvP = false;
    uint8 _round = 0;
};

#endif // PetBattle_h__
