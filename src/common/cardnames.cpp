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

enum class namestate_e
{
    LOOKFORNUL,
    SCANPASTNULS
};

//
// Read in the card names from the ROM file
//
bool WCTCardNames::ReadCardNames(FILE *f)
{
    if(f == nullptr)
        return false;

    // get number of cards
    m_numcards = WCTUtils::GetNumCards(f);
    if(m_numcards == 0)
        return false;

    // Seek to start of OFFS_CARDNAMES
    if(std::fseek(f, long(WCTConstants::OFFS_CARDNAMES), SEEK_SET) != 0)
        return false;

    // Full number of strings is NUMLANGUAGES * numcards
    const size_t numlangs = size_t(WCTConstants::Languages::NUMLANGUAGES);
    const size_t numstrs  = numlangs * m_numcards;
    if(numstrs <= numlangs)
        return false; // safety check

    m_names.resize(numstrs);

    namestate_e state = namestate_e::LOOKFORNUL;
    size_t idx = numlangs; // first set is empty
    while(1)
    {
        char c = '\0';
        if(std::fread(&c, 1, 1, f) != 1)
            return false; // should not hit EOF during this scan

        bool bustloop = false;
        switch(state)
        {
        case namestate_e::LOOKFORNUL:
            if(c == '\0')
            {
                state = namestate_e::SCANPASTNULS;
                ++idx; // next string
                if(idx == m_names.size())
                {
                    bustloop = true;
                    break; // finished
                }
            }
            else
            {
                m_names[idx] += c;
            }
            break;
        case namestate_e::SCANPASTNULS:
            if(c != '\0')
            {
                state = namestate_e::LOOKFORNUL;
                m_names[idx] += c;
            }
            break;
        }

        if(bustloop == true)
            break; // done
    }

    return true;
}

// EOF
