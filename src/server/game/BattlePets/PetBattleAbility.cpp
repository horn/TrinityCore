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

#include "PetBattleAbility.h"
#include "Log.h"

std::unordered_map<PetBattleAbilityEffectName, PetBattleAbility::EffectTypeInfo> PetBattleAbility::_effectTypesInfo
{
    //{EFFECT_DO_NOTHING,      {TARGET_NONE}},
    {EFFECT_STANDARD_HEAL,   {PETBATTLE_EFFECT_TYPE_SET_HEALTH,    TARGET_CASTER, &PetBattleAbility::EffectNULL}},
    {EFFECT_STANDARD_DAMAGE, {PETBATTLE_EFFECT_TYPE_SET_HEALTH,    TARGET_CASTER, &PetBattleAbility::EffectDealDamage}},
    {EFFECT_PET_TRAP,        {PETBATTLE_EFFECT_TYPE_STATUS_CHANGE, TARGET_ENEMY,  &PetBattleAbility::EffectNULL}},
    //{EFFECT_APPLY_AURA_26,   TARGET_ENEMY}, 
    //{EFFECT_INCREASING_DAMAGE, {PETBATTLE_EFFECT_TYPE_SET_HEALTH, TARGET_CASTER}},
    /*{EFFECT_APPLY_AURA_28
    {EFFECT_DEAL_DAMAGE_IF_STATE
    {EFFECT_SET_STATE
    {EFFECT_HEAL_PCT_OF_DAMAGE_DEALT
    {EFFECT_UNUSED_33
    {EFFECT_UNUSED_43
    {EFFECT_HEAL_PCT_OF_DAMAGE_TAKEN
    {EFFECT_UNUSED_45
    {EFFECT_REMOVE_AURA
    {EFFECT_APPLY_AURA_50
    {EFFECT_APPLY_AURA_52
    {EFFECT_HEAL_PCT_OF_MAX_HEALTH
    {EFFECT_APPLY_AURA_54
    {EFFECT_UNUSED_55
    {EFFECT_UNUSED_56
    {EFFECT_UNUSED_57
    {EFFECT_UNUSED_58
    {EFFECT_DEAL_DAMAGE_IF_LESS_HP, TARGET_ENEMY},
    {EFFECT_HEAL_PCT_CONSUME_CORPSE
    {EFFECT_DEAL_DAMAGE_PCT_OF_HP
    {EFFECT_APPLY_AURA_63
    {EFFECT_DEAL_DAMAGE_WITH_BONUS
    {EFFECT_DEAL_DOUBLE_DAMAGE_BELOW_25_PCT, TARGET_ENEMY}
    {EFFECT_EQUALIZE_HEALTH*/
};

std::unordered_map<uint32 /*abilityId*/, std::set<BattlePetAbilityTurnEntry const*>> PetBattleAbility::_abilityTurnsByAbility;
std::unordered_map<uint32 /*abilityTurnId*/, std::set<BattlePetAbilityEffectEntry const*>> PetBattleAbility::_abilityEffectsByTurn;

void PetBattleAbility::LoadAbilities()
{
    for (BattlePetAbilityEntry const* ability : sBattlePetAbilityStore)
    {
        std::vector<BattlePetAbilityTurnEntry const*> turns = sDB2Manager.GetAbilityTurnByAbilityId(ability->ID);
        if (!turns.empty())
            _abilityTurnsByAbility[ability->ID] = std::set<BattlePetAbilityTurnEntry const*>(turns.begin(), turns.end());
    }

    for (BattlePetAbilityTurnEntry const* turn : sBattlePetAbilityTurnStore)
    {
        std::vector<BattlePetAbilityEffectEntry const*> abilityEffects = sDB2Manager.GetAbilityEffectByTurnId(turn->ID);
        if (!abilityEffects.empty())
            _abilityEffectsByTurn[turn->ID] = std::set<BattlePetAbilityEffectEntry const*>(abilityEffects.begin(), abilityEffects.end());
    }
}

PetBattleEffectTarget PetBattleAbility::GetEffectTargetName(PetBattleAbilityEffectName const& effect) const
{
    // TODO: if explicit target exists (from DB), use it, otherwise use implicit target
    for (auto const& item : _effectTypesInfo)
        if (item.first == effect)
            return item.second.implicitTarget;

    return TARGET_NONE;
}

PetBattle::PetBattleObject* PetBattleAbility::GetEffectTarget(PetBattleAbilityEffectName const& effectId) const
{
    uint8 index = 0;
    bool participant = _casterId > PBOID_P0_PET_2 ? true : false;

    switch (GetEffectTargetName(effectId))
    {
        case TARGET_CASTER:
            index = participant ? PBOID_P1_PET_0 : PBOID_P0_PET_0;
            break;
        case TARGET_CASTERS_TEAM_PET_1:
            index = participant ? PBOID_P1_PET_1 : PBOID_P0_PET_1;
            break;
        case TARGET_CASTERS_TEAM_PET_2:
            index = participant ? PBOID_P1_PET_2 : PBOID_P0_PET_2;
            break;
        case TARGET_CASTERS_TEAM_PAD:
            index = participant ? PBOID_PAD_1 : PBOID_PAD_0;
            break;
        case TARGET_ENEMY:
            index = participant ? PBOID_P0_PET_0 : PBOID_P1_PET_0;
            break;
        case TARGET_ENEMYS_TEAM_PET_1:
            index = _casterId ? PBOID_P0_PET_1 : PBOID_P1_PET_1;
            break;
        case TARGET_ENEMYS_TEAM_PET_2:
            index = participant ? PBOID_P0_PET_2 : PBOID_P1_PET_2;
            break;
        case TARGET_ENEMYS_TEAM_PAD:
            index = participant ? PBOID_PAD_0 : PBOID_PAD_1;
            break;
        case TARGET_WEATHER:
            index = PBOID_WEATHER;
            break;
        case TARGET_NONE: // TODO: check what to do with no target
        default:
            return nullptr;
    }

    return _parentBattle->GetPetBattleObject(PBOIDNames(index));
}

void PetBattleAbility::ProcessEffects()
{
    for (BattlePetAbilityTurnEntry const* turnEntry : _abilityTurnsByAbility[GetId()])
    {
        if (turnEntry->Turn == _round)
        {
            for (BattlePetAbilityEffectEntry const* effectEntry : _abilityEffectsByTurn[turnEntry->ID])
            {
                (this->*_effectTypesInfo[PetBattleAbilityEffectName(effectEntry->EffectPropertiesID)].handler)(effectEntry);

                // TODO: generate PetBattleEffect and PetBattleEffectTarget properly and append them to PetBattle::_roundResult
                /*WorldPackets::BattlePet::PetBattleEffect eff;
                eff.PetBattleEffectType = _effectTypesInfo[PetBattleAbilityEffectName(effectEntry->EffectPropertiesID)].type;
                WorldPackets::BattlePet::PetBattleEffectTarget effTarget;
                effTarget.stuff = values;*/
            }

            break;
        }
    }
}

void PetBattleAbility::ProcessProc(PetBattleAbilityProcType procType)
{
    for (BattlePetAbilityTurnEntry const* turnEntry : _abilityTurnsByAbility[GetId()])
        if (turnEntry->HasProcType && turnEntry->ProcType == procType)
            for (BattlePetAbilityEffectEntry const* effectEntry : _abilityEffectsByTurn[turnEntry->ID])
            {
                // same as ProcessEffects()
            }
}

void PetBattleAbility::EffectNULL(BattlePetAbilityEffectEntry const* effect)
{
    // TODO: print effect name instead of number
    TC_LOG_ERROR("server.fixthisstring", "Received battle pet ability %s (ID: %u) with unhandled effect %u", _ability->Name->Str[LOCALE_enUS], GetId(), effect->EffectPropertiesID);
}

void PetBattleAbility::EffectUnused(BattlePetAbilityEffectEntry const* /*effect*/)
{

}

void PetBattleAbility::EffectHeal(BattlePetAbilityEffectEntry const* effect)
{
    PetBattle::PetBattleObject* target = GetEffectTarget(PetBattleAbilityEffectName(effect->EffectPropertiesID));
    if (!target)
        return;

    _caster->DealHeal(target, effect->PropertyValue[0]); // TODO: Maybe convert properties to structs or associative map
}

void PetBattleAbility::EffectDealDamage(BattlePetAbilityEffectEntry const* effect)
{
    PetBattle::PetBattleObject* target = GetEffectTarget(PetBattleAbilityEffectName(effect->EffectPropertiesID));
    if (!target)
        return;

    _caster->DealDamage(target, effect->PropertyValue[0]);
}
