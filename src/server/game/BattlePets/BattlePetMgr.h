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

#ifndef BattlePetMgr_h__
#define BattlePetMgr_h__

#include "BattlePetPackets.h"
#include "PetBattle.h"

enum BattlePetMisc
{
    MAX_PET_BATTLE_SLOTS            = 3,
    MAX_BATTLE_PETS_PER_SPECIES     = 3,
    BATTLE_PET_CAGE_ITEM_ID         = 82800,
    DEFAULT_SUMMON_BATTLE_PET_SPELL = 118301
};

enum BattlePetDBFlags
{
    BATTLE_PET_DB_FLAG_FAVORITE         = 0x01,
    BATTLE_PET_DB_FLAG_SPELL_1_ROW_2    = 0x10,
    BATTLE_PET_DB_FLAG_SPELL_2_ROW_2    = 0x20,
    BATTLE_PET_DB_FLAG_SPELL_3_ROW_2    = 0x40
};

// TODO: fix undefined values in this enum
enum BattlePetError
{
    BATTLEPETRESULT_CANT_HAVE_MORE_PETS_OF_THAT_TYPE = 9,
    BATTLEPETRESULT_TOO_HIGH_LEVEL_TO_UNCAGE         = 12,
    BATTLEPETRESULT_CANT_HAVE_MORE_PETS              = 13,

    // wrong order
    BATTLEPETRESULT_DUPLICATE_CONVERTED_PET,
    BATTLEPETRESULT_NEED_TO_UNLOCK,
    BATTLEPETRESULT_BAD_PARAM,
    BATTLEPETRESULT_LOCKED_PET_ALREADY_EXISTS,
    BATTLEPETRESULT_OK,
    BATTLEPETRESULT_UNCAPTURABLE,
    BATTLEPETRESULT_CANT_INVALID_CHARACTER_GUID
};

// taken from BattlePetState.db2 - it seems to store some initial values for battle pets
// there are only values used in BattlePetSpeciesState.db2 and BattlePetBreedState.db2 + some used in pet battles packets
// TODO: expand this enum if needed
enum BattlePetState
{
    STATE_MAX_HEALTH_BONUS          = 2,
    STATE_INTERNAL_INITIAL_LEVEL    = 17,
    STATE_STAT_POWER                = 18,
    STATE_STAT_STAMINA              = 19,
    STATE_STAT_SPEED                = 20,
    STATE_MOD_DAMAGE_DEALT_PERCENT  = 23,
    STATE_STAT_CRIT_CHANCE          = 40,
    STATE_PASSIVE_CRITTER           = 42,
    STATE_PASSIVE_BEAST             = 43,
    STATE_PASSIVE_HUMANOID          = 44,
    STATE_PASSIVE_FLYING            = 45,
    STATE_PASSIVE_DRAGON            = 46,
    STATE_PASSIVE_ELEMENTAL         = 47,
    STATE_PASSIVE_MECHANICAL        = 48,
    STATE_PASSIVE_MAGIC             = 49,
    STATE_PASSIVE_UNDEAD            = 50,
    STATE_PASSIVE_AQUATIC           = 51,
    STATE_GENDER                    = 78, // 1 - male, 2 - female
    STATE_COSMETIC_WATER_BUBBLED    = 85,
    STATE_SPECIAL_IS_COCKROACH      = 93,
    STATE_COSMETIC_FLY_TIER         = 128,
    STATE_COSMETIC_BIGGLESWORTH     = 144,
    STATE_PASSIVE_ELITE             = 153,
    STATE_PASSIVE_BOSS              = 162,
    STATE_COSMETIC_TREASURE_GOBLIN  = 176,
    // these are not in BattlePetState.db2 but are used in BattlePetSpeciesState.db2
    STATE_START_WITH_BUFF           = 183,
    STATE_START_WITH_BUFF_2         = 184,
    //
    STATE_COSMETIC_SPECTRAL_BLUE    = 196
};

enum BattlePetFamily
{
    BATTLE_PET_FAMILY_HUMANOID      = 0,
    BATTLE_PET_FAMILY_DRAGONKIN     = 1,
    BATTLE_PET_FAMILY_FLYING        = 2,
    BATTLE_PET_FAMILY_UNDEAD        = 3,
    BATTLE_PET_FAMILY_CRITTER       = 4,
    BATTLE_PET_FAMILY_MAGIC         = 5,
    BATTLE_PET_FAMILY_ELEMENTAL     = 6,
    BATTLE_PET_FAMILY_BEAST         = 7,
    BATTLE_PET_FAMILY_AQUATIC       = 8,
    BATTLE_PET_FAMILY_MECHANICAL    = 9,

    BATTLE_PET_FAMILY_MAX
};

enum BattlePetSaveInfo
{
    BATTLE_PET_UNCHANGED = 0,
    BATTLE_PET_CHANGED   = 1,
    BATTLE_PET_NEW       = 2,
    BATTLE_PET_REMOVED   = 3
};

uint32 const TrapSpells[4] = { 427, 77, 135, 1368 };
BattlePetState const FamilyStates[10] =
{
    STATE_PASSIVE_HUMANOID,
    STATE_PASSIVE_DRAGON,
    STATE_PASSIVE_FLYING,
    STATE_PASSIVE_UNDEAD,
    STATE_PASSIVE_CRITTER,
    STATE_PASSIVE_MAGIC,
    STATE_PASSIVE_ELEMENTAL,
    STATE_PASSIVE_BEAST,
    STATE_PASSIVE_AQUATIC,
    STATE_PASSIVE_MECHANICAL
};

class BattlePetMgr
{
public:
    struct BattlePet
    {
        int32 GetBaseStateValue(BattlePetState state);
        void CalculateStats();

        BattlePetFamily GetFamily();

        WorldPackets::BattlePet::BattlePetJournalInfo JournalInfo;
        WorldPackets::BattlePet::PetBattlePetUpdateInfo UpdateInfo; // contains JournalInfo too - maybe redunant
        BattlePetSaveInfo SaveInfo;
    };

    explicit BattlePetMgr(WorldSession* owner);

    static void Initialize();

    static uint16 RollPetBreed(uint32 species);
    static uint8 GetDefaultPetQuality(uint32 species);

    void LoadFromDB(PreparedQueryResult pets, PreparedQueryResult slots);
    void SaveToDB(SQLTransaction& trans);

    BattlePet* GetPet(ObjectGuid guid);
    void AddPet(uint32 species, uint32 creatureId, uint16 breed, uint8 quality, uint16 level = 1);
    void RemovePet(ObjectGuid guid);

    uint8 GetPetCount(uint32 species) const;

    WorldPackets::BattlePet::BattlePetSlot* GetSlot(uint8 slot) { return &_slots[slot]; }
    void UnlockSlot(uint8 slot);

    WorldSession* GetOwner() const { return _owner; }

    uint16 GetTrapLevel() const { return _trapLevel; }
    std::vector<BattlePet> GetLearnedPets() const;
    std::vector<WorldPackets::BattlePet::BattlePetSlot> GetSlots() const { return _slots; }

    void CageBattlePet(ObjectGuid guid);
    void HealBattlePetsPct(uint8 pct);

    void SummonPet(ObjectGuid guid);

    void SendUpdates(std::vector<BattlePet> pets, bool petAdded);
    void SendError(BattlePetError error, uint32 creatureId);

    // Pet Battles

    void InitializePetBattle(ObjectGuid target);

    WorldPackets::BattlePet::PlayerUpdate GetPlayerUpdateInfo();
    WorldPackets::BattlePet::PlayerUpdate GetWildPetUpdateInfo(Creature* creature) const;

    PetBattle* GetPetBattle() const { return _battle; }

private:
    WorldSession* _owner;
    PetBattle* _battle;
    uint16 _trapLevel = 0;
    std::unordered_map<uint64 /*battlePetGuid*/, BattlePet> _pets;
    std::vector<WorldPackets::BattlePet::BattlePetSlot> _slots;

    static void LoadAvailablePetBreeds();
    static void LoadDefaultPetQualities();

    // hash no longer required in C++14
    static std::unordered_map<uint16 /*BreedID*/, std::unordered_map<BattlePetState /*state*/, int32 /*value*/, std::hash<std::underlying_type<BattlePetState>::type> >> _battlePetBreedStates;
    static std::unordered_map<uint32 /*SpeciesID*/, std::unordered_map<BattlePetState /*state*/, int32 /*value*/, std::hash<std::underlying_type<BattlePetState>::type> >> _battlePetSpeciesStates;
    static std::unordered_map<uint32 /*SpeciesID*/, std::unordered_set<uint8 /*breed*/>> _availableBreedsPerSpecies;
    static std::unordered_map<uint32 /*SpeciesID*/, uint8 /*quality*/> _defaultQualityPerSpecies;
    static std::unordered_map<uint32 /*SpeciesID*/, std::array<std::array<uint32, 3>, 2> /*abilities*/> _abilitiesPerSpecies;
};

#endif // BattlePetMgr_h__
