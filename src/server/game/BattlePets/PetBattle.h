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

enum ParticipantId
{
    CHALLENGER          = 0,
    OPPONENT            = 1,

    PARTICIPANTS_COUNT  = 2
};

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
    PETBATTLE_MOVE_REQUEST_LEAVE = 0, // forfeit, sent together with CMSG_PET_BATTLE_QUIT_NOTIFY
    PETBATTLE_MOVE_ABILITY       = 1,
    PETBATTLE_MOVE_PET_SWAP      = 2, // swap pet and pass round
    PETBATTLE_MOVE_CAGE          = 3,
    PETBATTLE_MOVE_LEAVE_BATTLE  = 4  // battle end after SMSG_PET_BATTLE_FINAL_ROUND, sent together with CMSG_PET_BATTLE_FINAL_NOTIFY
};

enum PetBattleEffectType
{
    PETBATTLE_EFFECT_TYPE_SET_HEALTH            = 0,  // PET_BATTLE_EFFECT_TARGET_EX_PET
    PETBATTLE_EFFECT_TYPE_AURA_APPLY            = 1,  // PET_BATTLE_EFFECT_TARGET_EX_AURA
    PETBATTLE_EFFECT_TYPE_AURA_CANCEL           = 2,  // PET_BATTLE_EFFECT_TARGET_EX_AURA
    PETBATTLE_EFFECT_TYPE_AURA_CHANGE           = 3,  // PET_BATTLE_EFFECT_TARGET_EX_AURA
    PETBATTLE_EFFECT_TYPE_PET_SWAP              = 4,  // PET_BATTLE_EFFECT_TARGET_EX_NONE
    PETBATTLE_EFFECT_TYPE_STATUS_CHANGE         = 5,  // PET_BATTLE_EFFECT_TARGET_EX_STAT_CHANGE
    PETBATTLE_EFFECT_TYPE_SET_STATE             = 6,  // PET_BATTLE_EFFECT_TARGET_EX_STATE
    PETBATTLE_EFFECT_TYPE_SET_MAX_HEALTH        = 7,  // PET_BATTLE_EFFECT_TARGET_EX_STAT_CHANGE
    PETBATTLE_EFFECT_TYPE_SET_SPEED             = 8,  // PET_BATTLE_EFFECT_TARGET_EX_STAT_CHANGE
    PETBATTLE_EFFECT_TYPE_SET_POWER             = 9,  // no idea which spells could use that
    PETBATTLE_EFFECT_TYPE_TRIGGER_ABILITY       = 10, // PET_BATTLE_EFFECT_TARGET_EX_TRIGGER_ABILITY
    PETBATTLE_EFFECT_TYPE_ABILITY_CHANGE        = 11, // probably for Dark Simulacrum which no longer exists
    PETBATTLE_EFFECT_TYPE_NPC_EMOTE             = 12, // PET_BATTLE_EFFECT_TARGET_EX_NPC_EMOTE
    PETBATTLE_EFFECT_TYPE_AURA_PROCESSING_BEGIN = 13, // PET_BATTLE_EFFECT_TARGET_EX_NONE
    PETBATTLE_EFFECT_TYPE_AURA_PROCESSING_END   = 14, // PET_BATTLE_EFFECT_TARGET_EX_NONE
    PETBATTLE_EFFECT_TYPE_INVALID               = 15  // not sure about the value
};

// 6.2.4
enum PetBattleEffectTargetEx
{
    PET_BATTLE_EFFECT_TARGET_EX_NONE            = 0,
    PET_BATTLE_EFFECT_TARGET_EX_AURA            = 1,
    PET_BATTLE_EFFECT_TARGET_EX_STATE           = 2,
    PET_BATTLE_EFFECT_TARGET_EX_PET             = 3, // ?
    PET_BATTLE_EFFECT_TARGET_EX_STAT_CHANGE     = 4,
    PET_BATTLE_EFFECT_TARGET_EX_TRIGGER_ABILITY = 5,
    PET_BATTLE_EFFECT_TARGET_EX_ABILITY_CHANGE  = 6,
    PET_BATTLE_EFFECT_TARGET_EX_NPC_EMOTE       = 7
};

PetBattleEffectTargetEx const targetExByType[PETBATTLE_EFFECT_TYPE_INVALID]
{
    PET_BATTLE_EFFECT_TARGET_EX_PET,
    PET_BATTLE_EFFECT_TARGET_EX_AURA,
    PET_BATTLE_EFFECT_TARGET_EX_AURA,
    PET_BATTLE_EFFECT_TARGET_EX_AURA,
    PET_BATTLE_EFFECT_TARGET_EX_NONE,
    PET_BATTLE_EFFECT_TARGET_EX_STAT_CHANGE,
    PET_BATTLE_EFFECT_TARGET_EX_STATE,
    PET_BATTLE_EFFECT_TARGET_EX_STAT_CHANGE,
    PET_BATTLE_EFFECT_TARGET_EX_STAT_CHANGE,
    PET_BATTLE_EFFECT_TARGET_EX_STAT_CHANGE,     // not verified
    PET_BATTLE_EFFECT_TARGET_EX_TRIGGER_ABILITY,
    PET_BATTLE_EFFECT_TARGET_EX_ABILITY_CHANGE,  // not verified
    PET_BATTLE_EFFECT_TARGET_EX_NPC_EMOTE,
    PET_BATTLE_EFFECT_TARGET_EX_NONE,
    PET_BATTLE_EFFECT_TARGET_EX_NONE
};

class PetBattleAbility;

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
        uint32 roundTime = 0;
        uint32 abilityId = 0;
    };

    struct PetBattleObject
    {
        // PetBattleUpdateInfo::States should be enough to hold all info during the battle.
        // If not, replace UpdateInfo with variables.
        WorldPackets::BattlePet::PetBattlePetUpdateInfo UpdateInfo;

        void DealDamage(PetBattleObject* target, uint32 points);
        void DealHeal(PetBattleObject* target, uint32 points);

        std::vector<PetBattleAbility*> Auras;
    };

    PetBattle(Player* player, ObjectGuid target, WorldPackets::BattlePet::LocationInfo locationInfo);
    ~PetBattle();

    void StartBattle();
    void EndBattle(uint8 winner, bool forfeit);
    void Update(uint32 diff);
    void ProcessRound();
    void EndRound();
    void SwapPet(Player* player, uint8 frontPet);
    void ForfeitBattle(Player* player);
    void UseAbility(Player* player, uint32 ability);

    WorldPackets::BattlePet::PetBattleEffect* AddEffect(PetBattleEffectType type, PetBattleObject* caster = nullptr, uint32 effectId = 0, uint16 flags = 0, uint16 sourceAuraInstanceId = 0, uint16 turnInstanceId = 0, uint8 stackDepth = 0);
    WorldPackets::BattlePet::PetBattleEffectTarget* AddEffectTarget(WorldPackets::BattlePet::PetBattleEffect* effect, PetBattleObject* target = nullptr, int32 param0 = 0, int32 param1 = 0, int32 param2 = 0, int32 param3 = 0);

    PetBattleObject* GetPetBattleObject(PBOIDNames pboid) { return &_objects[pboid]; }
    PBOIDNames GetPetBattleObjectId(PetBattleObject* object);

    WorldPackets::BattlePet::LocationInfo GetLocationInfo() const { return _locationInfo; }
    uint8 GetForfeitPenalty() const { return _forfeitPenalty; }

    void NotifyParticipants(const WorldPacket* packet);

private:
    Participant _participants[2];
    std::array<PetBattleObject, PBOID_INVALID> _objects;
    std::list<PetBattleAbility> _abilities;

    WorldPackets::BattlePet::LocationInfo _locationInfo;
    bool _isPvP = false;
    uint8 _forfeitPenalty = 0;

    WorldPackets::BattlePet::RoundResult _roundResult;
    uint8 _round = 0;

    WorldPackets::BattlePet::PlayerUpdate GetPlayerUpdateInfo(Player* player, uint8& PBOID);
    WorldPackets::BattlePet::PlayerUpdate GetCreatureUpdateInfo(Creature* creature, uint8& PBOID);
};

#endif // PetBattle_h__
