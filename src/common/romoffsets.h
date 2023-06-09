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

namespace WCTConstants
{
    // GBA ROM base address
    // Any offsets/addresses that get read in from the file will have this
    // added in already, and any such values written will need it added in.
    static constexpr uint32_t GBA_ROM_BASEADDR = 0x8000000u;

    // Header
    static constexpr int32_t  HEADER_ENTRYPT     = 0x2E;
    static constexpr uint32_t HEADER_GAMEID_OFFS = 0xA0u;
    static constexpr uint32_t HEADER_GAMEID_LEN  = 12u;
    
    // ROM size
    static constexpr long     EXPECTED_ROM_SIZE  = 16777216;

    // Languages, in the order the game stores data for them internally
    enum class Languages
    {
        JAPANESE,
        ENGLISH,
        GERMAN,
        FRENCH,
        ITALIAN,
        SPANISH,
        NUMLANGUAGES
    };

    // Number of cards (1139, unlikely to be changeable as everything else
    // is sized relative to this, and despite it being stored here, a lot
    // of places in the code have the value-or one dependent on it-hardcoded)
    static constexpr uint32_t OFFS_DEF_ALLCARD_NUM = 0x99A60u;
    static constexpr uint32_t SIZE_DEF_ALLCARD_NUM = 4;

    // g_CardIDs - array of constants used to identify cards in a way that 
    // didn't depend directly on their ordering.
    static constexpr uint32_t OFFS_CARDIDS = 0x99A64u;
    static constexpr uint32_t CARDID_SIZE  = 2;        // all card IDs are shorts

    // Card graphic palettes
    // 128 bytes each, 64 entries of 16-bit RGB555 colors. In same order as the cards by their number.
    static constexpr uint32_t OFFS_CARDPALETTES_START     = 0x9A34Cu;
    static constexpr uint32_t OFFS_CARDPALETTES_END       = 0xBDC4Cu;
    static constexpr uint32_t SIZE_ALL_CARDPALETTES_BYTES = OFFS_CARDPALETTES_END - OFFS_CARDPALETTES_START;
    static constexpr uint32_t CARDPALETTE_READ_SIZEOF     = 128u;
    static constexpr uint32_t CARDPALETTE_ENTRY_SIZE      = 2u;
    static constexpr uint32_t CARDPALETTE_NUMENTRIES      = CARDPALETTE_READ_SIZEOF / CARDPALETTE_ENTRY_SIZE;

    // Card graphics
    // 9x10 of 8x8 tiles stored in a packed 6bpp format. In same order as the cards by their number.
    static constexpr uint32_t OFFS_CARDGFX_START     = 0x0BDC4Cu;                                       // start of packed 6bpp card graphic tile data
    static constexpr uint32_t OFFS_CARDGFX_END       = 0x56E00Cu;                                       // end of packed 6bpp card graphic tile data
    static constexpr uint32_t SIZE_ALL_CARDGFX_BYTES = OFFS_CARDGFX_END - OFFS_CARDGFX_START;           // full size of card gfx area
    static constexpr uint32_t CARDGFX_TILE_WIDTH_PX  = 8;                                               // width of a single tile, in pixels
    static constexpr uint32_t CARDGFX_TILE_HEIGHT_PX = 8;                                               // height of a single tile, in pixels
    static constexpr uint32_t CARDGFX_TILEMAP_WIDTH  = 9;                                               // width of full card graphic, in tiles
    static constexpr uint32_t CARDGFX_TILEMAP_HEIGHT = 10;                                              // height of full card graphic, in tiles
    static constexpr uint32_t CARDGFX_FULLWIDTH_PX   = CARDGFX_TILE_WIDTH_PX  * CARDGFX_TILEMAP_WIDTH;  // 9 tiles * 8 px/tile
    static constexpr uint32_t CARDGFX_FULLHEIGHT_PX  = CARDGFX_TILE_HEIGHT_PX * CARDGFX_TILEMAP_HEIGHT; // 10 tiles * 8 px/tile
    static constexpr uint32_t CARDGFX_PIXEL_COUNT    = CARDGFX_FULLWIDTH_PX * CARDGFX_FULLHEIGHT_PX;    // 5760 pixels
    static constexpr uint32_t CARDGFX_BPP            = 6;                                               // 6 bpp
    static constexpr uint32_t CARDGFX_READ_SIZEOF    = CARDGFX_PIXEL_COUNT * CARDGFX_BPP / 8;           // 4320 bytes stored per graphic, packed at 6bpp

    // Start and end of card names super-string
    static constexpr uint32_t OFFS_CARDNAMES     = 0x56E00Cu;
    static constexpr uint32_t OFFS_CARDNAMES_END = 0x58ACDCu;

    // Card name offsets - array of 32-bit offsets into the card names super-string
    static constexpr uint32_t OFFS_CARDNAME_OFFS = OFFS_CARDNAMES_END;
    static constexpr uint32_t CARDNAME_OFFS_SIZE = 4;

    // Start and end of card texts super-string
    static constexpr uint32_t OFFS_CARDTEXTS     = 0x5917A4u;
    static constexpr uint32_t OFFS_CARDTEXTS_END = 0x65CF38u;

    // Card text offsets - array of 32-bit offsets into the card texts super-string
    static constexpr uint32_t OFFS_CARDTEXTS_OFFS = OFFS_CARDTEXTS_END;
    static constexpr uint32_t CARDTEXTS_OFFS_SIZE = 4;

    // Card data is DEF_ALLCARD_NUM 32-bit DWORDs that have most of the information
    // about the game's cards packed into them, with some notable exceptions
    // (things like spirit, toon, and union monsters are just sloppily hard-coded!)
    static constexpr uint32_t OFFS_CARDDATA = 0x663A00u;
    static constexpr uint32_t CARDDATA_SIZE = 4;

    // Fusion data - divided into two arrays, one for two-material fusions and the
    // other for three-material fusions. Terminated by entries of all zeroes. Each
    // entry is just a structure (or array) of four card IDs.
    static constexpr uint32_t OFFS_FUSIONS_2MAT = 0xC42EF0u;
    static constexpr uint32_t OFFS_FUSIONS_3MAT = 0xC430C8u;

    // Ritual data - there is a table of packed information on rituals which contains
    // the monster card ID, spell card ID, and required levels. It is terminated with
    // a zero entry.
    static constexpr uint32_t OFFS_RITUALDATA       = 0xC430E8u;
    static constexpr uint32_t RITUALDATA_ENTRY_SIZE = 4;

    // g_OppDecks - opponent deck lists - see oppdeck_t structure.
    static constexpr uint32_t NUMOPPDECKS            = 29;
    static constexpr uint32_t OFFS_OPPDECKS          = 0xC483ECu;
    static constexpr uint32_t OPPDECK_ORIG_SIZEOF    = 0x0C;
    static constexpr uint32_t OPPDECK_DECKLIST_OFFS  = 0;
    static constexpr uint32_t OPPDECK_DECKLIST_SIZE  = 4; // dword offset
    static constexpr uint32_t OPPDECK_LISTLEN_OFFS   = 4;
    static constexpr uint32_t OPPDECK_LISTLEN_SIZE   = 2; // short int
    static constexpr uint32_t OPPDECK_FLAGS_OFFS     = 8;
    static constexpr uint32_t OPPDECK_FLAGS_SIZE     = 2; // short int

    // g_BoosterPacks - references lists of cards contained in each booster
    // See boosterref_t structure.
    static constexpr uint32_t NUMBOOSTERPACKS        = 24;
    static constexpr uint32_t OFFS_BOOSTERPACKS      = 0xC4FF04u;
    static constexpr uint32_t BOOSTERREF_ORIG_SIZEOF = 8;
    static constexpr uint32_t BOOSTERREF_LIST_OFFS   = 0;
    static constexpr uint32_t BOOSTERREF_LIST_SIZE   = 4; // dword offset
    static constexpr uint32_t BOOSTERREF_ID_OFFS     = 4;
    static constexpr uint32_t BOOSTERREF_ID_SIZE     = 4; // dword

    // booster_t structures - pointed at by boosterref_t::pBooster
    static constexpr uint32_t BOOSTER_ORIG_SIZEOF     = 0x40;
    static constexpr uint32_t BOOSTER_PAD_LEN         = 0x30; // there are 48 bytes of zeroes, no idea why.
    static constexpr uint32_t BOOSTER_RARELIST_OFFS   = 0x30;
    static constexpr uint32_t BOOSTER_RARELIST_SIZE   = 4;    // dword offset to list of rares
    static constexpr uint32_t BOOSTER_RARELEN_OFFS    = 0x34;
    static constexpr uint32_t BOOSTER_RARELEN_SIZE    = 4;    // dword count of rare cards in list
    static constexpr uint32_t BOOSTER_COMMONLIST_OFFS = 0x38; 
    static constexpr uint32_t BOOSTER_COMMONLIST_SIZE = 4;    // dword offset to list of commons
    static constexpr uint32_t BOOSTER_COMMONLEN_OFFS  = 0x3C;
    static constexpr uint32_t BOOSTER_COMMONLEN_SIZE  = 4;    // dword count of common cards in list

    // Opponent deck names - array of offsets to the strings; 30 names * 6 languages
    static constexpr uint32_t OFFS_OPPDECKNAMES         = 0xC509BCu;
    static constexpr uint32_t NUMOPPDECKNAMES           = 30;
    static constexpr uint32_t NUMOPPDECKNAMES_LOCALIZED = NUMOPPDECKNAMES * uint32_t(Languages::NUMLANGUAGES);
    static constexpr uint32_t OPPDECKNAMES_ENTRY_SIZE   = 4; // each entry is a dword offset to the string

} // end namespace WCTConstants

// EOF
