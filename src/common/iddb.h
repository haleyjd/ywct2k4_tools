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

#include <unordered_map>
#include <vector>
#include "elib/qstring.h"

//
// Allows saving associations between otherwise unknown IDs and the cards to which
// they belong, which are supported in the game's coding but not present in the
// game as it was released; this is for ease of looking them up again while
// reverse engineering the ROM.
//
class WCTIDDatabase
{
public:
    using cardid_t = uint16_t;
    using map_t = std::unordered_map<cardid_t, qstring>;

    static constexpr cardid_t INVALID_ID = 0;

    bool LoadFromFile(const char *filename);
    bool SaveToFile(const char *filename) const;

    // Test if there is a mapping for the given ID
    bool HasMappingForID(cardid_t id) const
    {
        return m_idmap.find(id) != m_idmap.cend();
    }

    // Get the name for a given ID, or empty string if it doesn't exist
    const qstring &GetNameForID(cardid_t id) const
    {
        const map_t::const_iterator citr = m_idmap.find(id);
        return (citr != m_idmap.cend()) ? citr->second : qstring::emptystr;
    }

    // Add a new mapping but only if such ID doesn't already exist in the map. Returns
    // true if the mapping was added and false otherwise.
    bool AddMappingIfNewID(cardid_t id, const char *name)
    {
        bool res = false;
        if(const map_t::const_iterator citr = m_idmap.find(id); citr == m_idmap.cend())
        {
            m_idmap.emplace(id, qstring { name });
            res = true;
        }
        return res;
    }
    // As with the above overload but moves in a qstring.
    bool AddMappingIfNewID(cardid_t id, qstring &&name)
    {
        bool res = false;
        if(const map_t::const_iterator citr = m_idmap.find(id); citr == m_idmap.cend())
        {
            m_idmap.emplace(id, std::move(name));
            res = true;
        }
        return res;
    }

    // Set a mapping unconditionally; any existing one will be destroyed
    void SetMapping(cardid_t id, const char *name)
    {
        m_idmap[id] = name;
    }
    // Set a mapping unconditionally; any existing one will be destroyed; move version.
    void SetMapping(cardid_t id, qstring &&str)
    {
        m_idmap[id] = std::move(str);
    }

    // Find an ID that has the given card name attached. Not efficient but this
    // is adequate for its purpose in these command-line tools. Returns INVALID_ID
    // if not found.
    cardid_t ReverseLookup(const char *name, bool exact) const
    {
        for(const auto &pair : m_idmap)
        {
            if(exact)
            {
                if(pair.second.strCaseCmp(name) == 0)
                    return pair.first;
            }
            else
            {
                if(pair.second.containsNoCase(name) == true)
                    return pair.first;
            }
        }

        return INVALID_ID; // not found
    }

    // Remove an id mapping
    void RemoveMapping(uint16_t id)
    {
        m_idmap.erase(id);
    }

    // Get read-only view of id-to-names map
    const map_t &GetMap() const { return m_idmap; }

    // If LoadFromFile returns false, this may contain error information
    const qstring &GetErrors() const { return m_errors; }

    // Test if in an error state
    bool HasError() const { return m_loadfailed; }

private:
    map_t   m_idmap;
    bool    m_loadfailed = false;
    qstring m_errors;
};

// EOF
