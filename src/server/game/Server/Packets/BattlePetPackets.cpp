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

#include "BattlePetPackets.h"
#include "World.h"

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetSlot const& slot)
{
    data << (slot.Pet.Guid.IsEmpty() ? ObjectGuid::Create<HighGuid::BattlePet>(0) : slot.Pet.Guid);
    data << uint32(slot.CollarID);
    data << uint8(slot.Index);
    data.WriteBit(slot.Locked);
    data.FlushBits();

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetJournalInfo const& pet)
{
    data << pet.Guid;
    data << uint32(pet.Species);
    data << uint32(pet.CreatureID);
    data << uint32(pet.CollarID);
    data << uint16(pet.Breed);
    data << uint16(pet.Level);
    data << uint16(pet.Exp);
    data << uint16(pet.Flags);
    data << uint32(pet.Power);
    data << uint32(pet.Health);
    data << uint32(pet.MaxHealth);
    data << uint32(pet.Speed);
    data << uint8(pet.Quality);
    data.WriteBits(pet.Name.size(), 7);
    data.WriteBit(!pet.Owner.IsEmpty()); // HasOwnerInfo
    data.WriteBit(pet.Name.empty()); // NoRename
    data.FlushBits();

    if (!pet.Owner.IsEmpty())
    {
        data << pet.Owner;
        data << uint32(GetVirtualRealmAddress()); // Virtual
        data << uint32(GetVirtualRealmAddress()); // Native
    }

    data.WriteString(pet.Name);

    return data;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetJournal::Write()
{
    _worldPacket << uint16(Trap);
    _worldPacket << uint32(Slots.size());
    _worldPacket << uint32(Pets.size());

    for (auto const& slot : Slots)
        _worldPacket << slot;

    for (auto const& pet : Pets)
        _worldPacket << pet;

    _worldPacket.WriteBit(HasJournalLock);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetUpdates::Write()
{
    _worldPacket << uint32(Pets.size());

    for (auto const& pet : Pets)
        _worldPacket << pet;

    _worldPacket.WriteBit(PetAdded);
    _worldPacket.FlushBits();

    return &_worldPacket;
}


WorldPacket const* WorldPackets::BattlePet::PetBattleSlotUpdates::Write()
{
    _worldPacket << uint32(Slots.size());

    for (auto const& slot : Slots)
        _worldPacket << slot;

    _worldPacket.WriteBit(NewSlot);
    _worldPacket.WriteBit(AutoSlotted);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::BattlePet::BattlePetSetBattleSlot::Read()
{
    _worldPacket >> PetGuid;
    _worldPacket >> Slot;
}

void WorldPackets::BattlePet::BattlePetModifyName::Read()
{
    _worldPacket >> PetGuid;
    uint32 nameLength = _worldPacket.ReadBits(7);
    bool hasDeclinedNames = _worldPacket.ReadBit();
    Name = _worldPacket.ReadString(nameLength);

    if (hasDeclinedNames)
    {
        uint8 declinedNameLengths[MAX_DECLINED_NAME_CASES];

        for (uint8 i = 0; i < 5; ++i)
            declinedNameLengths[i] = _worldPacket.ReadBits(7);

        for (uint8 i = 0; i < 5; ++i)
            Declined.name[i] = _worldPacket.ReadString(declinedNameLengths[i]);
    }
}

void WorldPackets::BattlePet::BattlePetDeletePet::Read()
{
    _worldPacket >> PetGuid;
}

void WorldPackets::BattlePet::BattlePetSetFlags::Read()
{
    _worldPacket >> PetGuid;
    _worldPacket >> Flags;
    ControlType = _worldPacket.ReadBits(2);
}

void WorldPackets::BattlePet::CageBattlePet::Read()
{
    _worldPacket >> PetGuid;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetDeleted::Write()
{
    _worldPacket << PetGuid;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetError::Write()
{
    _worldPacket.WriteBits(Result, 4);
    _worldPacket.FlushBits();
    _worldPacket << uint32(CreatureID);

    return &_worldPacket;
}

void WorldPackets::BattlePet::BattlePetSummon::Read()
{
    _worldPacket >> PetGuid;
}

// Pet Battles
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetAbility const& ability)
{
    data << uint32(ability.Id);
    data << uint16(ability.CooldownRemaining);
    data << uint16(ability.LockdownRemaining);
    data << uint8(ability.Slot);
    data << uint8(ability.PBOID);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetAura const& aura)
{
    data << uint32(aura.Id);
    data << uint32(aura.InstanceId);
    data << uint32(aura.RoundsRemaining);
    data << uint32(aura.CurrentRound);
    data << uint8(aura.CasterPBOID);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattlePetUpdateInfo const& pet)
{
    // TODO: use ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePet const& pet) if possible
    data << pet.JournalInfo->Guid;
    data << uint32(pet.JournalInfo->Species);
    data << uint32(pet.JournalInfo->CreatureID);
    data << uint32(pet.JournalInfo->CollarID);
    data << uint16(pet.JournalInfo->Level);
    data << uint16(pet.JournalInfo->Exp);
    data << uint32(pet.JournalInfo->Health);
    data << uint32(pet.JournalInfo->MaxHealth);
    data << uint32(pet.JournalInfo->Power);
    data << uint32(pet.JournalInfo->Speed);
    data << uint32(pet.NpcTeamMemberId);
    data << uint16(pet.JournalInfo->Quality);
    data << uint16(pet.StatusFlags);
    data << uint8(pet.Slot);
    data << uint32(pet.Abilities.size());
    data << uint32(pet.Auras.size());
    data << uint32(pet.States.size());

    for (auto ability : pet.Abilities)
        data << ability;

    for (auto aura : pet.Auras)
        data << aura;

    for (auto state : pet.States)
    {
        data << uint32(state.first);
        data << int32(state.second);
    }

    data.WriteBits(pet.JournalInfo->Name.size(), 7);
    data.FlushBits();
    data.WriteString(pet.JournalInfo->Name);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PlayerUpdate const& player)
{
    data << player.Guid;
    data << uint32(player.TrapAbilityID);
    data << uint32(player.TrapStatus);
    data << uint16(player.RoundTimeSecs);
    data << int8(player.FrontPet);
    data << uint8(player.InputFlags);
    data.WriteBits(player.Pets.size(), 2);
    data.FlushBits();

    for (auto pet : player.Pets)
        data << pet;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::EnviromentUpdate const& enviroment)
{
    data << uint32(enviroment.Auras.size());
    data << uint32(enviroment.States.size());

    for (auto aura : enviroment.Auras)
        data << aura;

    for (auto state : enviroment.States)
    {
        data << uint32(state.first);
        data << int32(state.second);
    }

    return data;
}

ByteBuffer& operator>>(ByteBuffer& buf, WorldPackets::BattlePet::LocationInfo& locationInfo)
{
    buf >> locationInfo.LocationResult;
    buf >> locationInfo.BattleOrigin.PositionXYZOStream();

    for (uint8 i = 0; i < 2; ++i)
        buf >> locationInfo.PlayerPositions[i].PositionXYZStream();

    return buf;
}

void WorldPackets::BattlePet::PetBattleRequestPvp::Read()
{
    _worldPacket >> TargetGuid;
    _worldPacket >> LocationInfo;
}

WorldPacket const* WorldPackets::BattlePet::PetBattlePvpChallenge::Write()
{
    _worldPacket << ChallengerGuid;
    // TODO: LocationInfo operator<<
    _worldPacket << uint32(LocationInfo.LocationResult);
    _worldPacket << LocationInfo.BattleOrigin.PositionXYZOStream();

    for (uint8 i = 0; i < 2; ++i)
        _worldPacket << LocationInfo.PlayerPositions[i].PositionXYZStream();

    return &_worldPacket;
}

void WorldPackets::BattlePet::PetBattleRequestWild::Read()
{
    _worldPacket >> TargetGuid;
    _worldPacket >> LocationInfo;
}

WorldPacket const* WorldPackets::BattlePet::PetBattleFinalizeLocation::Write()
{
    _worldPacket << uint32(LocationInfo.LocationResult);
    _worldPacket << LocationInfo.BattleOrigin.PositionXYZOStream();

    for (uint8 i = 0; i < 2; ++i)
        _worldPacket << LocationInfo.PlayerPositions[i].PositionXYZStream();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::PetBattleInitialUpdate::Write()
{
    for (uint8 i = 0; i < 2; ++i)
        _worldPacket << PlayerUpdate[i];

    for (uint8 i = 0; i < 3; ++i)
        _worldPacket << EnviromentUpdate[i];

    _worldPacket << uint16(WaitingForFrontPetsMaxSecs);
    _worldPacket << uint16(PvpMaxRoundTime);
    _worldPacket << uint32(CurrentRound);
    _worldPacket << uint32(NpcCreatureId);
    _worldPacket << uint32(NpcDisplayId);
    _worldPacket << uint8(CurrentPetBattleState);
    _worldPacket << uint8(ForfeitPenalty);
    _worldPacket << InitialWildPetGuid;
    _worldPacket.WriteBit(IsPvp);
    _worldPacket.WriteBit(CanAwardXP);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::BattlePet::PetBattleInput::Read()
{
    _worldPacket >> MoveType;
    _worldPacket >> NewFrontPet;
    _worldPacket >> DebugFlags;
    _worldPacket >> BattleInterrupted;
    _worldPacket >> AbilityID;
    _worldPacket >> Round;
    IgnoreAbandonPenalty = _worldPacket.ReadBit();
}

void WorldPackets::BattlePet::PetBattleReplaceFrontPet::Read()
{
    _worldPacket >> FrontPet;
}

void WorldPackets::BattlePet::PetBattleRequestUpdate::Read()
{
    _worldPacket >> TargetGUID;
    Canceled = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::BattlePet::PetBattleRequestFailed::Write()
{
    _worldPacket << uint8(Reason);

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattleActiveAbility const& ability)
{
    data << int32(ability.AbilityID);
    data << int16(ability.CooldownRemaining);
    data << int16(ability.LockdownRemaining);
    data << uint8(ability.AbilityIndex);
    data << uint8(ability.Pboid);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattleRoundResult const& round)
{
    data << int32(round.CurRound);
    data << int8(round.NextPetBattleState);

    data << int32(round.Effects.size());
    for (uint8 i = 0; i < 2; ++i)
    {
        data << uint8(round.NextInputFlags[i]);
        data << int8(round.NextTrapStatus[i]);
        data << uint16(round.RoundTimeSecs[i]);
    }

    data << int32(round.Cooldowns.size());

    for (auto const& effect : round.Effects)
    {
        data << uint32(effect.AbilityEffectID);
        data << uint16(effect.Flags);
        data << uint16(effect.SourceAuraInstanceID);
        data << uint16(effect.TurnInstanceID);
        data << int8(effect.PetBattleEffectType);
        data << uint8(effect.CasterPBOID);
        data << uint8(effect.StackDepth);

        data << uint32(effect.Targets.size());

        for (auto const& target : effect.Targets)
        {
            data.WriteBits(target.Type, 3);
            data << uint8(target.Petx);

            switch (target.Type)
            {
                case 1:
                    data << int32(target.StateID);
                    data << int32(target.StateValue);
                    break;
                case 2:
                    data << int32(target.BroadcastTextID);
                    break;
                case 3:
                    data << int32(target.ChangedAbilityID);
                    data << int32(target.CooldownRemaining);
                    data << int32(target.LockdownRemaining);
                    break;
                case 4:
                    data << int32(target.NewStatValue);
                    break;
                case 5:
                    data << int32(target.AuraInstanceID);
                    data << int32(target.AuraAbilityID);
                    data << int32(target.RoundsRemaining);
                    data << int32(target.CurrentRound);
                    break;
                case 6:
                    data << int32(target.Health);
                    break;
                case 7:
                    data << int32(target.TriggerAbilityID);
                    break;
            }
        }
    }

    for (auto const& ability : round.Cooldowns)
        data << ability;

    data.WriteBits(round.PetXDied.size(), 3);
    for (int8 pet : round.PetXDied)
        data << int8(pet);

    return data;
}

WorldPacket const* WorldPackets::BattlePet::PetBattleFirstRound::Write()
{
    _worldPacket << MsgData;
    
    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::PetBattleReplacementsMade::Write()
{
    _worldPacket << MsgData;

    return &_worldPacket;
}
