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

#ifndef PetBattleAbility_h__
#define PetBattleAbility_h__

#include <unordered_map>

enum PetBattleAbilityProcType
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
    PET_BATTLE_EVENT_ON_SWAP_OUT        = 11,
    PET_BATTLE_EVENT_UNK                = 12  // Feign Death only
};

// custom
enum PetBattleAbilityEffectName
{
    EFFECT_DO_NOTHING                       = 22,
    EFFECT_STANDARD_HEAL                    = 23,
    EFFECT_STANDARD_DAMAGE                  = 24,
    EFFECT_PET_TRAP                         = 25,
    EFFECT_APPLY_AURA_26                    = 26,
    EFFECT_INCREASING_DAMAGE                = 27, // on use, StateToTriggerMaxPoints = 21 (Mechanic_IsPoisoned ??) on hit
    EFFECT_APPLY_AURA_28                    = 28, // avoid dmg auras (flying, underwater...), speed auras (speedpriority, speed boost...)
    EFFECT_DEAL_DAMAGE_IF_STATE             = 29,
    EFFECT_SET_STATE                        = 31, // Special_IsRecovering, Special_IsCleansing, Clone_Active, Last_HitDealt, Condition_WasDamagedThisTurn...
    EFFECT_HEAL_PCT_OF_DAMAGE_DEALT         = 32,
    EFFECT_UNUSED_33                        = 33,
    EFFECT_UNUSED_43                        = 43,
    EFFECT_HEAL_PCT_OF_DAMAGE_TAKEN         = 44,
    EFFECT_UNUSED_45                        = 45,
    EFFECT_REMOVE_AURA                      = 49,
    EFFECT_APPLY_AURA_50                    = 50,
    EFFECT_APPLY_AURA_52                    = 52,
    EFFECT_HEAL_PCT_OF_MAX_HEALTH           = 53,
    EFFECT_APPLY_AURA_54                    = 54,
    EFFECT_UNUSED_55                        = 55,
    EFFECT_UNUSED_56                        = 56,
    EFFECT_UNUSED_57                        = 57,
    EFFECT_UNUSED_58                        = 58,
    EFFECT_DEAL_DAMAGE_IF_LESS_HP           = 59,
    EFFECT_HEAL_PCT_CONSUME_CORPSE          = 61, // Consume Corpse
    EFFECT_DEAL_DAMAGE_PCT_OF_HP            = 62,
    EFFECT_APPLY_AURA_63                    = 63, // Tranquility, Photosynthesis, Wish, Renewing Mists
    EFFECT_DEAL_DAMAGE_WITH_BONUS           = 65, // Creeping Fungus
    EFFECT_DEAL_DOUBLE_DAMAGE_BELOW_25_PCT  = 66,
    EFFECT_EQUALIZE_HEALTH                  = 67,
};

// custom
enum PetBattleEffectTarget
{
    TARGET_NONE,
    TARGET_CASTER,
    TARGET_CASTERS_TEAM_PET_1,
    TARGET_CASTERS_TEAM_PET_2,
    TARGET_CASTERS_TEAM_PAD,
    TARGET_ENEMY,
    TARGET_ENEMYS_TEAM_PET_1,
    TARGET_ENEMYS_TEAM_PET_2,
    TARGET_ENEMYS_TEAM_PAD,
    TARGET_WEATHER
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

class PetBattleAbility
{

private:
    struct EffectInfo
    {
        PetBattleEffectType type;
        PetBattleEffectTarget implicitTarget;
    };
    static std::unordered_map<PetBattleAbilityEffectName, EffectInfo> _effectsInfo;
};

#endif // PetBattleAbility_h__
