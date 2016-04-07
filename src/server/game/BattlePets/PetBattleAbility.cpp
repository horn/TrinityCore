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

std::unordered_map<PetBattleAbilityEffectName, PetBattleAbility::EffectInfo> PetBattleAbility::_effectsInfo
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

void PetBattleAbility::EffectNULL(WorldPackets::BattlePet::PetBattlePetUpdateInfo* /*effectTarget*/)
{
    //TC_LOG_ERROR("server", "Received battle pet ability %s (ID: %u) with unhandled effect %s");
}

void PetBattleAbility::EffectUnused(WorldPackets::BattlePet::PetBattlePetUpdateInfo* effectTarget)
{

}

void PetBattleAbility::EffectDealDamage(WorldPackets::BattlePet::PetBattlePetUpdateInfo* effectTarget)
{
    //effectTarget->States[STATE_STAT_STAMINA] -= effect->points; // points modified by power and stuff
}
