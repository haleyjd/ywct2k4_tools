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
}

// EOF
