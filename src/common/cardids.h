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

class WCTCardIDs final
{
public:
    using cardid_t = uint16_t;
    using cardids_t = std::vector<cardid_t>;

    static constexpr size_t npos = ((size_t) -1);
    static constexpr uint16_t INVALID_ID = 0;

    // Read in the 16-bit card ID constants from the ROM file
    bool ReadCardIDs(FILE *f);

    const cardids_t &GetIDs() const { return m_ids; }

    // Look up the ID for a given card by number.
    cardid_t IDForCardNum(size_t cardnum) const
    {
        return (cardnum < m_ids.size()) ? m_ids[cardnum] : INVALID_ID;
    }

    // Find a given ID in the set of card IDs and return the card number to which it
    // corresponds if found. If not found, npos is returned.
    size_t CardNumForID(cardid_t id) const;

private:
    cardids_t m_ids;
};

// EOF
