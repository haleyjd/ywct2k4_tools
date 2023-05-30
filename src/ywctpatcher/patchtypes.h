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

#include "../common/carddata.h"

//
// Virtual base class for all ROM patches
//
class WCTROMPatch
{
public:
    virtual ~WCTROMPatch() {}

    // Called by the patch script execution process to apply the patch to the ROM
    virtual bool Apply() const = 0;
};

//
// A patch that simply replaces data of size 1, 2, or 4 bytes at a given offset.
//
class WCTSimplePatch : public WCTROMPatch
{
public:
    WCTSimplePatch(uint32_t offset, uint32_t size, uint32_t value)
        : m_offset(offset), m_size(size), m_value(value)
    {
    }

    static WCTROMPatch *New(const Json::Value &jv);

    virtual bool Apply() const override;

protected:
    uint32_t m_offset; // where to write
    uint32_t m_size;   // size of write (1, 2, or 4 bytes)
    uint32_t m_value;  // value to write
};

//
// A patch that searches in a string area of the ROM and replaces all instances
// of a given substring.
//
class WCTStringMassReplacePatch : public WCTROMPatch
{
public:
    WCTStringMassReplacePatch(const char *term, const char *replace, uint32_t start, uint32_t end) 
        : m_term(term) , m_replace(replace), m_start(start), m_end(end)
    {
    }

    static WCTROMPatch *New(const Json::Value &jv);

    virtual bool Apply() const override;

protected:
    qstring  m_term;    // term to find
    qstring  m_replace; // term to replace with
    uint32_t m_start;   // start of region in which to do replacement
    uint32_t m_end;     // end of region in which to do replacement
};

//
// A patch for a single string - the length must be within existing tolerance
//
class WCTSingleStringPatch : public WCTROMPatch
{
public:
    WCTSingleStringPatch(uint32_t offset, const char *value, bool allowLonger, uint32_t howmuch)
        : m_offset(offset), m_value(value), m_allowLonger(allowLonger), m_howmuch(howmuch)
    {
    }

    static WCTROMPatch *New(const Json::Value &jv);

    virtual bool Apply() const override;

protected:
    uint32_t m_offset;      // offset to write at
    qstring  m_value;       // value to write there
    bool     m_allowLonger; // if true, value can be longer than what it is replacing
    uint32_t m_howmuch;     // if longer replacement is allowed, this is how much tolerance exists (usually one or two bytes at most)
};

//
// Card patch - contains all the information necessary to change one card definition into
// another.
//
class WCTCardPatch : public WCTROMPatch
{
public:
    using Attribute       = WCTConstants::Attribute;
    using CardType        = WCTConstants::CardType;
    using SpellTrapType   = WCTConstants::SpellTrapType;
    using MonsterCardType = WCTConstants::MonsterCardType;

    static WCTROMPatch *New(const Json::Value &jv);

    virtual bool Apply() const override;

    euint            GetNum()      const { return m_num;      }
    uint16_t         GetID()       const { return m_id;       }
    const qstring   &GetName()     const { return m_name;     }
    const qstring   &GetText()     const { return m_text;     }
    const qstring   &GetPix()      const { return m_pix;      }
    const qstring   &GetPal()      const { return m_pal;      }
    Attribute        GetAttr()     const { return m_attr;     }
    uint8_t          GetLevel()    const { return m_level;    }
    CardType         GetCardType() const { return m_cardType; }
    SpellTrapType    GetSTType()   const { return m_sttType;  }
    MonsterCardType  GetMCType()   const { return m_mcType;   }
    uint16_t         GetATK()      const { return m_atk;      }
    uint16_t         GetDEF()      const { return m_def;      }

    void SetNum     (euint           num ) { m_num      = num;  }
    void SetID      (uint16_t        id  ) { m_id       = id;   }
    void SetName    (const char     *name) { m_name     = name; }
    void SetText    (const char     *text) { m_text     = text; }
    void SetPix     (const char     *pix ) { m_pix      = pix;  }
    void SetPal     (const char     *pal ) { m_pal      = pal;  }
    void SetAttr    (Attribute       attr) { m_attr     = attr; }
    void SetLevel   (uint8_t         lvl ) { m_level    = lvl;  }
    void SetCardType(CardType        ct  ) { m_cardType = ct;   }
    void SetSTType  (SpellTrapType   stt ) { m_sttType  = stt;  }
    void SetMCType  (MonsterCardType mc  ) { m_mcType   = mc;   }
    void SetATK     (uint16_t        atk ) { m_atk      = atk;  }
    void SetDEF     (uint16_t        def ) { m_def      = def;  }

protected:
    euint           m_num      = 0;                       // card number
    uint16_t        m_id       = 0;                       // card ID
    qstring         m_name;                               // card name
    qstring         m_text;                               // card text
    qstring         m_pix;                                // path to .pix file created by cardgfxtool
    qstring         m_pal;                                // path to .pal file created by cardgfxtool
    Attribute       m_attr     = Attribute::Nothing;      // attribute
    uint8_t         m_level    = 0;                       // level
    CardType        m_cardType = CardType::Nothing;       // card type
    SpellTrapType   m_sttType  = SpellTrapType::Normal;   // spell or trap subtype, if card type == 21 or 22
    MonsterCardType m_mcType   = MonsterCardType::Normal; // monster card subtype, if card type < 21
    uint16_t        m_atk      = 0;                       // ATK
    uint16_t        m_def      = 0;                       // DEF
};

//
// Instantiate ROM patches from JSON objects which describe them
//
namespace WCTPatchFactory
{
    // Given a JSON object, instantiate the type of patch object it describes. Returns
    // null if there is any problem doing so. Can also potentially throw JsonCpp exceptions
    // for exceptionally malformed fields (feeding arrays/objects to simple fields; you're
    // expected to catch them).
    WCTROMPatch *New(const Json::Value &jv);
}

// EOF
