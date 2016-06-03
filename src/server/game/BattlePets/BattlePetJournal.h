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

#ifndef BattlePetJournal_h__
#define BattlePetJournal_h__

#include "BattlePetPackets.h"
#include "DatabaseEnv.h"

enum BattlePetMisc
{
    MAX_PET_BATTLE_SLOTS                = 3,
    MAX_BATTLE_PETS_PER_SPECIES         = 3,
    MAX_BATTLE_PETS                     = 1000,

    BATTLE_PET_CAGE_ITEM_ID             = 82800,

    SPELL_SUMMON_BATTLE_PET_DEFAULT     = 118301,
    SPELL_BATTLE_PET_TRAINING_PASSIVE   = 119467,
    SPELL_TRACK_PETS                    = 122026,
    SPELL_REVIVE_BATTLE_PETS            = 125439,
    SPELL_BATTLE_PET_TRAINING           = 125610
};

enum BattlePetDBFlags
{
    BATTLE_PET_DB_FLAG_FAVORITE         = 0x01,
    BATTLE_PET_DB_FLAG_SPELL_1_ROW_2    = 0x10,
    BATTLE_PET_DB_FLAG_SPELL_2_ROW_2    = 0x20,
    BATTLE_PET_DB_FLAG_SPELL_3_ROW_2    = 0x40
};

enum BattlePetDBSpeciesFlags
{
    SPECIES_FLAG_UNK1                   = 0x002,
    SPECIES_FLAG_UNK2                   = 0x004,
    SPECIES_FLAG_CAPTURABLE             = 0x008,
    SPECIES_FLAG_CANT_TRADE             = 0x010,
    SPECIES_FLAG_UNOBTAINABLE           = 0x020,
    SPECIES_FLAG_UNIQUE                 = 0x040,
    SPECIES_FLAG_CANT_BATTLE            = 0x080,
    SPECIES_FLAG_UNK3                   = 0x200,
    SPECIES_FLAG_ELITE                  = 0x400,
};

// 6.2.4
enum FlagsControlType
{
    FLAGS_CONTROL_TYPE_APPLY            = 1,
    FLAGS_CONTROL_TYPE_REMOVE           = 2
};

// 6.2.4
enum BattlePetError
{
    BATTLEPETRESULT_CANT_HAVE_MORE_PETS_OF_THAT_TYPE = 3,
    BATTLEPETRESULT_CANT_HAVE_MORE_PETS              = 4,
    BATTLEPETRESULT_TOO_HIGH_LEVEL_TO_UNCAGE         = 7,

    // TODO: find correct values if possible and needed (also wrong order)
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
    STATE_IS_DEAD                   = 1,
    STATE_MAX_HEALTH_BONUS          = 2,
    STATE_INTERNAL_INITIAL_LEVEL    = 17,
    STATE_STAT_POWER                = 18,
    STATE_STAT_STAMINA              = 19,
    STATE_STAT_SPEED                = 20,
    STATE_MECHANIC_POISONED         = 21,
    STATE_MECHANIC_STUNNED          = 22,
    STATE_MOD_DAMAGE_DEALT_PERCENT  = 23,
    STATE_MOD_DAMAGE_TAKEN_PERCENT  = 24,
    STATE_MOD_SPEED_PERCENT         = 25,
    STATE_RAMPING_DAMAGE_ID         = 26,
    STATE_RAMPING_DAMAGE_USES       = 27,
    STATE_CONDITION_WAS_DAMAGED_THIS_ROUND = 28,
    STATE_UNTARGETABLE              = 29,
    STATE_UNDERGROUND               = 30,
    STATE_LAST_HIT_TAKEN            = 31,
    STATE_LAST_HIT_DEALT            = 32,
    STATE_MECHANIC_FLYING           = 33,
    STATE_MECHANIC_BURNING          = 34,
    STATE_TURN_LOCK                 = 35,
    STATE_SWAP_OUT_LOCK             = 36,
    STATE_STAT_CRIT_CHANCE          = 40,
    STATE_STAT_ACCURACY             = 41,
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
    STATE_MECHANIC_CHILLED          = 52,
    STATE_MECHANIC_WEBBED           = 64,
    STATE_MOD_HEALING_DEALT_PCT     = 65,
    STATE_MOD_HEALING_TAKEN_PCT     = 66,
    STATE_MECHANIC_INVISIBLE        = 67,
    STATE_UNKILLABLE                = 68,
    STATE_STAT_DODGE                = 73,
    STATE_MECHANIC_BLEEDING         = 77,
    STATE_GENDER                    = 78, // 1 - male, 2 - female
    STATE_MECHANIC_BLIND            = 82,
    STATE_COSMETIC_WATER_BUBBLED    = 85,
    STATE_MOD_PET_TYPE_ID           = 89,
    STATE_INTERNAL_CAPTURE_BOOST    = 90,
    STATE_SPECIAL_IS_COCKROACH      = 93,
    STATE_SWAP_IN_LOCK              = 98,
    STATE_CONDITION_DID_DAMAGE_THIS_ROUND = 127,
    STATE_COSMETIC_FLY_TIER         = 128,
    STATE_MECHANIC_BOMB             = 136,
    STATE_COSMETIC_BIGGLESWORTH     = 144,
    STATE_RESILITANT                = 149,
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
BattlePetState const FamilyStates[BATTLE_PET_FAMILY_MAX] =
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

class PetBattle;

class BattlePetJournal
{
public:
    struct BattlePet
    {
        int32 GetBaseStateValue(BattlePetState state);
        void CalculateStats();

        BattlePetFamily GetFamily();
        std::vector<uint32 /*abilityId*/> GetActiveAbilities();

        WorldPackets::BattlePet::BattlePetJournalInfo JournalInfo;
        BattlePetSaveInfo SaveInfo;
    };

    explicit BattlePetJournal(WorldSession* owner);

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

    TempSummon* GetSummonedPet() { return _summonedPet; }
    void SetSummonedPet(TempSummon* pet) { _summonedPet = pet; }
    ObjectGuid GetSummonedPetGuid() { return _summonedPetGuid; }
    void SummonPet(ObjectGuid guid);

    void SendUpdates(std::vector<BattlePet> pets, bool petAdded);
    void SendError(BattlePetError error, uint32 creatureId);
    void SendJournalLock(bool acquired);

    // Pet Battles
    PetBattle* GetPetBattle() const { return _battle; }
    void SetPetBattle(PetBattle* battle) { _battle = battle; }
    bool IsInBattle() const { return !_battle; }

private:
    WorldSession* _owner;
    PetBattle* _battle = nullptr;
    ObjectGuid _summonedPetGuid;
    TempSummon* _summonedPet = nullptr;
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

#endif // BattlePetJournal_h__
