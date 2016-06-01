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

#ifndef BattlePetPackets_h__
#define BattlePetPackets_h__

#include "Packet.h"
#include "ObjectGuid.h"
#include "Unit.h"

namespace WorldPackets
{
    namespace BattlePet
    {
        struct BattlePetJournalInfo
        {
            ObjectGuid Guid;
            uint32 Species = 0;
            uint32 CreatureID = 0;
            uint32 CollarID = 0; // what's this?
            uint16 Breed = 0;
            uint16 Level = 0;
            uint16 Exp = 0;
            uint16 Flags = 0;
            uint32 Power = 0;
            uint32 Health = 0;
            uint32 MaxHealth = 0;
            uint32 Speed = 0;
            uint8 Quality = 0;
            ObjectGuid Owner; // for non-account wide pets only? (Guild Page, Guild Herald)
            std::string Name;
        };

        struct BattlePetSlot
        {
            BattlePetJournalInfo* Pet = nullptr;
            uint32 CollarID = 0; // what's this?
            uint8 Index = 0;
            bool Locked = true;
        };

        class BattlePetJournal final : public ServerPacket
        {
        public:
            BattlePetJournal() : ServerPacket(SMSG_BATTLE_PET_JOURNAL) { }

            WorldPacket const* Write() override;

            uint16 Trap = 0;
            std::vector<BattlePetSlot> Slots;
            std::vector<BattlePetJournalInfo> Pets;
            bool HasJournalLock = true;
        };

        class BattlePetJournalLockAcquired final : public ServerPacket
        {
        public:
            BattlePetJournalLockAcquired() : ServerPacket(SMSG_BATTLE_PET_JOURNAL_LOCK_ACQUIRED, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class BattlePetJournalLockDenied final : public ServerPacket
        {
        public:
            BattlePetJournalLockDenied() : ServerPacket(SMSG_BATTLE_PET_JOURNAL_LOCK_DENIED, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class BattlePetRequestJournal final : public ClientPacket
        {
        public:
            BattlePetRequestJournal(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PET_REQUEST_JOURNAL, std::move(packet)) { }

            void Read() override { }
        };

        class BattlePetUpdates final : public ServerPacket
        {
        public:
            BattlePetUpdates() : ServerPacket(SMSG_BATTLE_PET_UPDATES) { }

            WorldPacket const* Write() override;

            std::vector<BattlePetJournalInfo> Pets;
            bool PetAdded = false;
        };

        class PetBattleSlotUpdates final : public ServerPacket
        {
        public:
            PetBattleSlotUpdates() : ServerPacket(SMSG_PET_BATTLE_SLOT_UPDATES) { }

            WorldPacket const* Write() override;

            std::vector<BattlePetSlot> Slots;
            bool AutoSlotted = false;
            bool NewSlot = false;
        };

        class BattlePetSetBattleSlot final : public ClientPacket
        {
        public:
            BattlePetSetBattleSlot(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PET_SET_BATTLE_SLOT, std::move(packet)) { }

            void Read() override;

            ObjectGuid PetGuid;
            uint8 Slot = 0;
        };

        class BattlePetModifyName final : public ClientPacket
        {
        public:
            BattlePetModifyName(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PET_MODIFY_NAME, std::move(packet)) { }

            void Read() override;

            ObjectGuid PetGuid;
            std::string Name;
            DeclinedName Declined;
        };

        class BattlePetDeletePet final : public ClientPacket
        {
        public:
            BattlePetDeletePet(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PET_DELETE_PET, std::move(packet)) { }

            void Read() override;

            ObjectGuid PetGuid;
        };

        class BattlePetSetFlags final : public ClientPacket
        {
        public:
            BattlePetSetFlags(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PET_SET_FLAGS, std::move(packet)) { }

            void Read() override;

            ObjectGuid PetGuid;
            uint32 Flags = 0;
            uint8 ControlType = 0;
        };

        class CageBattlePet final : public ClientPacket
        {
        public:
            CageBattlePet(WorldPacket&& packet) : ClientPacket(CMSG_CAGE_BATTLE_PET, std::move(packet)) { }

            void Read() override;

            ObjectGuid PetGuid;
        };

        class BattlePetDeleted final : public ServerPacket
        {
        public:
            BattlePetDeleted() : ServerPacket(SMSG_BATTLE_PET_DELETED, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid PetGuid;
        };

        class BattlePetError final : public ServerPacket
        {
        public:
            BattlePetError() : ServerPacket(SMSG_BATTLE_PET_ERROR, 5) { }

            WorldPacket const* Write() override;

            uint8 Result = 0;
            uint32 CreatureID = 0;
        };

        class BattlePetSummon final : public ClientPacket
        {
        public:
            BattlePetSummon(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PET_SUMMON, std::move(packet)) { }

            void Read() override;

            ObjectGuid PetGuid;
        };

        // Pet Battles

        struct LocationInfo
        {
            uint32 LocationResult = 0;
            Position BattleOrigin;
            Position PlayerPositions[2];
        };

        class PetBattleRequestPvp final : public ClientPacket
        {
        public:
            PetBattleRequestPvp(WorldPacket&& packet) : ClientPacket(CMSG_PET_BATTLE_REQUEST_PVP, std::move(packet)) { }

            void Read() override;

            ObjectGuid TargetGuid;
            LocationInfo LocationInfo;
        };

        class PetBattlePvpChallenge final : public ServerPacket
        {
        public:
            PetBattlePvpChallenge() : ServerPacket(SMSG_PET_BATTLE_PVP_CHALLENGE) { }

            WorldPacket const* Write() override;

            ObjectGuid ChallengerGuid;
            LocationInfo LocationInfo;
        };

        class PetBattleRequestWild final : public ClientPacket
        {
        public:
            PetBattleRequestWild(WorldPacket&& packet) : ClientPacket(CMSG_PET_BATTLE_REQUEST_WILD, std::move(packet)) { }

            void Read() override;

            ObjectGuid TargetGuid;
            LocationInfo LocationInfo;
        };

        class PetBattleRequestUpdate final : public ClientPacket
        {
        public:
            PetBattleRequestUpdate(WorldPacket&& packet) : ClientPacket(CMSG_PET_BATTLE_REQUEST_UPDATE, std::move(packet)) { }

            void Read() override;

            bool Canceled = false;
            ObjectGuid TargetGUID;
        };

        class PetBattleRequestFailed final : public ServerPacket
        {
        public:
            PetBattleRequestFailed() : ServerPacket(SMSG_PET_BATTLE_REQUEST_FAILED, 1) { }

            WorldPacket const* Write() override;

            uint8 Reason = 0;
        };

        class PetBattleFinalizeLocation final : public ServerPacket
        {
        public:
            PetBattleFinalizeLocation() : ServerPacket(SMSG_PET_BATTLE_FINALIZE_LOCATION) { }

            WorldPacket const* Write() override;

            LocationInfo LocationInfo;
        };

        struct BattlePetAbility
        {
            uint32 Id = 0;
            uint16 CooldownRemaining = 0;
            uint16 LockdownRemaining = 0; // what's this?
            uint8 Slot = 0;
            uint8 PBOID = 0;
        };

        struct BattlePetAura
        {
            uint32 Id = 0;
            uint32 InstanceId = 0;
            int32 RoundsRemaining = 0; // -1 = infinite
            uint32 CurrentRound = 0;
            uint8 CasterPBOID = 0;
        };

        struct PetBattlePetUpdateInfo
        {
            // TODO: consider replacing JournalInfo pointer with variables
            BattlePetJournalInfo* JournalInfo = nullptr;
            uint32 NpcTeamMemberId = 0;
            uint16 StatusFlags = 0;
            uint8 Slot = 0;
            std::vector<BattlePetAbility> Abilities;
            std::vector<BattlePetAura> Auras;
            std::unordered_map<uint32 /*BattlePetState*/, int32 /*Value*/> States;
        };

        struct PlayerUpdate
        {
            ObjectGuid Guid;
            uint32 TrapAbilityID = 0;
            uint32 TrapStatus = 0;
            uint16 RoundTimeSecs = 0;
            int8 FrontPet = 0;
            uint8 InputFlags = 0;
            std::vector<PetBattlePetUpdateInfo> Pets;
        };

        struct EnviromentUpdate
        {
            std::vector<BattlePetAura> Auras;
            std::unordered_map<uint32 /*BattlePetState*/, int32 /*Value*/> States;
        };

        class PetBattleInitialUpdate final : public ServerPacket
        {
        public:
            PetBattleInitialUpdate() : ServerPacket(SMSG_PET_BATTLE_INITIAL_UPDATE) { }

            WorldPacket const* Write() override;

            PlayerUpdate PlayerUpdate[2];
            EnviromentUpdate EnviromentUpdate[3];
            uint16 WaitingForFrontPetsMaxSecs = 0;
            uint16 PvpMaxRoundTime = 0;
            uint32 CurrentRound = 0;
            uint32 NpcCreatureId = 0;
            uint32 NpcDisplayId = 0;
            uint8 CurrentPetBattleState = 0;
            uint8 ForfeitPenalty = 0;
            ObjectGuid InitialWildPetGuid;
            bool IsPvp = false;
            bool CanAwardXP = false;
        };

        class PetBattleFinalNotify final : public ClientPacket
        {
        public:
            PetBattleFinalNotify(WorldPacket&& packet) : ClientPacket(CMSG_PET_BATTLE_FINAL_NOTIFY, std::move(packet)) { }

            void Read() override { }
        };

        class PetBattleInput final : public ClientPacket
        {
        public:
            PetBattleInput(WorldPacket&& packet) : ClientPacket(CMSG_PET_BATTLE_INPUT, std::move(packet)) { }

            void Read() override;

            uint8 MoveType = 0;
            int8 NewFrontPet = -1;
            uint8 DebugFlags = 0;
            uint8 BattleInterrupted = 0;
            uint32 AbilityID = 0;
            int32 Round = -1;
            bool IgnoreAbandonPenalty = false;
        };

        class PetBattleQuitNotify final : public ClientPacket
        {
        public:
            PetBattleQuitNotify(WorldPacket&& packet) : ClientPacket(CMSG_PET_BATTLE_QUIT_NOTIFY, std::move(packet)) { }

            void Read() override { }
        };

        class PetBattleReplaceFrontPet final : public ClientPacket
        {
        public:
            PetBattleReplaceFrontPet(WorldPacket&& packet) : ClientPacket(CMSG_PET_BATTLE_REPLACE_FRONT_PET, std::move(packet)) { }

            void Read() override;

            uint8 FrontPet = 0;
        };

        struct PetBattleEffectTarget
        {
            uint8 Type = 0;
            uint8 Petx = 0;

            union TargetParams
            {
                TargetParams() { }

                struct
                {
                    uint32 AuraInstanceID = 0;
                    uint32 AuraAbilityID = 0;
                    int32 RoundsRemaining = 0;
                    int32 CurrentRound = 0;
                } Aura;

                struct
                {
                    uint32 StateID = 0;
                    int32 StateValue = 0;
                } State;

                int32 Health;

                int32 NewStatValue;

                int32 TriggerAbilityID;

                struct
                {
                    int32 ChangedAbilityID = 0;
                    int32 CooldownRemaining = 0;
                    int32 LockdownRemaining = 0;
                } AbilityChange;

                int32 BroadcastTextID;
            } Params;
        };

        struct PetBattleEffect
        {
            uint32 AbilityEffectID = 0;
            uint16 Flags = 0;
            uint16 SourceAuraInstanceID = 0;
            uint16 TurnInstanceID = 0;
            int8 PetBattleEffectType = 0;
            uint8 CasterPBOID = 0;
            uint8 StackDepth = 0;
            std::vector<PetBattleEffectTarget> Targets;
        };

        struct PetBattleActiveAbility
        {
            int32 AbilityID = 0;
            int16 CooldownRemaining = 0;
            int16 LockdownRemaining = 0;
            int8 AbilityIndex = 0;
            uint8 Pboid = 0;
        };

        struct RoundResult
        {
            int32 CurRound = 0;
            int8 NextPetBattleState = 0;
            std::vector<PetBattleEffect> Effects;
            std::vector<int8> PetXDied;
            std::vector<PetBattleActiveAbility> Cooldowns;
            uint8 NextInputFlags[2] = { };
            int8 NextTrapStatus[2] = { };
            uint16 RoundTimeSecs[2] = { };
        };

        class PetBattleFirstRound final : public ServerPacket
        {
        public:
            PetBattleFirstRound() : ServerPacket(SMSG_PET_BATTLE_FIRST_ROUND) { }

            WorldPacket const* Write() override;

            RoundResult RoundResult;
        };

        class PetBattleRoundResult final : public ServerPacket
        {
        public:
            PetBattleRoundResult() : ServerPacket(SMSG_PET_BATTLE_ROUND_RESULT) { }

            WorldPacket const* Write() override;

            RoundResult RoundResult;
        };

        struct FinalRoundPetInfo
        {
            ObjectGuid Guid;
            uint16 Level = 0;
            uint16 Xp = 0;
            int32 Health = 0;
            uint32 MaxHealth = 0;
            uint16 InitialLevel = 0;
            uint8 PBOID = 0;
            bool Captured = false;
            bool Caged = false;
            bool SeenAction = false;
            bool AwardedXP = false;
        };

        class PetBattleFinalRound final : public ServerPacket
        {
        public:
            PetBattleFinalRound() : ServerPacket(SMSG_PET_BATTLE_FINAL_ROUND) { }

            WorldPacket const* Write() override;

            bool Abandoned = false;
            bool PvpBattle = false;
            bool Winners[2] = { };
            uint32 NpcCreatureID[2] = { };
            std::vector<FinalRoundPetInfo> Pets;
        };

        class PetBattleReplacementsMade final : public ServerPacket
        {
        public:
            PetBattleReplacementsMade() : ServerPacket(SMSG_PET_BATTLE_REPLACEMENTS_MADE) { }

            WorldPacket const* Write() override;

            RoundResult RoundResult;
        };

        class PetBattleFinished final : public ServerPacket
        {
        public:
            PetBattleFinished() : ServerPacket(SMSG_PET_BATTLE_FINISHED, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };
    }
}

#endif // BattlePetPackets_h__
