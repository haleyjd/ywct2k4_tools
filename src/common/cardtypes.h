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
    enum class Attribute : uint8_t
    {
        Nothing,
        Light,
        Dark,
        Water,
        Fire,
        Earth,
        Wind,
        NUMATTRIBUTES
    };

    extern const char *const AttributeNames[size_t(Attribute::NUMATTRIBUTES)];
    static inline const char *SafeAttributeName(Attribute attr)
    {
        const uint8_t uattr = uint8_t(attr);
        return (uattr < uint8_t(Attribute::NUMATTRIBUTES)) ? AttributeNames[uattr] : "";
    }

    // The game distinguishes spells & traps from monsters by checking the
    // card type value for 21 or 22.
    enum class CardType : uint8_t
    {
        Nothing,
        Dragon,
        Zombie,
        Fiend,
        Pyro,
        SeaSerpent,
        Rock,
        Machine,
        Fish,
        Dinosaur,
        Insect,
        Beast,
        BeastWarrior,
        Plant,
        Aqua,
        Warrior,
        WingedBeast,
        Fairy,
        Spellcaster,
        Thunder,
        Reptile,
        Trap,
        Spell,
        Ticket,      // unused; left over all the way from DM5 Expert 1 lol
        DivineBeast, // also unused, and likewise.
        NUMCARDTYPES
    };

    extern const char *const CardTypeNames[size_t(CardType::NUMCARDTYPES)];
    static inline const char *SafeCardTypeName(CardType ct)
    {
        const uint8_t uct = uint8_t(ct);
        return (uct < uint8_t(CardType::NUMCARDTYPES)) ? CardTypeNames[uct] : "";
    }

    enum class SpellTrapType : uint8_t
    {
        Normal,
        Counter,
        Field,
        Equip,
        Continuous,
        Quick,
        Ritual,
        NUMSTTYPES
    };

    extern const char *const SpellTrapTypeNames[size_t(SpellTrapType::NUMSTTYPES)];
    static inline const char *SafeSpellTrapTypeName(SpellTrapType stt)
    {
        const uint8_t ustt = uint8_t(stt);
        return (ustt < uint8_t(SpellTrapType::NUMSTTYPES)) ? SpellTrapTypeNames[ustt] : "";
    }

    // This value is found inside monster cards only. It is not always correct 
    // and the game contains a load of hardcoded crap that compensates for the
    // fact. Note that Spirit, Toon, and Union don't even exist here - each has
    // its own routine that checks for every single card ID in that class...
    enum class MonsterCardType : uint8_t
    {
        Normal,
        Effect,
        Fusion,
        Ritual,
        NUMMONCARDTYPES
    };

    extern const char *const MonsterCardTypeNames[size_t(MonsterCardType::NUMMONCARDTYPES)];
    static inline const char *SafeMonsterCardTypeName(MonsterCardType mct)
    {
        const uint8_t umct = uint8_t(mct);
        return (umct < uint8_t(MonsterCardType::NUMMONCARDTYPES)) ? MonsterCardTypeNames[umct] : "";
    }

}

// EOF
