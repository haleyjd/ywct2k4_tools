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
}

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

// EOF
