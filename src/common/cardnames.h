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

#pragma once

#include <vector>
#include "elib/qstring.h"
#include "romoffsets.h"

class WCTCardNames final
{
public:
    using cardnames_t = std::vector<qstring>;

    // Read in the card names from the ROM file
    bool ReadCardNames(FILE *f);

    const uint32_t     GetNumCards() const { return m_numcards; }
    const cardnames_t &GetNames()    const { return m_names;    }

    const qstring &GetName(WCTConstants::Languages language, size_t cardnum) const
    {
        const size_t idx = cardnum * size_t(WCTConstants::Languages::NUMLANGUAGES) + size_t(language);
        return idx < m_names.size() ? m_names[idx] : qstring::emptystr;
    }

private:
    uint32_t    m_numcards = 0;
    cardnames_t m_names;    
};

// EOF
