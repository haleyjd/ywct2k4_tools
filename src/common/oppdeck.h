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

class WCTOpponentDeck
{
public:
    using cardid_t = uint16_t;
    using decklist_t = std::vector<cardid_t>;

    // Read a single opponent deck from the ROM file
    bool ReadDeck(FILE *f, uint32_t offset, uint16_t len);

    const decklist_t &GetDeckList() const { return m_decklist; }

private:
    decklist_t m_decklist;
};

// Original file data for an opponent deck structure
struct WCTOppDeckData
{
    uint32_t offset   = 0; // points to card list
    uint16_t len      = 0; // number of cards in list
    uint16_t unknown1 = 0; // unknown half-word or padding
    uint16_t flags    = 0; // AI flags associated with deck
    uint16_t unknown2 = 0; // unknown half-word or padding
};

class WCTOpponentDecks
{
public:
    using rawdecks_t = std::array<WCTOppDeckData,  WCTConstants::NUMOPPDECKS>;
    using decks_t    = std::array<WCTOpponentDeck, WCTConstants::NUMOPPDECKS>;

    enum flags_e : uint16_t
    {
        FLAG_EXODIA = 0x200u // is an Exodia player
    };

    // Read all opponent decks from the ROM file
    bool ReadDecks(FILE *f);

    const rawdecks_t &GetRawData() const { return m_rawdecks; }
    const decks_t    &GetDecks()   const { return m_decks;    }

private:
    rawdecks_t m_rawdecks;
    decks_t    m_decks;
};

// EOF
