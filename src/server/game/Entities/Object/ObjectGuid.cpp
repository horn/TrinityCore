/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "ObjectGuid.h"
#include "World.h"
#include "ObjectMgr.h"
#include <sstream>
#include <iomanip>

ObjectGuid const ObjectGuid::Empty = ObjectGuid();

char const* ObjectGuid::GetTypeName(HighGuid high)
{
    switch (high)
    {
        case HIGHGUID_ITEM:         return "Item";
        case HIGHGUID_PLAYER:       return "Player";
        case HIGHGUID_GAMEOBJECT:   return "Gameobject";
        case HIGHGUID_TRANSPORT:    return "Transport";
        case HIGHGUID_UNIT:         return "Creature";
        case HIGHGUID_PET:          return "Pet";
        case HIGHGUID_VEHICLE:      return "Vehicle";
        case HIGHGUID_DYNAMICOBJECT: return "DynObject";
        case HIGHGUID_CORPSE:       return "Corpse";
        case HIGHGUID_AREATRIGGER:  return "AreaTrigger";
        case HIGHGUID_BATTLEGROUND: return "Battleground";
        case HIGHGUID_MO_TRANSPORT: return "MoTransport";
        case HIGHGUID_INSTANCE:     return "InstanceID";
        case HIGHGUID_GROUP:        return "Group";
        case HIGHGUID_GUILD:        return "Guild";
        default:
            return "<unknown>";
    }
}

std::string ObjectGuid::ToString() const
{
    std::ostringstream str;
    str << "GUID Full: 0x" << std::hex << std::setw(16) << std::setfill('0') << _low << std::dec;
    str << " Type: " << GetTypeName();
    if (HasEntry())
        str << (IsPet() ? " Pet number: " : " Entry: ") << GetEntry() << " ";

    str << " Low: " << GetCounter();
    return str.str();
}

std::vector<uint8> ObjectGuid::GetRawValue() const
{
    std::vector<uint8> raw(16);
    memcpy(raw.data(), this, sizeof(*this));
    return raw;
}

void ObjectGuid::SetRawValue(std::vector<uint8> const& guid)
{
    ASSERT(guid.size() == sizeof(*this));
    memcpy(this, guid.data(), sizeof(*this));
}

void PackedGuid::Set(ObjectGuid guid)
{
    uint8 lowMask = 0;
    uint8 highMask = 0;
    _packedGuid << uint8(lowMask);
    _packedGuid << uint8(highMask);

    uint8 packed[8];
    if (size_t packedSize = _packedGuid.PackUInt64(guid._low, &lowMask, packed))
        _packedGuid.append(packed, packedSize);
    if (size_t packedSize = _packedGuid.PackUInt64(guid._high, &highMask, packed))
        _packedGuid.append(packed, packedSize);

    _packedGuid.put(0, lowMask);
    _packedGuid.put(1, highMask);
}

template<HighGuid high>
uint64 ObjectGuidGenerator<high>::Generate()
{
    if (_nextGuid >= ObjectGuid::GetMaxCounter(high) - 1)
    {
        TC_LOG_ERROR("", "%s guid overflow!! Can't continue, shutting down server. ", ObjectGuid::GetTypeName(high));
        World::StopNow(ERROR_EXIT_CODE);
    }
    return _nextGuid++;
}

ByteBuffer& operator<<(ByteBuffer& buf, ObjectGuid const& guid)
{
    buf << guid.WriteAsPacked();
    return buf;
}

ByteBuffer& operator>>(ByteBuffer& buf, ObjectGuid& guid)
{
    buf >> guid.ReadAsPacked();
    return buf;
}

ByteBuffer& operator<<(ByteBuffer& buf, PackedGuid const& guid)
{
    buf.append(guid._packedGuid);
    return buf;
}

ByteBuffer& operator>>(ByteBuffer& buf, PackedGuidReader const& guid)
{
    uint8 lowMask, highMask;
    buf >> lowMask >> highMask;
    buf.ReadPackedUInt64(lowMask, guid.GuidPtr->_low);
    buf.ReadPackedUInt64(highMask, guid.GuidPtr->_high);
    return buf;
}

std::ostream& operator<<(std::ostream& stream, ObjectGuid const& guid)
{
    std::ostringstream tmp;
    tmp << std::hex << std::setw(16) << std::setfill('0') << guid._high << std::setw(16) << std::setfill('0') << guid._low;
    stream << tmp.str();
    return stream;
}

template uint64 ObjectGuidGenerator<HIGHGUID_ITEM>::Generate();
template uint64 ObjectGuidGenerator<HIGHGUID_PLAYER>::Generate();
template uint64 ObjectGuidGenerator<HIGHGUID_GAMEOBJECT>::Generate();
template uint64 ObjectGuidGenerator<HIGHGUID_TRANSPORT>::Generate();
template uint64 ObjectGuidGenerator<HIGHGUID_UNIT>::Generate();
template uint64 ObjectGuidGenerator<HIGHGUID_PET>::Generate();
template uint64 ObjectGuidGenerator<HIGHGUID_VEHICLE>::Generate();
template uint64 ObjectGuidGenerator<HIGHGUID_DYNAMICOBJECT>::Generate();
template uint64 ObjectGuidGenerator<HIGHGUID_CORPSE>::Generate();
template uint64 ObjectGuidGenerator<HIGHGUID_AREATRIGGER>::Generate();
template uint64 ObjectGuidGenerator<HIGHGUID_BATTLEGROUND>::Generate();
template uint64 ObjectGuidGenerator<HIGHGUID_INSTANCE>::Generate();
template uint64 ObjectGuidGenerator<HIGHGUID_GROUP>::Generate();
template uint64 ObjectGuidGenerator<HIGHGUID_GUILD>::Generate();