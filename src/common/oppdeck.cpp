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
#include "oppdeck.h"
#include "romfile.h"

/*
    ******* RANT TIME *******
    So why are the opponent's decks in this game such total dogfood, you ask?
    Many of them aren't fully specified. The game uses a system similar to what
    it uses to build your starter decks at the beginning of the game to fill up
    a CPU's deck until it has 40 cards, primarily using the myriad of trash 
    normal monster cards they flooded this game with (while simultaneously 
    needlessly disabling tons of cards from WWE:SDD and earlier games that are
    STILL SUPPORTED IN THE GAME'S CODE... more on that later...) and still 
    neglecting to even reimplement more than 3 of the 20+ cards removed from
    DM6 Expert 2 because they were "OCG only" (several of them were in fact out
    in the starter decks already, so it was never good reasoning).

    I don't see any excuse for this other than lack of interest and 
    understanding from the devs. There's 3MB of empty space in the ROM, but it
    was easier to spam trash cards through an automated system than spend any
    time doing hard game design work, I guess. This game was phoned-in and
    is easily the worst entry in the Expert series next to the international
    EDS with its barely 800-large card pool. Even it felt more spirited and
    alive than this though - the characters could talk to you at least - and
    was more understandable given the state of TCG when it came out. This is
    years later and still just using excuses about why it's behind the actual
    game in card support.

    Most of the effort was already going toward NT on DS, so that explains part
    of why this game was treated like a stepchild. Yet this game engine is NOT
    simply a regurgitation of EDS/SDD - a ton of stuff is actually heavily 
    reworked and I suspect the code is shared with NT, but I've not verified 
    yet. There are references to things in this game's code that were never 
    supported in any of the GBA games before it (nor in this one, even) but 
    which ARE present in NT. I suspect there was a corporate edict to keep this
    game watered down (and cheaper to make) so that hype could be reserved for
    the bigger - and more expensive - NT when it came out a year-ish later.

    This is why I've singled this game out as being in need of a ROM hack, a
    20-year anniversary update if you will. SDD would be a better base game to
    build a hack from, but it's already fun enough that it doesn't feel like it
    inherently needs patching in the way this one does, IMHO.

    The cool thing is that because the array of decks uses offsets to find the 
    individual deck lists, it should be trivial to move these around in the ROM
    to build better decks for the opponents.
*/

//
// Read a single opponent decklist from the ROM file
//
bool WCTOpponentDeck::ReadDeck(FILE *f, uint32_t offset, uint16_t len)
{
    static_assert(WCTConstants::CARDID_SIZE == sizeof(cardid_t));

    if(f == nullptr)
        return false;

    // This is allowed as far as I know; such an opponent would have a deck
    // made entirely of filler junk cards. Some of them practically do
    // already (*cough*Tristan*cough*).
    if(offset == 0 || len == 0)
        return true;

    // adjust decklist offset relative to ROM base
    const uint32_t fileoffs = offset - WCTConstants::GBA_ROM_BASEADDR;

    // pre-size vector
    m_decklist.resize(len);

    // read the card IDs at the offset
    return WCTROMFile::GetVectorFromOffset(f, fileoffs, m_decklist);
}

//
// Read all opponent decks from the ROM file
//
bool WCTOpponentDecks::ReadDecks(FILE *f)
{
    static_assert(WCTConstants::OPPDECK_DECKLIST_SIZE == sizeof(uint32_t));
    static_assert(WCTConstants::OPPDECK_LISTLEN_SIZE  == sizeof(uint16_t));
    static_assert(WCTConstants::OPPDECK_FLAGS_SIZE    == sizeof(uint16_t));

    // seek to offset of oppdecks list
    if(std::fseek(f, long(WCTConstants::OFFS_OPPDECKS), SEEK_SET) != 0)
        return false;

    // read the deck definition structures
    for(WCTOppDeckData &raw : m_rawdecks)
    {
        raw.offset   = WCTROMFile::GetData<uint32_t>(f).value_or(0u);
        raw.len      = WCTROMFile::GetData<uint16_t>(f).value_or(0u);
        raw.unknown1 = WCTROMFile::GetData<uint16_t>(f).value_or(0u);
        raw.flags    = WCTROMFile::GetData<uint16_t>(f).value_or(0u);
        raw.unknown2 = WCTROMFile::GetData<uint16_t>(f).value_or(0u);
    }

    // read out the deck lists
    for(size_t i = 0; i < m_rawdecks.size(); i++)
    {
        if(m_decks[i].ReadDeck(f, m_rawdecks[i].offset, m_rawdecks[i].len) == false)
            return false;
    }

    return true;
}

// EOF
