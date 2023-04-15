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
#include "romoffsets.h"

//
// Read card data from the ROM file
//
bool WCTCardData::ReadCardData(FILE *f)
{
    if(f == nullptr)
        return false;

    // get number of cards
    const uint32_t numcards = WCTUtils::GetNumCards(f);
    if(numcards == 0)
        return false;

    // Seek to start of card data
    if(std::fseek(f, long(WCTConstants::OFFS_CARDDATA), SEEK_SET) != 0)
        return false;

    // resize array
    m_carddata.resize(numcards);

    // read in the card data values
    for(uint32_t &cd : m_carddata)
    {
        static_assert(WCTConstants::CARDDATA_SIZE == sizeof(uint32_t));

        if(std::fread(&cd, WCTConstants::CARDDATA_SIZE, 1, f) != 1)
            return false; // should not hit EOF during this loop
    }

    return true;
}

// EOF