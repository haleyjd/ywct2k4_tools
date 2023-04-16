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
#include "romoffsets.h"

class WCTCardNames final
{
public:
    using textstore_t = std::unique_ptr<char []>;
    using offsets_t   = std::vector<uint32_t>;

    // Read in the card names from the ROM file
    bool ReadCardNames(FILE *f);

    uint32_t GetNumCards() const { return m_numcards; }

    const char *GetName(WCTConstants::Languages language, size_t cardnum) const
    {
        const size_t idx = cardnum * size_t(WCTConstants::Languages::NUMLANGUAGES) + size_t(language);
        return (idx < m_offsets.size()) ? m_upText.get() + m_offsets[idx] : "";
    }

private:
    uint32_t    m_numcards = 0;
    textstore_t m_upText;
    offsets_t   m_offsets;
};

// EOF
