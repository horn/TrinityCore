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

#ifndef PetBattle_h__
#define PetBattle_h__

#include "PetBattleAbility.h"

enum PBOIDNames
{
    PBOID_P0_PET_0  = 0,
    PBOID_P0_PET_1  = 1,
    PBOID_P0_PET_2  = 2,
    PBOID_P1_PET_0  = 3,
    PBOID_P1_PET_1  = 4,
    PBOID_P1_PET_2  = 5,
    PBOID_PAD_0     = 6,
    PBOID_PAD_1     = 7,
    PBOID_WEATHER   = 8,
    PBOID_INVALID   = 9
};

// custom names, can't find them in client
enum PetBattleMoveType
{
    PETBATTLE_MOVE_TYPE_UNK1    = 0, // forfeit, sent together with CMSG_PET_BATTLE_QUIT_NOTIFY
    PETBATTLE_MOVE_ABILITY      = 1,
    PETBATTLE_MOVE_PET_SWAP     = 2, // swap pet and pass round
    PETBATTLE_MOVE_CAGE         = 3,
    PETBATTLE_MOVE_TYPE_UNK3    = 4  // battle end after SMSG_PET_BATTLE_FINAL_ROUND, sent together with CMSG_PET_BATTLE_FINAL_NOTIFY
};

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

        bool roundCompleted = false;
    };

    PetBattle(Player* player, ObjectGuid target, WorldPackets::BattlePet::LocationInfo locationInfo);
    ~PetBattle();

    void StartBattle();
    void EndBattle(uint8 winner, bool forfeit);
    void Update(uint32 diff);
    void EndRound();
    void SwapPet(Player* player, uint8 frontPet);
    void ForfeitBattle(Player* player);
    void UseAbility(Player* player, uint32 ability);

    WorldPackets::BattlePet::PetBattlePetUpdateInfo* GetPetBattleObject(PBOIDNames pboid) { return &_objects[pboid]; }

    WorldPackets::BattlePet::LocationInfo GetLocationInfo() const { return _locationInfo; }
    uint8 GetForfeitPenalty() const { return _forfeitPenalty; }

    void NotifyParticipants(const WorldPacket* packet);

private:
    Participant _participants[2];
    // PetBattleUpdateInfo::States should be enough to hold all info during the battle.
    // If not, replace PetBattleUpdateInfo::JournalInfo with variables and/or make a wrapper struct.
    WorldPackets::BattlePet::PetBattlePetUpdateInfo _objects[PBOID_INVALID];

    WorldPackets::BattlePet::LocationInfo _locationInfo;
    bool _isPvP = false;
    uint8 _forfeitPenalty = 0;

    WorldPackets::BattlePet::RoundResult _roundResult;
    uint8 _round = 0;

    WorldPackets::BattlePet::PlayerUpdate GetPlayerUpdateInfo(Player* player, uint8& PBOID);
    WorldPackets::BattlePet::PlayerUpdate GetCreatureUpdateInfo(Creature* creature, uint8& PBOID);
};

#endif // PetBattle_h__
