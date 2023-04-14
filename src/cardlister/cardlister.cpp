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
#include "../common/cardnames.h"

static bool s_waitForInput = false;

//
// Handy when debugging
//
static void MaybeWait()
{
    if(s_waitForInput == true) 
        std::getchar();
}

//
// Dump a numbered list of the English card names
//
static void DumpCardNames(FILE *romfile)
{
    // read in and output the card names only
    WCTCardNames cardnames;
    if(cardnames.ReadCardNames(romfile) == true)
    {
        const uint32_t numcards = cardnames.GetNumCards();
        for(uint32_t i = 0; i < numcards; i++)
        {
            const qstring &name = cardnames.GetName(WCTConstants::Languages::ENGLISH, i);
            std::printf("%04d: %s\n", i, name.c_str());
        }
        MaybeWait();
    }
    else
    {
        std::printf("Failed to read in card names from ROM\n");
    }
}

// 
// Main routine
//
void cardlister_main()
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
        std::printf("Need a WCT2004 ROM file\n");
        return; // bork
    }

    if(args.findArgument("-wait") == true)
    {
        s_waitForInput = true;
    }

    if(args.findArgument("-names") == true)
    {
        // dump names only
        DumpCardNames(romfile);
    }
}

// EOF
