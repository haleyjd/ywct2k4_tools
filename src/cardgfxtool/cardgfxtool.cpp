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
#include "elib/m_argv.h"
#include "elib/misc.h"
#include "elib/qstring.h"
#include "hal/hal_init.h"

#include "../common/numcards.h"
#include "../common/romfile.h"
#include "cardpic.h"

static bool WriteOneCard(FILE *romfile, uint32_t cardnum, uint32_t numcards, const qstring &outloc)
{
    if(cardnum < 1 || cardnum >= numcards)
    {
        std::printf("Invalid card number %u (1 to %u)\n", cardnum, numcards);
        return false;
    }

    // cardnums are 1-based but the picture storage is 0-based
    const uint32_t cardNumToUse = cardnum - 1;
  
    WCTCardPic thePic;
    if(thePic.ReadCardPic(romfile, uint16_t(cardNumToUse)) == false)
    {
        std::printf("Could not read in picture for card %u\n", cardnum);
        return false;
    }
    
    qstring outfn;
    outfn.printf("%s/card%04u.png", outloc.c_str(), cardnum);

    // write it
    return thePic.WriteToPNG(outfn.c_str());
}

// 
// Main routine
//
void cardgfxtool_main()
{
    // init elib HAL
    HAL_Init();

    const EArgManager &args = EArgManager::GetGlobalArgs();
    const char *const *argv = args.getArgv();
    const int          argc = args.getArgc();
    FILE *romfile = nullptr;

    // need ROM file
    if(const int p = args.getArgParameters("-rom", 1); p != 0)
    {
        const char *const filename = argv[p];
        romfile = std::fopen(filename, "rb");
        if(romfile == nullptr)
        {
            std::printf("Could not open file '%s'\n", filename);
            return; // bork
        }
    }
    else
    {
        std::puts("Need a WCT2004 ROM file\n");
        return; // bork
    }

    // allow output directory spec
    qstring outloc { "." };
    if(const int p = args.getArgParameters("-out", 1); p != 0)
    {
        outloc = argv[p];
        outloc.normalizeSlashes();
        if(hal_platform.makeDirectory(outloc.c_str()) == HAL_FALSE)
        {
            std::puts("Could not creat output directory\n");
            return;
        }
    }

    // basic verify
    if(WCTROMFile::VerifyROM(romfile) == false)
    {
        std::puts("File does not look like a YWCT2K4 ROM, continue anyway? (Y/N)\n");
        std::fflush(stdout);
        char resp[2];
        if(const char *const inl = gets_s(resp, sizeof(resp)); inl != nullptr)
        {
            if(*inl == 'n' || *inl == 'N')
                return; // moo.
        }
    }

    // get number of cards
    const uint32_t numcards = WCTUtils::GetNumCards(romfile);
    if(numcards == 0)
    {
        std::puts("No cards defined in ROM, or file was unreadable\n");
        return;
    }

    // check for specific cardnum to write
    uint32_t cardnum = 0;
    if(const int p = args.getArgParameters("-card", 1); p != 0)
        cardnum = uint32_t(std::strtoul(argv[p], nullptr, 10));

    if(cardnum == 0)
    {
        // write all cards
        for(uint32_t i = 1; i < numcards; i++)
            WriteOneCard(romfile, i, numcards, outloc);
    }
    else
    {
        // write a specific card
        WriteOneCard(romfile, cardnum, numcards, outloc);
    }
}