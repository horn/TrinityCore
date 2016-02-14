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

#include "BattlePetPackets.h"

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
    PETBATTLE_USE_ABILITY       = 1,
    PETBATTLE_CHANGE_PET        = 2, // swap pet and pass round
    PETBATTLE_MOVE_TYPE_UNK2    = 3, // i guess this will be used for cage ability
    PETBATTLE_MOVE_TYPE_UNK3    = 4  // battle end after SMSG_PET_BATTLE_FINAL_ROUND, sent together with CMSG_PET_BATTLE_FINAL_NOTIFY

    // ChangePet
    // UseAbility
    // UseTrap
    // SkipTurn
    // ForfeitGame
};

// TODO: new class BattlePetAbility
enum PetBattleEffectTargetEx
{
    PET_BATTLE_EFFECT_TARGET_EX_NONE            = 0, // not sure about the value
    PET_BATTLE_EFFECT_TARGET_EX_NPC_EMOTE       = 1,
    PET_BATTLE_EFFECT_TARGET_EX_AURA            = 2,
    PET_BATTLE_EFFECT_TARGET_EX_STAT_CHANGE     = 3,
    PET_BATTLE_EFFECT_TARGET_EX_PET             = 4, // ?
    PET_BATTLE_EFFECT_TARGET_EX_ABILITY_CHANGE  = 5,
    PET_BATTLE_EFFECT_TARGET_EX_TRIGGER_ABILITY = 6,
    PET_BATTLE_EFFECT_TARGET_EX_STATE           = 7
};

enum PetBattleEffectType
{
    PETBATTLE_EFFECT_TYPE_SET_HEALTH            = 0,
    PETBATTLE_EFFECT_TYPE_AURA_APPLY            = 1,
    PETBATTLE_EFFECT_TYPE_AURA_CANCEL           = 2,
    PETBATTLE_EFFECT_TYPE_AURA_CHANGE           = 3,
    PETBATTLE_EFFECT_TYPE_PET_SWAP              = 4,
    PETBATTLE_EFFECT_TYPE_STATUS_CHANGE         = 5,
    PETBATTLE_EFFECT_TYPE_SET_STATE             = 6,
    PETBATTLE_EFFECT_TYPE_SET_MAX_HEALTH        = 7,
    PETBATTLE_EFFECT_TYPE_SET_SPEED             = 8,
    PETBATTLE_EFFECT_TYPE_SET_POWER             = 9,
    PETBATTLE_EFFECT_TYPE_TRIGGER_ABILITY       = 10,
    PETBATTLE_EFFECT_TYPE_ABILITY_CHANGE        = 11,
    PETBATTLE_EFFECT_TYPE_NPC_EMOTE             = 12,
    PETBATTLE_EFFECT_TYPE_AURA_PROCESSING_BEGIN = 13, // or AURAS_BEGIN
    PETBATTLE_EFFECT_TYPE_AURA_PROCESSING_END   = 14, // or AURAS_END
    PETBATTLE_EFFECT_TYPE_INVALID               = 15  // not sure about the value
};

enum AbilityProcType
{
    PET_BATTLE_EVENT_ON_APPLY           = 0,
    PET_BATTLE_EVENT_ON_DAMAGE_TAKEN    = 1,
    PET_BATTLE_EVENT_ON_DAMAGE_DEALT    = 2,
    PET_BATTLE_EVENT_ON_HEAL_TAKEN      = 3,
    PET_BATTLE_EVENT_ON_HEAL_DEALT      = 4,
    PET_BATTLE_EVENT_ON_AURA_REMOVED    = 5,
    PET_BATTLE_EVENT_ON_ROUND_START     = 6,
    PET_BATTLE_EVENT_ON_ROUND_END       = 7,
    PET_BATTLE_EVENT_ON_TURN            = 8,
    PET_BATTLE_EVENT_ON_ABILITY         = 9,
    PET_BATTLE_EVENT_ON_SWAP_IN         = 10,
    PET_BATTLE_EVENT_ON_SWAP_OUT        = 11
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
    void RegisterMove(uint8 playerId);
    void SwapPet(Player* player, uint8 frontPet);
    void ForfeitBattle(Player* player);

    WorldPackets::BattlePet::LocationInfo GetLocationInfo() const { return _locationInfo; }
    uint8 GetForfeitPenalty() const { return _forfeitPenalty; }

    void NotifyParticipants(const WorldPacket* packet);

private:
    Participant _participants[2];

    WorldPackets::BattlePet::LocationInfo _locationInfo;
    bool _isPvP = false;
    uint8 _forfeitPenalty = 0;

    WorldPackets::BattlePet::RoundResult _roundResult;
    uint8 _round = 0;

    WorldPackets::BattlePet::PlayerUpdate GetPlayerUpdateInfo(Player* player, uint8& PBOID);
    WorldPackets::BattlePet::PlayerUpdate GetCreatureUpdateInfo(Creature* creature, uint8& PBOID);
};

#endif // PetBattle_h__
