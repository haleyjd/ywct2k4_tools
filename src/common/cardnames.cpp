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
#include "cardnames.h"
#include "numcards.h"
#include "romfile.h"

//
// Read in the card names from the ROM file
//
bool WCTCardNames::ReadCardNames(FILE *f)
{
    static_assert(WCTConstants::CARDNAME_OFFS_SIZE == sizeof(uint32_t));
    static_assert(WCTConstants::OFFS_CARDNAMES_END > WCTConstants::OFFS_CARDNAMES);

    if(f == nullptr)
        return false;

    // get number of cards
    m_numcards = WCTUtils::GetNumCards(f);
    if(m_numcards == 0)
        return false;

    // Full number of strings is NUMLANGUAGES * numcards (6834 normally)
    const size_t numlangs = size_t(WCTConstants::Languages::NUMLANGUAGES);
    const size_t numstrs  = numlangs * m_numcards;
    if(numstrs <= numlangs)
        return false; // safety check

    // allocate super-string
    const size_t fulltextlen = WCTConstants::OFFS_CARDNAMES_END - WCTConstants::OFFS_CARDNAMES;
    m_upText.reset(new char [fulltextlen]);

    // read in the full string
    if(WCTROMFile::GetArrayFromOffset(f, WCTConstants::OFFS_CARDNAMES, m_upText.get(), fulltextlen) == false)
        return false;

    // resize offsets array
    m_offsets.resize(numstrs);
       
    // read in the offsets
    if(WCTROMFile::GetVectorFromOffset(f, WCTConstants::OFFS_CARDNAME_OFFS, m_offsets) == false)
        return false;

    // validate offsets
    for(uint32_t &offs : m_offsets)
    {
        if(offs > fulltextlen - 1)
            offs = 0;
    }

    return true;
}

// EOF
