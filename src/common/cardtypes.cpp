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
#include "cardtypes.h"

const char *const WCTConstants::AttributeNames[size_t(Attribute::NUMATTRIBUTES)]
{
    "Nothing",
    "Light",
    "Dark",
    "Water",
    "Fire",
    "Earth",
    "Wind"
};

const char *const WCTConstants::CardTypeNames[size_t(CardType::NUMCARDTYPES)]
{
    "Nothing",
    "Dragon",
    "Zombie",
    "Fiend",
    "Pyro",
    "Sea Serpent",
    "Rock",
    "Machine",
    "Fish",
    "Dinosaur",
    "Insect",
    "Beast",
    "Beast-Warrior",
    "Plant",
    "Aqua",
    "Warrior",
    "Winged Beast",
    "Fairy",
    "Spellcaster",
    "Thunder",
    "Reptile",
    "Trap",
    "Spell",
    "Ticket",
    "Divine-Beast"
};

const char *const WCTConstants::SpellTrapTypeNames[size_t(SpellTrapType::NUMSTTYPES)]
{
    "Normal",
    "Counter",
    "Field",
    "Equip",
    "Continuous",
    "Quick",
    "Ritual"
};

const char *const WCTConstants::MonsterCardTypeNames[size_t(MonsterCardType::NUMMONCARDTYPES)]
{
    "Normal",
    "Effect",
    "Fusion",
    "Ritual"
};

// EOF
