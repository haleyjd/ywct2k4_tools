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

#include <algorithm>

#include "elib/elib.h"
#include "elib/m_argv.h"
#include "elib/misc.h"
#include "elib/qstring.h"
#include "hal/hal_init.h"
#include "../common/boosters.h"
#include "../common/cardnames.h"
#include "../common/carddata.h"
#include "../common/cardids.h"
#include "../common/iddb.h"
#include "../common/oppdeck.h"
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
// Interactive mode: object holding data to pass between routines
//
class WCTInteractiveData final
{
public:
    qstring          input;
    WCTCardNames     cardnames;
    WCTCardData      carddata;
    WCTRitualData    ritualdata;
    WCTFusionData    fusiondata;
    WCTCardIDs       cardids;
    WCTBoosterRefs   boosterrefs;
    WCTOpponentDecks decks;
    WCTIDDatabase    db;
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
        bool found = false;
        for(uint32_t i = 1; i < numcards; i++)
        {
            const char *const name = data.cardnames.GetName(Languages::ENGLISH, i);
            if(M_StrCaseStr(name, searchterm) != nullptr)
            {
                std::printf("\n%04u: %s", i, name);
                found = true;
            }
        }
        if(found == false)
            std::puts("\nNo game results were found.");

        // also check user database
        const WCTIDDatabase::map_t &dbmap = data.db.GetMap();
        bool firstdb = true;
        for(const auto &pair : dbmap)
        {
            if(pair.second.containsNoCase(searchterm) == true)
            {
                if(firstdb == true)
                {
                    std::printf("\n%sResults from user database:", found ? "\n" : "");
                    firstdb = false;
                }
                std::printf("\n%04hX (%hu): %s", pair.first, pair.first, pair.second.c_str());
                found = true;
            }
        }
        if(found == false)
            std::puts("\nNo database results were found.");
        else
            std::puts(" ");
    }
}

//
// Interactive mode: Search by card ID
//
static void SearchByCardID(const WCTInteractiveData &data)
{
    using namespace WCTConstants;

    // Thumb codegen is awful with constants so this will help.
    enum class idop_e
    {
        LOOKUP, // normal lookup
        ADD,    // add to previous searched id
        SUB     // subtract from previous searched id
    };

    static uint16_t lastid = 0;

    // Search by id
    if(const size_t pos = data.input.findFirstOf(' '); pos != qstring::npos)
    {
        const char *arg = &(data.input[pos]) + 1;
        
        idop_e op = idop_e::LOOKUP;
        switch(*arg)
        {
        case '+':
            op = idop_e::ADD;
            ++arg;
            break;
        case '-':
            op = idop_e::SUB;
            ++arg;
            break;
        default:
            break;
        }
        
        uint16_t id = uint16_t(std::strtoul(arg, nullptr, 16));
        switch(op)
        {
        case idop_e::ADD:
            id = lastid + id;
            break;
        case idop_e::SUB:
            id = lastid - id;
            break;
        default:
            break;
        }
        lastid = id;

        if(const size_t num = data.cardids.CardNumForID(id); num != 0 && num != WCTCardIDs::npos)
        {
            const char *const name = data.cardnames.GetName(Languages::ENGLISH, num);
            std::printf("\n%04hX: %04u %s (%hu)\n", id, num, name, id);
        }
        else
        {
            // check in the user database, which can store the IDs of cards that are supported in
            // the game's code but NOT present in its data normally (there are a literal ton of
            // these and I need help keeping track of them all).
            const qstring &dbname = data.db.GetNameForID(id);
            if(dbname.empty() == false)
                std::printf("\n%04hX (%hu) has been defined by the user as \"%s\"\n", id, id, dbname.c_str());
            else
                std::printf("\n%04hX not found; look up %hu on YP\n", id, id);
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
        std::printf("\n%04u: %s | ID 0x%04hX (%hu)\n", cardnum, name, id, id);
    
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

//
// Interactive mode: View a list of cards in a booster pack or deck
//
static void ViewCardList(const WCTInteractiveData &data, const std::vector<uint16_t> &list)
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
            std::printf("0x%04hX: Invalid entry in card list (%04u)\n", id, cardnum);
        }
    }
}

//
// Interactive mode: Execute the booster pack viewing menu
//
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

//
// Interactive mode: interpret the view booster command for the main loop
//
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
// Interactive mode: interpret the deck command for the main loop
//
static void ViewDeck(const WCTInteractiveData &data)
{
    if(const size_t pos = data.input.findFirstOf(' '); pos != qstring::npos)
    {
        const WCTOpponentDecks::rawdecks_t &rawdecks = data.decks.GetRawData();
        const WCTOpponentDecks::decks_t    &decks    = data.decks.GetDecks();

        const char *const arg = &(data.input[pos]) + 1;
        const size_t decknum  = size_t(strtoull(arg, nullptr, 10));
        const size_t numdecks = rawdecks.size();

        if(decknum < numdecks)
        {
            const WCTOppDeckData  &rawdeck = rawdecks[decknum];
            const WCTOpponentDeck &deck    = decks[decknum];

            std::printf(
                "\nOpponent Deck %zu - %hu cards | AI Flags: %04hX\n"
                "---------------------------------------------------\n",
                decknum, rawdeck.len, rawdeck.flags
            );
            ViewCardList(data, deck.GetDeckList());
        }
        else
        {
            std::printf("Bad deck number (0 to %zu), try again.\n", numdecks - 1);
        }
    }
}

//
// Interactive mode: add a card ID to name mapping into the user database
//
static void AddIDToDatabase(WCTInteractiveData &data)
{
    if(data.db.HasError())
    {
        std::puts("\nFunction unavailable.\n");
        return;
    }

    if(const size_t pos = data.input.findFirstOf(' '); pos != qstring::npos)
    {
        const char *const arg = &(data.input[pos]) + 1;
        const uint16_t id = uint16_t(std::strtoul(arg, nullptr, 16));

        // don't allow aliasing built-in IDs; there's no point
        if(const size_t num = data.cardids.CardNumForID(id); num != WCTCardIDs::npos)
        {
            std::printf("\nThat card is already defined by the game as card #%zu.\n", num);
            return;
        }

        // is there already an ID with that name?
        const qstring &oldname = data.db.GetNameForID(id);
        if(oldname.empty() == false)
        {
            std::printf("\n%04hX is mapped to \"%s\", continue anyway? (Y/N)\n", id, oldname.c_str());
            std::fflush(stdout);
            char resp[2];
            if(const char *const inl = gets_s(resp, sizeof(resp)); inl != nullptr)
            {
                if(*inl == 'n' || *inl == 'N')
                    return; // moo.
            }
        }

        // Get card name
        std::puts("\nEnter a card name: ");
        std::fflush(stdout);

        char inp[256]; // methinks it a reasonable limit
        if(const char *const inl = gets_s(inp, sizeof(inp)); inl != nullptr)
        {
            if(std::strlen(inl) != 0)
            {
                data.db.SetMapping(id, inl);
                data.db.SaveToFile("cardids.json");
                std::printf("Defined %04hX (%hu) as \"%s\"\n", id, id, inl);
            }
            else
                std::puts("Empty card names are not allowed, ignored.\n");
        }
    }
}

//
// Interactive mode: remove a card ID to name mapping from the user database
//
static void RemoveDatabaseID(WCTInteractiveData &data)
{
    if(data.db.HasError())
    {
        std::puts("\nFunction unavailable.\n");
        return;
    }

    if(const size_t pos = data.input.findFirstOf(' '); pos != qstring::npos)
    {
        const char *const arg = &(data.input[pos]) + 1;
        const uint16_t id = uint16_t(std::strtoul(arg, nullptr, 16));

        // is there a mapping for that?
        const qstring &name = data.db.GetNameForID(id);
        if(name.empty() == true)
        {
            std::printf("\nThere is no mapping for ID %04hX (%hu).\n", id, id);
            return; // moo.
        }

        std::printf("\nAre you sure you want to remove the mapping for %04hX to \"%s\"? (Y/N)\n", id, name.c_str());
        std::fflush(stdout);
        char resp[2];
        if(const char *const inl = gets_s(resp, sizeof(resp)); inl != nullptr)
        {
            if(*inl == 'n' || *inl == 'N')
                return; // moo.
        }

        data.db.RemoveMapping(id);
        data.db.SaveToFile("cardids.json");
        std::printf("Removed definition of %04hX\n", id);
    }
}

// Used to return info on a fusion or ritual card
struct frcinfo_t
{
    qstring name;
    size_t  num;  // may be npos
};

//
// Look up info on a single fusion or ritual card entry
//
static frcinfo_t GetFusionRitualCardInfo(const WCTInteractiveData &data, uint16_t id)
{
    frcinfo_t ret;
    ret.num = data.cardids.CardNumForID(id);
    if(ret.num == WCTCardIDs::npos)
    {
        ret.num = 0;

        // check user database
        const qstring &name = data.db.GetNameForID(id);
        if(name.empty() == false)
        {
            ret.name = "User-defined \"" + name + "\"";
        }
        else
        {
            ret.name = "Unknown";
        }
    }
    else
    {
        ret.name = data.cardnames.GetName(WCTConstants::Languages::ENGLISH, ret.num);
    }
    return ret;
}

//
// Interactive mode: View ritual summons data
//
static void ViewRitualSummons(const WCTInteractiveData &data)
{
    const WCTRitualData::ritualdata_t &rds = data.ritualdata.GetData();

    std::printf(
        "\nRitual Summons Data\n"
        "---------------------------------------------------\n"
    );

    uint32_t entry = 0;
    for(uint32_t rd : rds)
    {
        const uint32_t monsterid = WCTConstants::GetRitualMonster(rd);
        const uint32_t spellid   = WCTConstants::GetRitualSpell(rd);
        const uint32_t levels    = WCTConstants::GetRitualLevels(rd);

        const frcinfo_t monInfo   = GetFusionRitualCardInfo(data, monsterid);
        const frcinfo_t spellInfo = GetFusionRitualCardInfo(data, spellid);

        std::printf(
            "Ritual entry %u:\n"
            "Monster: %04hX (%04hu): %04zu %s\n"
            "Spell:   %04hX (%04hu): %04zu %s\n"
            "Levels:  %u\n\n",
            entry++,
            monsterid, monsterid, monInfo.num,   monInfo.name.c_str(),
            spellid,   spellid,   spellInfo.num, spellInfo.name.c_str(),
            levels
        );
    }
}

//
// Interactive mode: view fusion summons data
//
static void ViewFusionSummons(const WCTInteractiveData &data)
{
    const WCTFusionData::fusiontable_t &twomats   = data.fusiondata.GetFusion2Mats();
    const WCTFusionData::fusiontable_t &threemats = data.fusiondata.GetFusion3Mats();

    std::printf(
        "\nFusion Summons Data\n"
        "---------------------------------------------------\n"
    );

    uint32_t entry = 0;
    for(const WCTFusionData::fusionentry_t &ent : twomats)
    {
        const frcinfo_t monInfo  = GetFusionRitualCardInfo(data, ent.fusion_id);
        const frcinfo_t mat1Info = GetFusionRitualCardInfo(data, ent.material1_id);
        const frcinfo_t mat2Info = GetFusionRitualCardInfo(data, ent.material2_id);

        std::printf(
            "2-Mat %02u  : %04hX (%04hu): %04zu %s\n"
            "Material 1: %04hX (%04hu): %04zu %s\n"
            "Material 2: %04hX (%04hu): %04zu %s\n\n",
            entry++,
            ent.fusion_id,    ent.fusion_id,    monInfo.num,  monInfo.name.c_str(),
            ent.material1_id, ent.material1_id, mat1Info.num, mat1Info.name.c_str(),
            ent.material2_id, ent.material2_id, mat2Info.num, mat2Info.name.c_str()
        );
    }

    entry = 0;
    for(const WCTFusionData::fusionentry_t &ent : threemats)
    {
        const frcinfo_t monInfo  = GetFusionRitualCardInfo(data, ent.fusion_id);
        const frcinfo_t mat1Info = GetFusionRitualCardInfo(data, ent.material1_id);
        const frcinfo_t mat2Info = GetFusionRitualCardInfo(data, ent.material2_id);
        const frcinfo_t mat3Info = GetFusionRitualCardInfo(data, ent.material3_id);

        std::printf(
            "3-Mat %02u  : %04hX (%04hu): %04zu %s\n"
            "Material 1: %04hX (%04hu): %04zu %s\n"
            "Material 2: %04hX (%04hu): %04zu %s\n"
            "Material 3: %04hX (%04hu): %04zu %s\n\n",
            entry++,
            ent.fusion_id,    ent.fusion_id,    monInfo.num,  monInfo.name.c_str(),
            ent.material1_id, ent.material1_id, mat1Info.num, mat1Info.name.c_str(),
            ent.material2_id, ent.material2_id, mat2Info.num, mat2Info.name.c_str(),
            ent.material3_id, ent.material3_id, mat3Info.num, mat3Info.name.c_str()
        );
    }
}

//
// While having significant overlap, this is a way to view cards that are Not Good(TM),
// in their order of badness, and is not related to the "filler" table, which I also refer
// to as "junk" in the RE project files.
//
static void ViewJunkCards(const WCTInteractiveData &data)
{
    using namespace WCTConstants;

    const WCTCardData::carddata_t &carddata = data.carddata.GetData();

    struct badcard_t
    {
        uint16_t    id;
        size_t      num;
        uint32_t    atk;
        uint32_t    def;
        uint32_t    lv;
        uint32_t    sacs;
        const char *name;
    };
    std::vector<badcard_t> badcards;

    for(size_t i = 1; i < carddata.size(); i++)
    {
        uint32_t cd = carddata[i];

        // only monsters
        const CardType ct = GetCardType(cd);
        if(ct == CardType::Spell || ct == CardType::Trap)
            continue;

        // only normal monsters
        const MonsterCardType mtype = GetMonsterType(cd);
        if(mtype != MonsterCardType::Normal)
            continue;

        const uint16_t id = data.cardids.IDForCardNum(i);

        // exclude cards with special support or used by key anime characters in some cases
        static const uint16_t excludeIDs[] {
            0x142D, 0xFC9, 0x112D, 0x1453, 0xFB8, 0xFB9, 0xFB7, 0xFBA,
            0xFC8, 0x11CB, 0xFE4, 0x1464, 0x1288, 0xFCF, 0x1375, 0x10BC,
            0x1297, 0x1123, 0xFAE, 0x1126, 0x1414, 0x127B
        };
        if(std::find(std::begin(excludeIDs), std::end(excludeIDs), id) != std::end(excludeIDs))
            continue;

        // of normal monsters, not fusion materials
        if(data.fusiondata.IsFusionMaterial(id) == true)
            continue;

        const uint32_t lv  = GetCardLevel(cd);
        const uint32_t atk = GetMonsterATK(cd);
        const uint32_t def = GetMonsterDEF(cd);
        // ATK/DEF criteria: if Lv7+, yes; if Lv5+, ATK < 2400, DEF != 3000; if Lv4-, ATK < 1500 but not DEF >= 2000
        if(lv >= 7 || (lv >= 5 && atk < 2400 && def != 3000) || (atk < 1500 && def < 2000))
        {
            badcard_t bc;
            bc.id   = id;
            bc.num  = i;
            bc.atk  = atk;
            bc.def  = def;
            bc.lv   = lv;
            bc.name = data.cardnames.GetName(Languages::ENGLISH, i);
            bc.sacs = lv >= 7 ? 2 : lv >= 5 ? 1 : 0;

            badcards.push_back(bc); // BAD card, BAD!
        }
    }

    // sort by badness
    std::sort(badcards.begin(), badcards.end(), [] (const badcard_t &bc1, const badcard_t &bc2) -> bool {
        if(bc1.sacs > bc2.sacs)
        {
            return true;
        }
        else if(bc1.sacs == bc2.sacs)
        {
            if(bc1.atk < bc2.atk)
                return true;
            else
                return false;
        }
        else
        {
            return false;
        }
    });

    // view the bad cards
    euint counter = 0;
    for(const badcard_t &bc : badcards)
    {
        std::printf("\n%u. %04u: %s | ID 0x%04hX (%hu)\n", counter + 1, bc.num, bc.name, bc.id, bc.id);
        std::printf(
            "Level %u\n"
            "ATK %u/DEF %u\n",
            bc.lv, 
            bc.atk, bc.def
        );
        if(++counter % 10 == 0)
        {
            std::puts("Show more bad cards? (Y/N)");
            std::fflush(stdout);
            char resp[2];
            if(const char *const inl = gets_s(resp, sizeof(resp)); inl != nullptr)
            {
                if(*inl == 'n' || *inl == 'N')
                    break; // moo.
            }
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

    // init the ID database
    if(data.db.LoadFromFile("cardids.json") == false)
    {
        std::printf("Warning: could not load cardid db:\n %s\n", data.db.GetErrors().c_str());
    }

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

    if(data.cardnames.ReadCardNames(romfile) == false)
    {
        std::puts("Failed to read in card names from ROM\n");
        return; // oink.
    }
    
    if(data.carddata.ReadCardData(romfile) == false)
    {
        std::puts("Failed to read in card data from ROM\n");
        return; // bahh.
    }

    if(data.cardids.ReadCardIDs(romfile) == false)
    {
        std::puts("Failed to read card IDs from ROM\n");
        return; // whinny!
    }

    if(data.boosterrefs.ReadBoosterRefs(romfile) == false)
    {
        std::puts("Failed to read booster packs from ROM\n");
        return;
    }

    if(data.decks.ReadDecks(romfile) == false)
    {
        std::puts("Failed to read opponent decks from ROM\n");
        return;
    }

    if(data.fusiondata.ReadFusionTables(romfile) == false)
    {
        std::puts("Failed to read fusion summons data from ROM\n");
        return;
    }

    if(data.ritualdata.ReadRitualData(romfile) == false)
    {
        std::puts("Failed to read ritual data from ROM\n");
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
            case 'q': // quit
                exitflag = true;
                break;
            case 'b': // booster
                ViewBooster(data);
                break;
            case 'd': // deck
                ViewDeck(data);
                break;
            case 'n': // search by name
                SearchByCardName(data);
                break;
            case 'i': // search by id
                SearchByCardID(data);
                break;
            case 'a': // add an id to the database
                AddIDToDatabase(data);
                break;
            case 'r': // remove an id from the database
                RemoveDatabaseID(data);
                break;
            case 's': // view ritual 'S'ummons
                ViewRitualSummons(data);
                break;
            case 'f': // view fusion summons
                ViewFusionSummons(data);
                break;
            case 'j': // view junk cards
                ViewJunkCards(data);
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
