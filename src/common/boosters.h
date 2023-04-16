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

#include <array>
#include <vector>
#include "romoffsets.h"

// One list of cards contained in a booster pack (rares or commons)
struct WCTBoosterList
{
    uint32_t offset = 0;
    uint32_t len    = 0;
};

// Class representing a single booster pack
class WCTBoosterPack final
{
public:
    using cardid_t   = uint16_t;
    using cardlist_t = std::vector<cardid_t>;

    bool ReadBoosterPack(FILE *f, uint32_t offset);

    const WCTBoosterList &GetRaresFileData()   const { return m_listRares;   }
    const WCTBoosterList &GetCommonsFileData() const { return m_listCommons; }

    const cardlist_t &GetRares()   const { return m_rares;   }
    const cardlist_t &GetCommons() const { return m_commons; }

private:
    // original data from file

    WCTBoosterList m_listRares;   
    WCTBoosterList m_listCommons;
    
    // runtime data

    cardlist_t m_rares;
    cardlist_t m_commons;

    bool ReadCardList(FILE *f, const WCTBoosterList &bl, cardlist_t &list);
};

// Original file data for a boosterref structure, which points to the actual booster pack
struct WCTBoosterRef
{
    uint32_t offset = 0; // points to booster structure
    uint32_t id     = 0; // booster pack ID
};

// Class which holds boosterrefs data from the ROM
class WCTBoosterRefs final
{
public:
    using boosterrefs_t = std::array<WCTBoosterRef,  size_t(WCTConstants::NUMBOOSTERPACKS)>;
    using boosters_t    = std::array<WCTBoosterPack, size_t(WCTConstants::NUMBOOSTERPACKS)>;

    bool ReadBoosterRefs(FILE *f);

    const boosterrefs_t &GetRefs()     const { return m_refs;     }
    const boosters_t    &GetBoosters() const { return m_boosters; }

private:
    boosterrefs_t m_refs;     // ref data from file
    boosters_t    m_boosters; // read-in booster pack data
};

// EOF
