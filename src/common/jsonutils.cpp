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
#include "elib/misc.h"
#include "elib/qstring.h"

#include "json/json.h"

#include "jsonutils.h"

//
// Load the file into a string
//
std::optional<qstring> WCTJSONUtils::StringFromFile(const char *filename)
{
    std::optional<qstring> ret;
    
    if(hal_platform.fileExists(filename) == false)
    {
        // if the file doesn't exist yet, that's not an error; we'll write it anew
        ret = "";
    }
    else if(const EAutoFile upFile { std::fopen(filename, "rb") }; upFile != nullptr)
    {
        const size_t len = static_cast<size_t>(M_FileLength(upFile.get()));
        if(len != 0) 
        {
            // clear-allocate at length+1 for null termination
            const EUniquePtr<char> upBuf { ecalloc(char, 1, len + 1) };
            if(std::fread(upBuf.get(), 1, len, upFile.get()) == len)
                ret = upBuf.get();
        }
        else
            ret = ""; // if file is empty, this is not an error; empty database.
    }
    // else, fopen failure

    return ret;
}

//
// Parse the JSON string
//
bool WCTJSONUtils::ParseJsonFromString(const qstring &str, Json::Value &root, JSONCPP_STRING &errs)
{
    const char *const begin = str.c_str();
    const char *const end   = begin + str.length();

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> upReader { builder.newCharReader() };
    return upReader->parse(begin, end, &root, &errs);
}

//
// Convert a JSON value to uint; allows interpretation of strings as numbers
// via strtoul, which provides hexadecimal support.
//
std::optional<uint32_t> WCTJSONUtils::ValueToUint(const Json::Value &value)
{
    if(value.isConvertibleTo(Json::ValueType::uintValue))
        return value.asUInt();
    else if(value.isConvertibleTo(Json::ValueType::stringValue))
        return std::strtoul(value.asString().c_str(), nullptr, 0);
    else
        return std::nullopt;
}

//
// Convert a JSON value to bool while never throwing
//
std::optional<bool> WCTJSONUtils::ValueToBool(const Json::Value &value)
{
    if(value.isConvertibleTo(Json::ValueType::booleanValue))
        return value.asBool();
    else
        return std::nullopt;
}

// EOF
