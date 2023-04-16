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
#include "../common/carddata.h"
#include "../common/cardids.h"
#include "../common/romfile.h"

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
            const char *const name = cardnames.GetName(WCTConstants::Languages::ENGLISH, i);
            std::printf("%04d: %s\n", i, name);
        }
        MaybeWait();
    }
    else
    {
        std::printf("Failed to read in card names from ROM\n");
    }
}

//
// Interactive mode
//
static void InteractiveMode(FILE *romfile)
{
    using namespace WCTConstants;

    if(WCTROMFile::VerifyROM(romfile) == false)
    {
        std::puts("File does not look like a YWCT2K4 ROM, continue anyway? (Y/N)\n");
        std::fflush(stdout);
        if(const char c = std::getchar(); c == 'n' || c == 'N')
            return; // moo.
    }

    WCTCardNames cardnames;
    if(cardnames.ReadCardNames(romfile) == false)
    {
        std::printf("Failed to read in card names from ROM\n");
        return; // oink.
    }
    
    WCTCardData carddata;
    if(carddata.ReadCardData(romfile) == false)
    {
        std::printf("Failed to read in card data from ROM\n");
        return; // bahh.
    }

    WCTCardIDs cardids;
    if(cardids.ReadCardIDs(romfile) == false)
    {
        std::printf("Failed to read card IDs from ROM\n");
        return; // whinny!
    }

    // Output data
    const uint32_t numcards = cardnames.GetNumCards();

    qstring input;
    bool    exitflag = false;
    while(exitflag == false)
    {
        std::printf(
            "\nYWCT2K4 Card Lister - %u cards loaded\n"
            "---------------------------------------------------\n"
            "Input a card number to view that card.\n"
            "Input 'q' to exit.\n"
            "Input 'n' followed by term to search by name.\n"
            "Input 'i' followed by a hex number to search by ID.\n",
            numcards - 1
        );

        std::fflush(stdout);
        input.clear();

        char inp[64];
        if(const char *const inl = gets_s(inp, sizeof(inp)); inl != nullptr)
        {
            input = inl;
        }

        input.toLower();
        if(input.startsWith('q') == true)
        {
            exitflag = true;
        }
        else if(input.startsWith('n') == true)
        {
            // Search by name
            if(const size_t pos = input.findFirstOf(' '); pos != qstring::npos)
            {
                const char *const searchterm = input.bufferAt(pos) + 1;
                for(uint32_t i = 1; i < numcards; i++)
                {
                    const char *const name = cardnames.GetName(Languages::ENGLISH, i);
                    if(M_StrCaseStr(name, searchterm) != nullptr)
                    {
                        std::printf("\n%04u: %s", i, name);
                    }
                }
                std::puts("\n");
            }
        }
        else if(input.startsWith('i') == true)
        {
            // Search by id
            if(const size_t pos = input.findFirstOf(' '); pos != qstring::npos)
            {
                const char *const arg = input.bufferAt(pos) + 1;
                const uint16_t hexnum = uint16_t(std::strtoul(arg, nullptr, 16));
                if(const size_t num = cardids.CardNumForID(hexnum); num != 0 && num != WCTCardIDs::npos)
                {
                    const char *const name = cardnames.GetName(Languages::ENGLISH, num);
                    std::printf("\n%04u: %s\n", num, name);
                }
            }
        }
        else
        {
            // Show card info
            const uint32_t cardnum = uint32_t(input.toInt());
            if(cardnum >= 1 && cardnum < numcards)
            {
                const char *const name = cardnames.GetName(Languages::ENGLISH, cardnum);
                const WCTCardIDs::cardid_t id = cardids.IDForCardNum(cardnum);
                std::printf("\n%04u: %s | ID 0x%04hX\n", cardnum, name, id);

                const uint32_t cd = carddata.DataForCardNum(cardnum);
                const CardType ct = GetCardType(cd);
                if(ct == CardType::Spell || ct == CardType::Trap)
                {
                    const SpellTrapType stt = GetSpellTrapType(cd);
                    std::printf(
                        "%s %s\n\n",
                        SafeSpellTrapTypeName(stt),
                        ct == CardType::Spell ? "Spell Card" : "Trap Card"
                    );
                }
                else
                {
                    const uint32_t        level  = GetCardLevel(cd);
                    const Attribute       attrib = GetCardAttribute(cd);
                    const MonsterCardType mtype  = GetMonsterType(cd);
                    const uint32_t        atk    = GetMonsterATK(cd);
                    const uint32_t        def    = GetMonsterDEF(cd);

                    std::printf(
                        "%s Monster Card\n"
                        "Level %u\n"
                        "Type: %s\n"
                        "Attribute: %s\n"
                        "ATK %u/DEF %u\n\n",
                        SafeMonsterCardTypeName(mtype),
                        level, 
                        SafeCardTypeName(ct), 
                        SafeAttributeName(attrib),
                        atk, def
                    );
                }
            }
            else
            {
                std::printf("%u is not a valid card number (1 to %u), try again.\n", cardnum, numcards - 1);
            }
        }
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
        std::puts("Need a WCT2004 ROM file\n");
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
    else
    {
        // interactive mode
        InteractiveMode(romfile);
    }
}

// EOF
