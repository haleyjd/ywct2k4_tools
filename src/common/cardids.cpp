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

#include <algorithm>

#include "elib/elib.h"
#include "cardids.h"
#include "numcards.h"
#include "romfile.h"
#include "romoffsets.h"

//
// Read in the 16-bit card ID constants from the ROM file
//
bool WCTCardIDs::ReadCardIDs(FILE *f)
{
    static_assert(WCTConstants::CARDID_SIZE == sizeof(uint16_t));

    if(f == nullptr)
        return false;

    // get number of cards
    const uint32_t numcards = WCTUtils::GetNumCards(f);
    if(numcards == 0)
        return false;

    // resize vector
    m_ids.resize(numcards);

    // read in the IDs
    return WCTROMFile::GetVectorFromOffset(f, WCTConstants::OFFS_CARDIDS, m_ids);
}

//
// Find a given ID in the set of card IDs and return the card number to which it
// corresponds if found. If not found, npos is returned.
//
size_t WCTCardIDs::CardNumForID(cardid_t id) const
{
    const auto citr = std::find(m_ids.cbegin(), m_ids.cend(), id);
    return (citr != m_ids.cend()) ? size_t(citr - m_ids.cbegin()) : npos;
}

// EOF
