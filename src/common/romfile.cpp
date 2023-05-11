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
#include "instructions.h"
#include "romoffsets.h"
#include "romfile.h"

//
// Check if the file looks at least minimally like a YWCT2K4 ROM
//
bool WCTROMFile::VerifyROM(FILE *f)
{
    if(f == nullptr)
        return false;

    // check length
    const long len = M_FileLength(f);
    if(len != WCTConstants::EXPECTED_ROM_SIZE)
        return false;

    // verify branch instruction at entry point (offset 0)
    const uint32_t entrypt = GetDataFromOffset<uint32_t>(f, 0).value_or(0u);
    if(WCTCode::isBranchRel24(entrypt) == false ||
       WCTCode::getBranchRel24Dest(entrypt) != WCTConstants::HEADER_ENTRYPT)
    {
        return false;
    }

    // check signature in ROM header
    char sig[WCTConstants::HEADER_GAMEID_LEN];
    if(GetCArrayFromOffset(f, WCTConstants::HEADER_GAMEID_OFFS, sig) == false)
        return false;

    return std::strncmp(sig, "YWCT2004USA", WCTConstants::HEADER_GAMEID_LEN) == 0;
}

// EOF
