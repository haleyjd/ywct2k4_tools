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
#include "boosters.h"
#include "romfile.h"

//=============================================================================
// Booster Packs
//=============================================================================

bool WCTBoosterPack::ReadCardList(FILE *f, const WCTBoosterList &bl, cardlist_t &list)
{
    static_assert(WCTConstants::CARDID_SIZE == sizeof(cardid_t));

    // this is allowed; it means there are no cards in that list; BLS pack has no commons
    // list for example (for whatever reason; it apparently affects the way cards are
    // distributed when the pack is opened)
    if(bl.offset == 0 || bl.len == 0)
        return true;

    // size the card list
    list.resize(bl.len);

    // adjust offset relative to ROM into file-relative offset
    const uint32_t fileoffs = bl.offset - WCTConstants::GBA_ROM_BASEADDR;

    // seek to card list and read in the shorts
    return WCTROMFile::GetVectorFromOffset(f, fileoffs, list);
}

bool WCTBoosterPack::ReadBoosterPack(FILE *f, uint32_t offset)
{
    static_assert(WCTConstants::BOOSTER_COMMONLEN_SIZE  == sizeof(uint32_t));
    static_assert(WCTConstants::BOOSTER_RARELEN_SIZE    == sizeof(uint32_t));
    static_assert(WCTConstants::BOOSTER_COMMONLIST_SIZE == sizeof(uint32_t));
    static_assert(WCTConstants::BOOSTER_RARELIST_SIZE   == sizeof(uint32_t));

    if(f == nullptr)
        return false;

    if(offset == 0)
        return false;

    // adjust offset relative to file (value read-in is relative to GBA ROM base),
    // and add in the 48 pad bytes that are all zero in this game (suspect they
    // played a role in some earlier title, such as DM5 Expert 1 / EDS)
    const uint32_t fileoffs = (offset - WCTConstants::GBA_ROM_BASEADDR) + WCTConstants::BOOSTER_PAD_LEN;

    // seek to boosterpack struct start plus the pad bytes
    if(std::fseek(f, long(fileoffs), SEEK_SET) != 0)
        return false;

    // get rares list
    m_listRares.offset = WCTROMFile::GetData<uint32_t>(f).value_or(0u);
    m_listRares.len    = WCTROMFile::GetData<uint32_t>(f).value_or(0u);
    
    // get commons list
    m_listCommons.offset = WCTROMFile::GetData<uint32_t>(f).value_or(0u);
    m_listCommons.len    = WCTROMFile::GetData<uint32_t>(f).value_or(0u);

    // read in rare cards
    if(ReadCardList(f, m_listRares, m_rares) == false)
        return false;

    // read in commons
    if(ReadCardList(f, m_listCommons, m_commons) == false)
        return false;

    return true;
}

//=============================================================================
// Booster Refs
//=============================================================================

bool WCTBoosterRefs::ReadBoosterRefs(FILE *f)
{
    static_assert(WCTConstants::BOOSTERREF_LIST_SIZE == sizeof(uint32_t));
    static_assert(WCTConstants::BOOSTERREF_ID_SIZE   == sizeof(uint32_t));

    if(f == nullptr)
        return false;

    // seek to offset of boosterrefs
    if(std::fseek(f, long(WCTConstants::OFFS_BOOSTERPACKS), SEEK_SET) != 0)
        return false;

    for(WCTBoosterRef &ref : m_refs)
    {
        ref.offset = WCTROMFile::GetData<uint32_t>(f).value_or(0u);
        ref.id     = WCTROMFile::GetData<uint32_t>(f).value_or(0u);
    }

    // read in the boosters themselves
    for(size_t i = 0; i < m_refs.size(); i++)
    {
        if(m_boosters[i].ReadBoosterPack(f, m_refs[i].offset) == false)
            return false;
    }

    return true;
}

// EOF
