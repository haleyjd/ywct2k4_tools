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
#include "../common/boosters.h"
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

class WCTInteractiveData final
{
public:
    qstring        input;
    WCTCardNames   cardnames;
    WCTCardData    carddata;
    WCTCardIDs     cardids;
    WCTBoosterRefs boosterrefs;
};

//
// Interactive mode: Search by card name
//
static void SearchByCardName(const WCTInteractiveData &data)
{
    using namespace WCTConstants;

    // Search by name
    if(const size_t pos = data.input.findFirstOf(' '); pos != qstring::npos)
    {
        const char *const searchterm = &(data.input[pos]) + 1;
        const uint32_t numcards = data.cardnames.GetNumCards();
        for(uint32_t i = 1; i < numcards; i++)
        {
            const char *const name = data.cardnames.GetName(Languages::ENGLISH, i);
            if(M_StrCaseStr(name, searchterm) != nullptr)
            {
                std::printf("\n%04u: %s", i, name);
            }
        }
        std::puts("\n");
    }
}

//
// Interactive mode: Search by card ID
//
static void SearchByCardID(const WCTInteractiveData &data)
{
    using namespace WCTConstants;

    // Search by id
    if(const size_t pos = data.input.findFirstOf(' '); pos != qstring::npos)
    {
        const char *const arg = &(data.input[pos]) + 1;
        const uint16_t hexnum = uint16_t(std::strtoul(arg, nullptr, 16));
        if(const size_t num = data.cardids.CardNumForID(hexnum); num != 0 && num != WCTCardIDs::npos)
        {
            const char *const name = data.cardnames.GetName(Languages::ENGLISH, num);
            std::printf("\n%04u: %s\n", num, name);
        }
    }
}

//
// Interactive mode: Show all the info on a single card
//
static void ShowCardInfo(const WCTInteractiveData &data)
{
    using namespace WCTConstants;

    // Show card info
    const uint32_t numcards = data.cardnames.GetNumCards();
    const uint32_t cardnum  = uint32_t(data.input.toInt());
    if(cardnum >= 1 && cardnum < numcards)
    {
        const char *const name = data.cardnames.GetName(Languages::ENGLISH, cardnum);
        const WCTCardIDs::cardid_t id = data.cardids.IDForCardNum(cardnum);
        std::printf("\n%04u: %s | ID 0x%04hX\n", cardnum, name, id);
    
        const uint32_t cd = data.carddata.DataForCardNum(cardnum);
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

static void ViewCardList(const WCTInteractiveData &data, const WCTBoosterPack::cardlist_t &list)
{
    if(list.size() == 0)
    {
        std::printf("Card list is empty.\n");
        return;
    }

    for(WCTBoosterPack::cardid_t id : list)
    {
        const size_t cardnum = data.cardids.CardNumForID(id);
        if(cardnum != 0  && cardnum != WCTCardIDs::npos)
        {
            const char *const cardname = data.cardnames.GetName(WCTConstants::Languages::ENGLISH, cardnum);
            std::printf("0x%04hX: %04u %s\n", id, cardnum, cardname);
        }
        else
        {
            std::printf("0x%04hX: Invalid entry in pack list (%04u)\n", id, cardnum);
        }
    }
}

static void ViewPackMenu(const WCTInteractiveData &data, size_t idx)
{
    const WCTBoosterRefs::boosterrefs_t &refs  = data.boosterrefs.GetRefs();
    const WCTBoosterRefs::boosters_t    &packs = data.boosterrefs.GetBoosters();
    
    const WCTBoosterPack::cardlist_t &rares   = packs[idx].GetRares();
    const WCTBoosterPack::cardlist_t &commons = packs[idx].GetCommons();

    qstring input;
    bool exitflag = false;
    while(exitflag == false)
    {
        std::printf(
            "\nBooster Pack %zu | ID: %u\n"
            "---------------------------------------------------\n"
            "1. View rare cards (%zu)\n"
            "2. View common cards (%zu)\n"
            "3. Go back\n",
            idx, refs[idx].id, rares.size(), commons.size()
        );

        std::fflush(stdout);
        input.clear();

        char inp[64];
        if(const char *const inl = gets_s(inp, sizeof(inp)); inl != nullptr)
        {
            input = inl;
        }

        if(input.empty() == false)
        {
            input.toLower();
            switch(input[0])
            {
            case '1':
                ViewCardList(data, rares);
                break;
            case '2':
                ViewCardList(data, commons);
                break;
            case '3':
                exitflag = true;
                break;
            default:
                break;
            }
        }
    }
}

static void ViewBooster(const WCTInteractiveData &data)
{
    if(const size_t pos = data.input.findFirstOf(' '); pos != qstring::npos)
    {
        const char *const arg = &(data.input[pos]) + 1;
        const size_t packnum  = size_t(strtoull(arg, nullptr, 10));
        const size_t numpacks = data.boosterrefs.GetRefs().size();

        if(packnum < numpacks)
        {
            ViewPackMenu(data, packnum);
        }
        else
        {
            std::printf("Bad booster pack number (0 to %zu), try again.\n", numpacks - 1);
        }
    }
}

//
// Interactive mode
//
static void InteractiveMode(FILE *romfile)
{
    using namespace WCTConstants;

    WCTInteractiveData data;

    if(WCTROMFile::VerifyROM(romfile) == false)
    {
        std::puts("File does not look like a YWCT2K4 ROM, continue anyway? (Y/N)\n");
        std::fflush(stdout);
        if(const char c = std::getchar(); c == 'n' || c == 'N')
            return; // moo.
    }

    if(data.cardnames.ReadCardNames(romfile) == false)
    {
        std::printf("Failed to read in card names from ROM\n");
        return; // oink.
    }
    
    if(data.carddata.ReadCardData(romfile) == false)
    {
        std::printf("Failed to read in card data from ROM\n");
        return; // bahh.
    }

    if(data.cardids.ReadCardIDs(romfile) == false)
    {
        std::printf("Failed to read card IDs from ROM\n");
        return; // whinny!
    }

    if(data.boosterrefs.ReadBoosterRefs(romfile) == false)
    {
        std::printf("Failed to read booster packs from ROM\n");
        return;
    }

    // Output data
    const uint32_t numcards = data.cardnames.GetNumCards();

    bool exitflag = false;
    while(exitflag == false)
    {
        std::printf(
            "\nYWCT2K4 Card Lister - %u cards loaded\n"
            "---------------------------------------------------\n"
            "Input a card number to view that card.\n"
            "Input 'q' to exit.\n"
            "Input 'b' followed by a number to view a booster.\n"
            "Input 'n' followed by term to search by name.\n"
            "Input 'i' followed by a hex number to search by ID.\n",
            numcards - 1
        );

        std::fflush(stdout);
        data.input.clear();

        char inp[64];
        if(const char *const inl = gets_s(inp, sizeof(inp)); inl != nullptr)
        {
            data.input = inl;
        }

        if(data.input.empty() == false)
        {
            data.input.toLower();
            switch(data.input[0])
            {
            case 'q':
                exitflag = true;
                break;
            case 'b':
                ViewBooster(data);
                break;
            case 'n':
                SearchByCardName(data);
                break;
            case 'i':
                SearchByCardID(data);
                break;
            default:
                ShowCardInfo(data);
                break;
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
