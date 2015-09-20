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

#include "ObjectMgr.h"
#include "BattlePetMgr.h"
#include "Containers.h"
#include "Player.h"
#include "WorldSession.h"

int32 BattlePetMgr::BattlePet::GetBaseStateValue(BattlePetState state)
{
    int32 value = 0;

    // get base breed stats
    auto breedState = _battlePetBreedStates.find(JournalInfo.Breed);
    if (breedState == _battlePetBreedStates.end()) // non existing breed id
        return 0;

    value = breedState->second[state];

    // modify value depending on species - not all pets have this
    auto speciesState = _battlePetSpeciesStates.find(JournalInfo.Species);
    if (speciesState != _battlePetSpeciesStates.end())
        value += speciesState->second[state];

    return value;
}

void BattlePetMgr::BattlePet::CalculateStats()
{
    float health = 0.0f;
    float power = 0.0f;
    float speed = 0.0f;
    
    health = GetBaseStateValue(STATE_STAT_STAMINA);
    power = GetBaseStateValue(STATE_STAT_POWER);
    speed = GetBaseStateValue(STATE_STAT_SPEED);

    // modify stats by quality
    for (auto itr : sBattlePetBreedQualityStore)
    {
        if (itr->Quality == JournalInfo.Quality)
        {
            health *= itr->Modifier;
            power *= itr->Modifier;
            speed *= itr->Modifier;
            break;
        }
        // TOOD: add check if pet has existing quality
    }

    // scale stats depending on level
    health *= JournalInfo.Level;
    power *= JournalInfo.Level;
    speed *= JournalInfo.Level;

    // set stats
    // round, ceil or floor? verify this
    JournalInfo.MaxHealth = uint32((round(health / 20) + 100));
    JournalInfo.Power = uint32(round(power / 100));
    JournalInfo.Speed = uint32(round(speed / 100));
}

BattlePetFamily BattlePetMgr::BattlePet::GetFamily()
{
    if (BattlePetSpeciesEntry const* speciesEntry = sBattlePetSpeciesStore.LookupEntry(JournalInfo.Species))
        return BattlePetFamily(speciesEntry->PetType);

    return BATTLE_PET_FAMILY_MAX;
}

std::unordered_map<uint16 /*BreedID*/, std::unordered_map<BattlePetState /*state*/, int32 /*value*/, std::hash<std::underlying_type<BattlePetState>::type> >> BattlePetMgr::_battlePetBreedStates;
std::unordered_map<uint32 /*SpeciesID*/, std::unordered_map<BattlePetState /*state*/, int32 /*value*/, std::hash<std::underlying_type<BattlePetState>::type> >> BattlePetMgr::_battlePetSpeciesStates;
std::unordered_map<uint32 /*SpeciesID*/, std::unordered_set<uint8 /*breed*/>> BattlePetMgr::_availableBreedsPerSpecies;
std::unordered_map<uint32 /*SpeciesID*/, uint8 /*quality*/> BattlePetMgr::_defaultQualityPerSpecies;
std::unordered_map<uint32 /*SpeciesID*/, std::array<std::array<uint32, 3>, 2> /*abilities*/> BattlePetMgr::_abilitiesPerSpecies;

void BattlePetMgr::Initialize()
{
    if (QueryResult result = LoginDatabase.Query("SELECT MAX(guid) FROM battle_pets"))
        sObjectMgr->GetGenerator<HighGuid::BattlePet>().Set((*result)[0].GetUInt64() + 1);

    for (auto itr : sBattlePetBreedStateStore)
        _battlePetBreedStates[itr->BreedID][BattlePetState(itr->State)] = itr->Value;

    for (auto itr : sBattlePetSpeciesStateStore)
        _battlePetSpeciesStates[itr->SpeciesID][BattlePetState(itr->State)] = itr->Value;

    // note: this logic does not apply to species 354, it was a test pet with all slots = 0 (npc id = 59216)
    for (auto itr : sBattlePetSpeciesXAbilityStore)
        _abilitiesPerSpecies[itr->SpeciesID][itr->Level < 10 ? 0 : 1][itr->Slot] = itr->AbilityID;

    LoadAvailablePetBreeds();
    LoadDefaultPetQualities();
}

void BattlePetMgr::LoadAvailablePetBreeds()
{
    QueryResult result = WorldDatabase.Query("SELECT speciesId, breedId FROM battle_pet_breeds");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 battle pet breeds. DB table `battle_pet_breeds` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 speciesId = fields[0].GetUInt32();
        uint16 breedId = fields[1].GetUInt16();

        if (!sBattlePetSpeciesStore.LookupEntry(speciesId))
        {
            TC_LOG_ERROR("sql.sql", "Non-existing BattlePetSpecies.db2 entry %u was referenced in `battle_pet_breeds` by row (%u, %u).", speciesId, speciesId, breedId);
            continue;
        }

        // TODO: verify breed id (3 - 12 (male) or 3 - 22 (male and female)) if needed

        _availableBreedsPerSpecies[speciesId].insert(breedId);
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u battle pet breeds.", count);
}

void BattlePetMgr::LoadDefaultPetQualities()
{
    QueryResult result = WorldDatabase.Query("SELECT speciesId, quality FROM battle_pet_quality");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 battle pet qualities. DB table `battle_pet_quality` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        uint32 speciesId = fields[0].GetUInt32();
        uint8 quality = fields[1].GetUInt8();

        if (!sBattlePetSpeciesStore.LookupEntry(speciesId))
        {
            TC_LOG_ERROR("sql.sql", "Non-existing BattlePetSpecies.db2 entry %u was referenced in `battle_pet_quality` by row (%u, %u).", speciesId, speciesId, quality);
            continue;
        }

        // TODO: verify quality (0 - 3 for player pets or 0 - 5 for both player and tamer pets) if needed

        _defaultQualityPerSpecies[speciesId] = quality;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u battle pet qualities.", uint32(_defaultQualityPerSpecies.size()));
}

uint16 BattlePetMgr::RollPetBreed(uint32 species)
{
    auto itr = _availableBreedsPerSpecies.find(species);
    if (itr == _availableBreedsPerSpecies.end())
        return 3; // default B/B

    return Trinity::Containers::SelectRandomContainerElement(itr->second);
}

uint8 BattlePetMgr::GetDefaultPetQuality(uint32 species)
{
    auto itr = _defaultQualityPerSpecies.find(species);
    if (itr == _defaultQualityPerSpecies.end())
        return 0; // default poor

    return itr->second;
}

BattlePetMgr::BattlePetMgr(WorldSession* owner)
{
    _owner = owner;
    for (uint8 i = 0; i < MAX_PET_BATTLE_SLOTS; ++i)
    {
        WorldPackets::BattlePet::BattlePetSlot slot;
        slot.Index = i;
        _slots.push_back(slot);
    }
}

void BattlePetMgr::LoadFromDB(PreparedQueryResult pets, PreparedQueryResult slots)
{
    if (pets)
    {
        do
        {
            Field* fields = pets->Fetch();
            uint32 species = fields[1].GetUInt32();

            if (BattlePetSpeciesEntry const* speciesEntry = sBattlePetSpeciesStore.LookupEntry(species))
            {
                if (GetPetCount(species) >= MAX_BATTLE_PETS_PER_SPECIES)
                {
                    TC_LOG_ERROR("misc", "Battlenet account with id %u has more than 3 battle pets of species %u", _owner->GetBattlenetAccountId(), species);
                    continue;
                }

                BattlePet pet;
                pet.JournalInfo.Guid = ObjectGuid::Create<HighGuid::BattlePet>(fields[0].GetUInt64());
                pet.JournalInfo.Species = species;
                pet.JournalInfo.Breed = fields[2].GetUInt16();
                pet.JournalInfo.Level = fields[3].GetUInt16();
                pet.JournalInfo.Exp = fields[4].GetUInt16();
                pet.JournalInfo.Health = fields[5].GetUInt32();
                pet.JournalInfo.Quality = fields[6].GetUInt8();
                pet.JournalInfo.Flags = fields[7].GetUInt16();
                pet.JournalInfo.Name = fields[8].GetString();
                pet.JournalInfo.CreatureID = speciesEntry->CreatureID;
                pet.SaveInfo = BATTLE_PET_UNCHANGED;
                pet.CalculateStats();
                _pets[pet.JournalInfo.Guid.GetCounter()] = pet;
            }
        } while (pets->NextRow());
    }

    if (slots)
    {
        uint8 i = 0; // slots->GetRowCount() should equal MAX_BATTLE_PET_SLOTS

        do
        {
            Field* fields = slots->Fetch();
            _slots[i].Index = fields[0].GetUInt8();
            auto itr = _pets.find(fields[1].GetUInt64());
            if (itr != _pets.end())
                _slots[i].Pet = itr->second.JournalInfo;
            _slots[i].Locked = fields[2].GetBool();
            i++;
        } while (slots->NextRow());
    }
}

void BattlePetMgr::SaveToDB(SQLTransaction& trans)
{
    PreparedStatement* stmt = nullptr;

    for (auto itr = _pets.begin(); itr != _pets.end();)
    {
        switch (itr->second.SaveInfo)
        {
            case BATTLE_PET_NEW:
                stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_BATTLE_PETS);
                stmt->setUInt64(0, itr->first);
                stmt->setUInt32(1, _owner->GetBattlenetAccountId());
                stmt->setUInt32(2, itr->second.JournalInfo.Species);
                stmt->setUInt16(3, itr->second.JournalInfo.Breed);
                stmt->setUInt16(4, itr->second.JournalInfo.Level);
                stmt->setUInt16(5, itr->second.JournalInfo.Exp);
                stmt->setUInt32(6, itr->second.JournalInfo.Health);
                stmt->setUInt8(7, itr->second.JournalInfo.Quality);
                stmt->setUInt16(8, itr->second.JournalInfo.Flags);
                stmt->setString(9, itr->second.JournalInfo.Name);
                trans->Append(stmt);
                itr->second.SaveInfo = BATTLE_PET_UNCHANGED;
                ++itr;
                break;
            case BATTLE_PET_CHANGED:
                stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_BATTLE_PETS);
                stmt->setUInt16(0, itr->second.JournalInfo.Level);
                stmt->setUInt16(1, itr->second.JournalInfo.Exp);
                stmt->setUInt32(2, itr->second.JournalInfo.Health);
                stmt->setUInt8(3, itr->second.JournalInfo.Quality);
                stmt->setUInt16(4, itr->second.JournalInfo.Flags);
                stmt->setString(5, itr->second.JournalInfo.Name);
                stmt->setUInt32(6, _owner->GetBattlenetAccountId());
                stmt->setUInt64(7, itr->first);
                trans->Append(stmt);
                itr->second.SaveInfo = BATTLE_PET_UNCHANGED;
                ++itr;
                break;
            case BATTLE_PET_REMOVED:
                stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_BATTLE_PETS);
                stmt->setUInt32(0, _owner->GetBattlenetAccountId());
                stmt->setUInt64(1, itr->first);
                trans->Append(stmt);
                itr = _pets.erase(itr);
                break;
            default:
                ++itr;
                break;
        }
    }

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_BATTLE_PET_SLOTS);
    stmt->setUInt32(0, _owner->GetBattlenetAccountId());
    trans->Append(stmt);

    for (WorldPackets::BattlePet::BattlePetSlot const& slot : _slots)
    {
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_BATTLE_PET_SLOTS);
        stmt->setUInt8(0, slot.Index);
        stmt->setUInt32(1, _owner->GetBattlenetAccountId());
        stmt->setUInt64(2, slot.Pet.Guid.GetCounter());
        stmt->setBool(3, slot.Locked);
        trans->Append(stmt);
    }
}

BattlePetMgr::BattlePet* BattlePetMgr::GetPet(ObjectGuid guid)
{
    auto itr = _pets.find(guid.GetCounter());
    if (itr != _pets.end())
        return &itr->second;

    return nullptr;
}

void BattlePetMgr::AddPet(uint32 species, uint32 creatureId, uint16 breed, uint8 quality, uint16 level /*= 1*/)
{
    BattlePetSpeciesEntry const* battlePetSpecies = sBattlePetSpeciesStore.LookupEntry(species);
    if (!battlePetSpecies) // should never happen
        return;

    BattlePet pet;
    pet.JournalInfo.Guid = ObjectGuid::Create<HighGuid::BattlePet>(sObjectMgr->GetGenerator<HighGuid::BattlePet>().Generate());
    pet.JournalInfo.Species = species;
    pet.JournalInfo.CreatureID = creatureId;
    pet.JournalInfo.Level = level;
    pet.JournalInfo.Exp = 0;
    pet.JournalInfo.Flags = 0;
    pet.JournalInfo.Breed = breed;
    pet.JournalInfo.Quality = quality;
    pet.JournalInfo.Name = "";
    pet.CalculateStats();
    pet.JournalInfo.Health = pet.JournalInfo.MaxHealth;
    pet.SaveInfo = BATTLE_PET_NEW;

    _pets[pet.JournalInfo.Guid.GetCounter()] = pet;

    std::vector<BattlePet> updates;
    updates.push_back(pet);
    SendUpdates(updates, true);

    _owner->GetPlayer()->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_OWN_BATTLE_PET, species);
}

void BattlePetMgr::RemovePet(ObjectGuid guid)
{
    BattlePet* pet = GetPet(guid);
    if (!pet)
        return;

    pet->SaveInfo = BATTLE_PET_REMOVED;

    // spell is not unlearned on retail
    /*if (GetPetCount(pet->PacketInfo.Species) == 0)
        if (BattlePetSpeciesEntry const* speciesEntry = sBattlePetSpeciesStore.LookupEntry(pet->PacketInfo.Species))
            _owner->GetPlayer()->RemoveSpell(speciesEntry->SummonSpellID);*/
}

uint8 BattlePetMgr::GetPetCount(uint32 species) const
{
    uint8 count = 0;
    for (auto& itr : _pets)
        if (itr.second.JournalInfo.Species == species && itr.second.SaveInfo != BATTLE_PET_REMOVED)
            count++;

    return count;
}

void BattlePetMgr::UnlockSlot(uint8 slot)
{
    if (!_slots[slot].Locked)
        return;

    _slots[slot].Locked = false;

    WorldPackets::BattlePet::PetBattleSlotUpdates updates;
    updates.Slots.push_back(_slots[slot]);
    updates.AutoSlotted = false; // what's this?
    updates.NewSlot = true; // causes the "new slot unlocked" bubble to appear
    _owner->SendPacket(updates.Write());
}

std::vector<BattlePetMgr::BattlePet> BattlePetMgr::GetLearnedPets() const
{
    std::vector<BattlePet> pets;
    for (auto& pet : _pets)
        if (pet.second.SaveInfo != BATTLE_PET_REMOVED)
            pets.push_back(pet.second);

    return pets;
}

void BattlePetMgr::CageBattlePet(ObjectGuid guid)
{
    BattlePet* pet = GetPet(guid);
    if (!pet)
        return;

    ItemPosCountVec dest;

    if (_owner->GetPlayer()->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, BATTLE_PET_CAGE_ITEM_ID, 1) != EQUIP_ERR_OK)
        return;

    Item* item = _owner->GetPlayer()->StoreNewItem(dest, BATTLE_PET_CAGE_ITEM_ID, true);
    if (!item)
        return;

    item->SetModifier(ITEM_MODIFIER_BATTLE_PET_SPECIES_ID, pet->JournalInfo.Species);
    item->SetModifier(ITEM_MODIFIER_BATTLE_PET_BREED_DATA, pet->JournalInfo.Breed | (pet->JournalInfo.Quality << 24));
    item->SetModifier(ITEM_MODIFIER_BATTLE_PET_LEVEL, pet->JournalInfo.Level);
    item->SetModifier(ITEM_MODIFIER_BATTLE_PET_DISPLAY_ID, pet->JournalInfo.CreatureID);

    // FIXME: "You create: ." - item name missing in chat
    _owner->GetPlayer()->SendNewItem(item, 1, true, true);

    RemovePet(guid);

    WorldPackets::BattlePet::BattlePetDeleted deletePet;
    deletePet.PetGuid = guid;
    _owner->SendPacket(deletePet.Write());
}

void BattlePetMgr::HealBattlePetsPct(uint8 pct)
{
    // TODO: After each Pet Battle, any injured companion will automatically
    // regain 50 % of the damage that was taken during combat
    std::vector<BattlePet> updates;

    for (auto& pet : _pets)
        if (pet.second.JournalInfo.Health != pet.second.JournalInfo.MaxHealth)
        {
            pet.second.JournalInfo.Health += CalculatePct(pet.second.JournalInfo.MaxHealth, pct);
            // don't allow Health to be greater than MaxHealth
            pet.second.JournalInfo.Health = std::min(pet.second.JournalInfo.Health, pet.second.JournalInfo.MaxHealth);
            if (pet.second.SaveInfo != BATTLE_PET_NEW)
                pet.second.SaveInfo = BATTLE_PET_CHANGED;
            updates.push_back(pet.second);
        }

    SendUpdates(updates, false);
}

void BattlePetMgr::SummonPet(ObjectGuid guid)
{
    BattlePet* pet = GetPet(guid);
    if (!pet)
        return;

    BattlePetSpeciesEntry const* speciesEntry = sBattlePetSpeciesStore.LookupEntry(pet->JournalInfo.Species);
    if (!speciesEntry)
        return;

    // TODO: set proper CreatureID for spell DEFAULT_SUMMON_BATTLE_PET_SPELL (default EffectMiscValueA is 40721 - Murkimus the Gladiator)
    _owner->GetPlayer()->CastSpell(_owner->GetPlayer(), speciesEntry->SummonSpellID ? speciesEntry->SummonSpellID : uint32(DEFAULT_SUMMON_BATTLE_PET_SPELL));

    // TODO: set pet level, quality... update fields
}

void BattlePetMgr::SendUpdates(std::vector<BattlePet> pets, bool petAdded)
{
    WorldPackets::BattlePet::BattlePetUpdates updates;
    for (auto pet : pets)
        updates.Pets.push_back(pet.JournalInfo);

    updates.PetAdded = petAdded;
    _owner->SendPacket(updates.Write());
}

void BattlePetMgr::SendError(BattlePetError error, uint32 creatureId)
{
    WorldPackets::BattlePet::BattlePetError battlePetError;
    battlePetError.Result = error;
    battlePetError.CreatureID = creatureId;
    _owner->SendPacket(battlePetError.Write());
}

WorldPackets::BattlePet::PlayerUpdate BattlePetMgr::GetPlayerUpdateInfo()
{
    WorldPackets::BattlePet::PlayerUpdate player;
    player.Guid = _owner->GetPlayer()->GetGUID();
    player.TrapAbilityID = TrapSpells[_trapLevel];
    player.TrapStatus = 4; // ?
    player.InputFlags = 6; // ?

    for (auto slot : _slots)
    {
        if (!slot.Locked && !slot.Pet.Guid.IsEmpty())
        {
            BattlePet* pet = GetPet(slot.Pet.Guid);
            if (!pet || pet->JournalInfo.Health == 0) // pet is dead
                continue;

            pet->UpdateInfo.Slot = slot.Index;
            pet->UpdateInfo.JournalInfo = &pet->JournalInfo;

            // TODO: find better solution if possible and set proper PBOID (increment it to be unique for each pet in battle)
            WorldPackets::BattlePet::BattlePetAbility ability;
            if (pet->JournalInfo.Level >= 10 && (pet->JournalInfo.Flags & BATTLE_PET_DB_FLAG_SPELL_1_ROW_2))
                ability.Id = _abilitiesPerSpecies[pet->JournalInfo.Species][1][0];
            else
                ability.Id = _abilitiesPerSpecies[pet->JournalInfo.Species][0][0];

            pet->UpdateInfo.Abilities.push_back(ability);

            if (pet->JournalInfo.Level >= 2)
            {
                if (pet->JournalInfo.Level >= 15 && (pet->JournalInfo.Flags & BATTLE_PET_DB_FLAG_SPELL_2_ROW_2))
                    ability.Id = _abilitiesPerSpecies[pet->JournalInfo.Species][1][1];
                else
                    ability.Id = _abilitiesPerSpecies[pet->JournalInfo.Species][0][1];

                ability.Slot = 1;
                pet->UpdateInfo.Abilities.push_back(ability);

                if (pet->JournalInfo.Level >= 4)
                {
                    if (pet->JournalInfo.Level >= 25 && (pet->JournalInfo.Flags & BATTLE_PET_DB_FLAG_SPELL_3_ROW_2)) // lvl 25 is max
                        ability.Id = _abilitiesPerSpecies[pet->JournalInfo.Species][1][2];
                    else
                        ability.Id = _abilitiesPerSpecies[pet->JournalInfo.Species][0][2];

                    ability.Slot = 2;
                    pet->UpdateInfo.Abilities.push_back(ability);
                }
            }

            pet->UpdateInfo.States[STATE_STAT_POWER] = pet->JournalInfo.Power;
            pet->UpdateInfo.States[STATE_STAT_STAMINA] = pet->GetBaseStateValue(STATE_STAT_STAMINA); // from max or current health?
            pet->UpdateInfo.States[STATE_STAT_SPEED] = pet->GetBaseStateValue(STATE_STAT_SPEED);
            pet->UpdateInfo.States[STATE_STAT_CRIT_CHANCE] = 5; // 5 seems to be default value
            // probably add proper family check here (pet->GetFamily() != BATTLE_PET_FAMILY_MAX)
            pet->UpdateInfo.States[FamilyStates[pet->GetFamily()]] = 1; // STATE_PASSIVE_FAMILYTYPE
            // other states (pve enemies)

            // fill auras and other stuff
            player.Pets.push_back(pet->UpdateInfo);
        }
    }

    return player;
}

// Pet Battles
void BattlePetMgr::InitializePetBattle(ObjectGuid target)
{
    WorldPackets::BattlePet::PetBattleInitialUpdate init;
    init.PlayerUpdate[0] = GetPlayerUpdateInfo();

    if (target.IsPlayer())
    {
        // CMSG_PET_BATTLE_REQUEST_PVP (battle pet duel) or Find Battle
        // TODO: send packet to the opponent too
        if (Player* opponent = ObjectAccessor::FindPlayer(target))
            init.PlayerUpdate[1] = opponent->GetSession()->GetBattlePetMgr()->GetPlayerUpdateInfo();
    }
    else
    {
        // CMSG_PET_BATTLE_REQUEST_WILD or spell casts (from spellclick - menagerie, from gossip - tamers, Kura etc.)
        // fake player - NYI
        // generate pet teams (prepared or random) - next big sql awaits, yay!
        // find out what to do with BattlePetNPCTeamMember.db2
        return;
    }
    
    // TODO: send enviros and set other fields properly

    init.WaitingForFrontPetsMaxSecs = 30;
    init.PvpMaxRoundTime = 30;
    init.CurrentPetBattleState = 1; // ?

    _owner->SendPacket(init.Write());
}
