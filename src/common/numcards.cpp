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
#include "numcards.h"
#include "romfile.h"
#include "romoffsets.h"

//
// Get number of cards according to the ROM file
//
uint32_t WCTUtils::GetNumCards(FILE *f)
{
    if(f == nullptr)
        return 0;

    static_assert(WCTConstants::SIZE_DEF_ALLCARD_NUM == sizeof(uint32_t));
    return WCTROMFile::GetDataFromOffset<uint32_t>(f, WCTConstants::OFFS_DEF_ALLCARD_NUM).value_or(0u);
}

// EOF
