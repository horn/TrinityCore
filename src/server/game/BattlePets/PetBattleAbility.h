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

#include "DB2Stores.h"
#include "PetBattle.h"

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
    EFFECT_DEAL_DAMAGE_PCT_OF_MAX_HP        = 62,
    EFFECT_APPLY_AURA_63                    = 63, // Tranquility, Photosynthesis, Wish, Renewing Mists
    EFFECT_DEAL_DAMAGE_WITH_BONUS           = 65, // Creeping Fungus
    EFFECT_DEAL_DOUBLE_DAMAGE_BELOW_25_PCT  = 66,
    EFFECT_EQUALIZE_HEALTH                  = 67,
    EFFECT_DEAL_DAMAGE_PCT_OF_USERS_MAX_HP  = 68,
    EFFECT_APPLY_OR_CONSUME_AURA_TO_HEAL    = 72, // Stockpile
    EFFECT_UNUSED_73                        = 73,
    EFFECT_INITIALIZE_PET_BATTLE            = 74, // Pet Battle Initializer (Initializes pet auras and health.)
    EFFECT_APPLY_OR_CONSUME_AURA_TO_DMG_75  = 75, // Launch Rocket
    EFFECT_APPLY_OR_CONSUME_AURA_TO_DMG_76  = 76, // Pump, Barel Toss, Wind-Up
    EFFECT_APPLY_OR_CONSUME_AURA_TO_DMG_77  = 77, // Lock-On, GM Unkillable (?)
    //EFFECT_APPLY_ 78
    EFFECT_STATE_CHANGE                     = 79,
    EFFECT_WEATHER                          = 80,
    //EFFECT                                = 85, // something with states (destroy objects and type override)
    //EFFECT                                = 86, // Gravity (apply aura 2, if used on the same target, apply aura 1)
    EFFECT_DEAL_DAMAGE_HIT_CHANCE_CONDITION = 96,
    EFFECT_UNUSED_97                        = 97,
    EFFECT_UNUSED_99                        = 99,
    EFFECT_HEAL_WITH_PET_TYPE_CONDITION     = 100, // Rebuild, Healing Stream
    EFFECT_DEAL_DAMAGE_IF_FIRST             = 103,
    EFFECT_HEAL_WITH_STATE_CONDITION        = 104,

    EFFECT_INSTA_KILL_CASTER                = 135,
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

class PetBattleAbility
{
public:
    PetBattleAbility(uint32 abilityId, PetBattle::PetBattleObject* caster, PetBattle* parentBattle, uint8 casterId) :
        _caster(caster), _parentBattle(parentBattle), _abilityId(abilityId), _casterId(casterId) { }

    friend class PetBattle;

    uint32 GetId() const { return _abilityId; }

    PetBattle::PetBattleObject* GetCaster() { return _caster; }
    PetBattleEffectTarget GetEffectTargetName(PetBattleAbilityEffectName const& effect) const;
    PetBattle::PetBattleObject* GetEffectTarget(PetBattleAbilityEffectName const& effectId) const;

    static void LoadAbilities();

    void ProcessEffects();
    void ProcessProc(PetBattleAbilityProcType procType);

    void EffectNULL(PetBattle::PetBattleObject* effectTarget);
    void EffectUnused(PetBattle::PetBattleObject* effectTarget);
    void EffectHeal(PetBattle::PetBattleObject* effectTarget);
    void EffectDealDamage(PetBattle::PetBattleObject* effectTarget);

private:
    PetBattle::PetBattleObject* _caster = nullptr;
    PetBattle* _parentBattle;
    uint32 _abilityId = 0; // or BattlePetAbilityEntry const* instead
    uint8 _round = 0;
    uint8 _casterId;

    struct EffectTypeInfo
    {
        PetBattleEffectType type;
        PetBattleEffectTarget implicitTarget;
        // TODO: handler will need access to more stuff (effectID, properties...) -> waiting for containerception of DB2Stores
        void(PetBattleAbility::*handler)(PetBattle::PetBattleObject*);
    };
    static std::unordered_map<PetBattleAbilityEffectName, EffectTypeInfo> _effectTypesInfo;
    static std::unordered_map<uint32 /*abilityId*/, std::set<BattlePetAbilityTurnEntry const*>> _abilityTurnsByAbility; // TODO: getter here or move to singleton
    static std::unordered_map<uint32 /*abilityTurnId*/, std::set<BattlePetAbilityEffectEntry const*>> _abilityEffectsByTurn;
};

#endif // PetBattleAbility_h__
