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

#include <optional>

namespace WCTJSONUtils
{
    // Load the file into a string
    std::optional<qstring> StringFromFile(const char *filename);

    // Parse the JSON string
    bool ParseJsonFromString(const qstring &str, Json::Value &root, JSONCPP_STRING &errs);

    // Convert a JSON value to uint; allows interpretation of strings as numbers
    // via strtoul, which provides hexadecimal support.
    std::optional<uint32_t> ValueToUint(const Json::Value &value);

    // Test if the object contains fields with all the given names
    template<size_t N>
    bool HasFields(const Json::Value &value, const char *const (&names)[N])
    {
        for(const char *const name : names)
        {
            if(value.isMember(name) == false)
                return false;
        }
        return true;
    }

    // Convert a JSON value to bool while never throwing
    std::optional<bool> ValueToBool(const Json::Value &value);
}

// EOF
