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

#include <limits>
#include <type_traits>
#include <unordered_map>

#include "elib/elib.h"
#include "elib/qstring.h"

#include "json/json.h"

#include "../common/romoffsets.h"
#include "../common/jsonutils.h"
#include "patchtypes.h"

//
// Test if valid ROM offset (0 being not allowed as we don't patch that 
// location; this allows easy error checking)
//
static bool IsValidOffset(uint32_t offset)
{
    return offset > 0 && offset < WCTConstants::EXPECTED_ROM_SIZE;
}

//=============================================================================
//
// WCTSimplePatch
// A patch that simply replaces data of size 1, 2, or 4 bytes at a given offset.
//
//=============================================================================

//
// Create a new simple patch
//
WCTROMPatch *WCTSimplePatch::New(const Json::Value &jv)
{
    // required fields
    if(WCTJSONUtils::HasFields(jv, { "offset", "size", "value" }) == false)
        return nullptr;

    const uint32_t offset = WCTJSONUtils::ValueToUint(jv["offset"]).value_or(0u); // where to write
    const uint32_t size   = WCTJSONUtils::ValueToUint(jv["size"  ]).value_or(0u); // size of write (1, 2, or 4 bytes)
    const uint32_t value  = WCTJSONUtils::ValueToUint(jv["value" ]).value_or(0u); // value to write

    // validation
    if(IsValidOffset(offset) == false)
        return nullptr;
    if(size != 1 && size != 2 && size != 4)
        return nullptr;

    return new WCTSimplePatch { offset, size, value };
}

bool WCTSimplePatch::Apply() const
{
    return false;
}

//=============================================================================
//
// WCTStringMassReplacePatch
// A patch that searches in a string area of the ROM and replaces all instances
// of a given substring.
//
//=============================================================================

//
// Create a new string mass-replace patch
//
WCTROMPatch *WCTStringMassReplacePatch::New(const Json::Value &jv)
{
    // required fields
    if(WCTJSONUtils::HasFields(jv, { "term", "replace", "start", "end" }) == false)
        return nullptr;

    const std::string term    = jv["term"   ].asString(); // term to find
    const std::string replace = jv["replace"].asString(); // term to replace with
    const uint32_t    start   = WCTJSONUtils::ValueToUint(jv["start"]).value_or(0u); // start of region in which to do replacement
    const uint32_t    end     = WCTJSONUtils::ValueToUint(jv["end"  ]).value_or(0u); // end of region in which to do replacement

    // validation
    if(replace.length() > term.length()) // too long to fit there?
        return nullptr;
    if(IsValidOffset(start) == false || IsValidOffset(end) == false || end <= start) // invalid offsets?
        return nullptr;

    return new WCTStringMassReplacePatch { term.c_str(), replace.c_str(), start, end };
}

bool WCTStringMassReplacePatch::Apply() const
{
    return false;
}

//=============================================================================
//
// WCTSingleStringPatch
// A patch for a single string - the length must be within existing tolerance
//
//=============================================================================

//
// Create a new single string patch
//
WCTROMPatch *WCTSingleStringPatch::New(const Json::Value &jv)
{
    // required fields
    if(WCTJSONUtils::HasFields(jv, { "offset", "value" }) == false)
        return nullptr;

    const uint32_t    offset = WCTJSONUtils::ValueToUint(jv["offset"]).value_or(0u); // offset to write at
    const std::string value  = jv["value"].asString();                               // value to write there

    bool     allowLonger = false; // if true, value can be longer than what it is replacing
    uint32_t howmuch     = 0;     // if longer replacement is allowed, this is how much tolerance exists
    if(jv.isMember("allowLonger") == true)
    {
        if(jv.isMember("howmuch") == false) // must have "by how much" value also
            return nullptr;

        allowLonger = WCTJSONUtils::ValueToBool(jv["allowLonger"]).value_or(false);
        howmuch     = WCTJSONUtils::ValueToUint(jv["howmuch"    ]).value_or(0u);
    }

    // validation
    if(IsValidOffset(offset) == false)
        return nullptr;

    return new WCTSingleStringPatch { offset, value.c_str(), allowLonger, howmuch };
}

bool WCTSingleStringPatch::Apply() const
{
    return false;
}

//=============================================================================
//
// WCTCardPatch
// Card patch - contains all the information necessary to change one card 
// definition into another.
//
//=============================================================================

static constexpr uint32_t MAX_CARDPATCH_NUM = 1138u;
static constexpr uint32_t MAX_VALID_STAT    = ((1 << WCTConstants::SHIFT_ATTACK) - 1) * 10u; // highest value for ATK or DEF

//
// Test if valid enum value; max is non-inclusive
//
template<typename T, typename E>
static inline constexpr bool IsValidEnumValue(T input, E min, E max)
{
    using ul_t = typename std::underlying_type<E>::type;
    return (input >= ul_t(min) && input < ul_t(max));
}

//
// Test if valid monster card type
//
static inline constexpr bool IsValidMonsterType(WCTCardPatch::CardType ct)
{
    using ul_t = std::underlying_type<WCTCardPatch::CardType>::type;
    return (ul_t(ct) >= ul_t(WCTCardPatch::CardType::Dragon) && ul_t(ct) <= ul_t(WCTCardPatch::CardType::Reptile));
}

//
// Create a new card patch
//
WCTROMPatch *WCTCardPatch::New(const Json::Value &jv)
{
    // required fields
    if(WCTJSONUtils::HasFields(jv, { "num", "id" }) == false)
        return nullptr;

    std::unique_ptr<WCTCardPatch> upCard { new WCTCardPatch {} };
    WCTCardPatch &theCard = *upCard.get();

    // number
    const std::optional<uint32_t> optNum = WCTJSONUtils::ValueToUint(jv["num"]);
    if(optNum.has_value() && optNum.value() <= MAX_CARDPATCH_NUM)
        theCard.SetNum(optNum.value());
    else
        return nullptr; // invalid index

    // ID
    const std::optional<uint32_t> optID = WCTJSONUtils::ValueToUint(jv["id"]);
    if(optID.has_value() && optID.value() <= std::numeric_limits<uint16_t>::max())
        theCard.SetID(uint16_t(optID.value()));
    else
        return nullptr; // invalid ID

    // name
    if(jv.isMember("name"))
        theCard.SetName(jv["name"].asString().c_str());
    // text
    if(jv.isMember("text"))
        theCard.SetText(jv["text"].asString().c_str());
    // pix
    if(jv.isMember("pix"))
        theCard.SetPix(jv["pix"].asString().c_str());
    // pal
    if(jv.isMember("pal"))
        theCard.SetPal(jv["pal"].asString().c_str());

    // attribute
    if(jv.isMember("attribute"))
    {
        const std::optional<uint32_t> attr = WCTJSONUtils::ValueToUint(jv["attribute"]);
        if(attr.has_value() && IsValidEnumValue(attr.value(), Attribute::Nothing, Attribute::NUMATTRIBUTES))
            theCard.SetAttr(static_cast<Attribute>(attr.value()));
        else
            return nullptr; // invalid attribute
    }

    // level
    if(jv.isMember("level"))
    {
        const std::optional<uint32_t> lvl = WCTJSONUtils::ValueToUint(jv["level"]);
        if(lvl.has_value() && lvl.value() <= 15)
            theCard.SetLevel(uint8_t(lvl.value()));
        else
            return nullptr; // invalid level
    }

    // card type
    if(jv.isMember("cardtype"))
    {
        const std::optional<uint32_t> ct = WCTJSONUtils::ValueToUint(jv["cardtype"]);
        if(ct.has_value() && IsValidEnumValue(ct.value(), CardType::Nothing, CardType::NUMCARDTYPES))
            theCard.SetCardType(static_cast<CardType>(ct.value()));
        else
            return nullptr; // invalid card type
    }

    // spell-trap subtype
    if(jv.isMember("spelltraptype"))
    {
        if(theCard.GetCardType() != CardType::Spell && theCard.GetCardType() != CardType::Trap)
            return nullptr; // there should not be a spell-trap subtype for a non-spell-or-trap card
        const std::optional<uint32_t> stt = WCTJSONUtils::ValueToUint(jv["spelltraptype"]);
        if(stt.has_value() && IsValidEnumValue(stt.value(), SpellTrapType::Normal, SpellTrapType::NUMSTTYPES))
            theCard.SetSTType(static_cast<SpellTrapType>(stt.value()));
        else
            return nullptr; // invalid spell-trap subtype
    }

    // monster card subtype
    if(jv.isMember("monstertype"))
    {
        if(IsValidMonsterType(theCard.GetCardType()) == false)
            return nullptr; // must be a valid monster card type
        const std::optional<uint32_t> mct = WCTJSONUtils::ValueToUint(jv["monstertype"]);
        if(mct.has_value() && IsValidEnumValue(mct.value(), MonsterCardType::Normal, MonsterCardType::NUMMONCARDTYPES))
            theCard.SetMCType(static_cast<MonsterCardType>(mct.value()));
        else
            return nullptr; // invalid monster card subtype
    }

    // attack
    if(jv.isMember("atk"))
    {
        if(IsValidMonsterType(theCard.GetCardType()) == false)
            return nullptr; // only monsters have attack
        const std::optional<uint32_t> atk = WCTJSONUtils::ValueToUint(jv["atk"]);
        if(atk.has_value() && atk.value() <= MAX_VALID_STAT)
            theCard.SetATK(uint16_t(atk.value() / 10u));
        else
            return nullptr; // invalid ATK value
    }

    // defense
    if(jv.isMember("def"))
    {
        if(IsValidMonsterType(theCard.GetCardType()) == false)
            return nullptr; // only monsters have defense
        const std::optional<uint32_t> def = WCTJSONUtils::ValueToUint(jv["def"]);
        if(def.has_value() && def.value() <= MAX_VALID_STAT)
            theCard.SetDEF(uint16_t(def.value() / 10u));
        else
            return nullptr; // invalid DEF value
    }

    // done!
    return upCard.release();
}

bool WCTCardPatch::Apply() const
{
    return false;
}

//=============================================================================
//
// WCTPatchFactory
// Instantiate ROM patches from JSON objects which describe them
//
//=============================================================================

namespace WCTPatchFactory 
{
    using instantiatorfn_t = WCTROMPatch *(*)(const Json::Value &);
    
    struct CCHash
    {
        size_t operator () (const char *const &str) const noexcept { return qstring::HashCodeCaseStatic(str); }
    };

    struct CCCompare
    {
        bool operator () (const char *const &lhs, const char *const &rhs) const noexcept { return strcmp(lhs, rhs) == 0; }
    };

    using typemap_t = std::unordered_map<const char *, instantiatorfn_t, CCHash, CCCompare>;

    static const typemap_t s_typemap
    {
        { "WCTSimplePatch"           , &WCTSimplePatch::New            },
        { "WCTStringMassReplacePatch", &WCTStringMassReplacePatch::New },
        { "WCTSingleStringPatch"     , &WCTSingleStringPatch::New      },
        { "WCTCardPatch"             , &WCTCardPatch::New              }
    };

    //
    // Given a JSON object, instantiate the type of patch object it describes. Returns
    // null if there is any problem doing so. Can also potentially throw JsonCpp exceptions
    // for exceptionally malformed fields (feeding arrays/objects to simple fields; you're
    // expected to catch them).
    //
    WCTROMPatch *New(const Json::Value &jv)
    {
        if(jv.isMember("type") == false)
            return nullptr;

        const auto &citr = s_typemap.find(jv["type"].asString().c_str());
        if(citr == s_typemap.cend())
            return nullptr;

        return citr->second(jv);
    }
}

// EOF
