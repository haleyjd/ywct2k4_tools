/*
  Copyright (C) 2023 James Haley
  
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
*/

#include "elib/elib.h"
#include "elib/misc.h"
#include "carddata.h"
#include "numcards.h"
#include "romfile.h"
#include "romoffsets.h"

//
// Read card data from the ROM file
//
bool WCTCardData::ReadCardData(FILE *f)
{
    static_assert(WCTConstants::CARDDATA_SIZE == sizeof(uint32_t));

    if(f == nullptr)
        return false;

    // get number of cards
    const uint32_t numcards = WCTUtils::GetNumCards(f);
    if(numcards == 0)
        return false;

    // resize array
    m_carddata.resize(numcards);

    // read in the card data values
    return WCTROMFile::GetVectorFromOffset(f, WCTConstants::OFFS_CARDDATA, m_carddata);
}

//
// Read ritual data from the ROM file
//
bool WCTRitualData::ReadRitualData(FILE *f)
{
    static_assert(WCTConstants::RITUALDATA_ENTRY_SIZE == sizeof(uint32_t));

    if(f == nullptr)
        return false;

    // seek to offset
    if(std::fseek(f, long(WCTConstants::OFFS_RITUALDATA), SEEK_SET) != 0)
        return false;

    // read DWORDs until one has a zero value (the table has 21 entries normally, but
    // could potentially be relocated in the ROM to contain more)
    while(1)
    {
        const uint32_t rd = WCTROMFile::GetData<uint32_t>(f).value_or(0u);
        if(rd == 0)
            break;  // terminated by zero entry
        m_ritualdata.push_back(rd);
    }

    return true;
}

//
// Read in a single table of fusion summon entries
//
bool WCTFusionData::ReadFusionTable(FILE *f, uint32_t offset, fusiontable_t &table)
{
    // seek to offset
    if(std::fseek(f, long(offset), SEEK_SET) != 0)
        return false;

    while(1)
    {
        fusionentry_t ent;
        ent.fusion_id = WCTROMFile::GetData<uint16_t>(f).value_or(0u);
        
        if(ent.fusion_id == 0)
            break; // terminated by zero entry

        ent.material1_id = WCTROMFile::GetData<uint16_t>(f).value_or(0u);
        ent.material2_id = WCTROMFile::GetData<uint16_t>(f).value_or(0u);
        ent.material3_id = WCTROMFile::GetData<uint16_t>(f).value_or(0u);

        table.push_back(ent);
    }

    return true;
}

//
// Read the fusion summon tables from the ROM
//
bool WCTFusionData::ReadFusionTables(FILE *f)
{
    static_assert(WCTConstants::CARDID_SIZE == sizeof(uint16_t));

    if(f == nullptr)
        return false;

    // read in fusion 2-mats
    if(ReadFusionTable(f, WCTConstants::OFFS_FUSIONS_2MAT, m_fusion2mats) == false)
        return false;

    // read in fusion 3-mats
    if(ReadFusionTable(f, WCTConstants::OFFS_FUSIONS_3MAT, m_fusion3mats) == false)
        return false;

    return true;
}

//
// Test if a card is fusion material
//
bool WCTFusionData::IsFusionMaterial(cardid_t id) const
{
    // check two-materials table
    for(const fusionentry_t &ent : m_fusion2mats)
    {
        if(id == ent.material1_id || id == ent.material2_id)
            return true;
    }

    // check three-materials table
    for(const fusionentry_t &ent : m_fusion3mats)
    {
        if(id == ent.material1_id || id == ent.material2_id || id == ent.material3_id)
            return true;
    }

    return false;
}

// EOF
