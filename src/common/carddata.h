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

#include <vector>
#include "cardtypes.h"

namespace WCTConstants
{
    static constexpr uint32_t MASK_ATTRIBUTE      = 0xE0000000u; // xxx00000000000000000000000000000
    static constexpr uint32_t MASK_LEVEL          = 0x1E000000u; // 000xxxx0000000000000000000000000
    static constexpr uint32_t MASK_CARD_TYPE      = 0x01F00000u; // 0000000xxxxx00000000000000000000
    static constexpr uint32_t MASK_SPELLTRAP_TYPE = 0x000E0000u; // 000000000000xxx00000000000000000
    static constexpr uint32_t MASK_MONSTER_TYPE   = 0x000C0000u; // 000000000000xx000000000000000000
    static constexpr uint32_t MASK_ATTACK         = 0x0003FE00u; // 00000000000000xxxxxxxxx000000000
    static constexpr uint32_t MASK_DEFENSE        = 0x000001FFu; // 00000000000000000000000xxxxxxxxx

    static constexpr uint32_t SHIFT_ATTRIBUTE      = 29u;
    static constexpr uint32_t SHIFT_LEVEL          = 25u;
    static constexpr uint32_t SHIFT_CARD_TYPE      = 20u;
    static constexpr uint32_t SHIFT_SPELLTRAP_TYPE = 17u;
    static constexpr uint32_t SHIFT_MONSTER_TYPE   = 18u;
    static constexpr uint32_t SHIFT_ATTACK         =  9u;

    // Extract attribute from card data value
    static inline constexpr Attribute GetCardAttribute(uint32_t data)
    {
        return static_cast<Attribute>((data & MASK_ATTRIBUTE) >> SHIFT_ATTRIBUTE);
    }

    // Extract monster level from card data value
    static inline constexpr uint32_t GetCardLevel(uint32_t carddata)
    {
        return ((carddata & MASK_LEVEL) >> SHIFT_LEVEL);
    }

    // Extract card type from card data value
    static inline constexpr CardType GetCardType(uint32_t carddata)
    {
        return static_cast<CardType>((carddata & MASK_CARD_TYPE) >> SHIFT_CARD_TYPE);
    }

    // Extract spell/trap subtype from card data value
    static inline constexpr SpellTrapType GetSpellTrapType(uint32_t carddata)
    {
        return static_cast<SpellTrapType>((carddata & MASK_SPELLTRAP_TYPE) >> SHIFT_SPELLTRAP_TYPE);
    }

    // Extract monster card subtype from card data value
    static inline constexpr MonsterCardType GetMonsterType(uint32_t carddata)
    {
        return static_cast<MonsterCardType>((carddata & MASK_MONSTER_TYPE) >> SHIFT_MONSTER_TYPE);
    }

    // Extract attack for monster from card data value
    static inline constexpr uint32_t GetMonsterATK(uint32_t carddata)
    {
        return ((carddata & MASK_ATTACK) >> SHIFT_ATTACK) * 10;
    }
    
    // Extract defense for monster from card data value
    static inline constexpr uint32_t GetMonsterDEF(uint32_t data)
    {
        return (data & MASK_DEFENSE) * 10;
    }

    static constexpr uint32_t MASK_RITUAL_MONSTER = 0x00001FFFu; // 0000000000000000000xxxxxxxxxxxxx
    static constexpr uint32_t MASK_RITUAL_SPELL   = 0x03FFE000u; // 000000xxxxxxxxxxxxx0000000000000
    static constexpr uint32_t MASK_RITUAL_LEVELS  = 0xFC000000u; // xxxxxx00000000000000000000000000

    static constexpr uint32_t SHIFT_RITUAL_SPELL  = 13u;
    static constexpr uint32_t SHIFT_RITUAL_LEVELS = 26u;

    // Extract target monster for ritual from ritual data value
    static inline constexpr uint32_t GetRitualMonster(uint32_t ritualdata)
    {
        return (ritualdata & MASK_RITUAL_MONSTER);
    }

    // Extract required spell card for ritual from ritual data value
    static inline constexpr uint32_t GetRitualSpell(uint32_t ritualdata)
    {
        return ((ritualdata & MASK_RITUAL_SPELL) >> SHIFT_RITUAL_SPELL);
    }

    // Extract required levels for ritual from ritual data value
    static inline constexpr uint32_t GetRitualLevels(uint32_t ritualdata)
    {
        return ((ritualdata & MASK_RITUAL_LEVELS) >> SHIFT_RITUAL_LEVELS);
    }
}

//
// Stores packed information on the cards supported by the game.
//
class WCTCardData final
{
public:
    using carddata_t = std::vector<uint32_t>;

    // Read card data from the ROM file
    bool ReadCardData(FILE *f);

    const carddata_t &GetData() const { return m_carddata; }

    uint32_t DataForCardNum(size_t num) const
    {
        return num < m_carddata.size() ? m_carddata[num] : 0;
    }

private:
    carddata_t m_carddata;
};

//
// Stores packed information on ritual summons supported by the game.
//
class WCTRitualData final
{
public:
    using ritualdata_t = std::vector<uint32_t>;

    // Read ritual data from the ROM file
    bool ReadRitualData(FILE *f);

    const ritualdata_t &GetData() const { return m_ritualdata; }

private:
    ritualdata_t m_ritualdata;
};

//
// Stores information on supported fusion summons. Note that it isn't possible
// to add new entries, because there's a huge function in the game that just ignores
// these tables and reimplements all the same data using hardcoded comparisons.
// Only certain aspects of fusion summons thus obey these tables and they're only 
// useful for informative purposes as a result.
// Absolutely repugnant coding practices going on in this game.
//
class WCTFusionData final
{
public:
    using cardid_t = uint16_t;
    struct fusionentry_t
    {
        cardid_t fusion_id    = 0;
        cardid_t material1_id = 0;
        cardid_t material2_id = 0;
        cardid_t material3_id = 0;
    };

    using fusiontable_t = std::vector<fusionentry_t>;

    // Read the fusion summon tables from the ROM
    bool ReadFusionTables(FILE *f);

    const fusiontable_t &GetFusion2Mats() const { return m_fusion2mats; }
    const fusiontable_t &GetFusion3Mats() const { return m_fusion3mats; }

private:
    fusiontable_t m_fusion2mats;
    fusiontable_t m_fusion3mats;

    bool ReadFusionTable(FILE *f, uint32_t offset, fusiontable_t &table);
};

// EOF
