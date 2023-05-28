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
#include "iddb.h"

#include "json/json.h"

#include "jsonutils.h"

//
// Load the database from file
//
bool WCTIDDatabase::LoadFromFile(const char *filename)
{
    const std::optional<qstring> input { WCTJSONUtils::StringFromFile(filename) };
    if(input.has_value() == false)
        return false; // failed to get input for a bad reason

    if(input.value().empty() == true)
        return true; // string is empty (but legitimately so), so no need to parse

    bool success = false;
    try
    {
        Json::Value root;
        JSONCPP_STRING errs;

        if(WCTJSONUtils::ParseJsonFromString(input.value(), root, errs) == true)
        {
            if(root.isObject())
            {
                const Json::Value::Members keys { root.getMemberNames() };
                for(const std::string &key : keys)
                {
                    const Json::Value &value = root[key];
                    if(value.isConvertibleTo(Json::ValueType::stringValue))
                    {
                        const cardid_t id = uint16_t(std::strtoul(key.c_str(), nullptr, 16));
                        SetMapping(id, value.asString().c_str());
                    }
                }
                success = true;
            }
        }
        else
        {
            m_errors = errs.c_str();
        }
    }
    catch(const std::exception &ex)
    {
        m_errors = ex.what();
    }

    // remember if loaded successfully or not
    m_loadfailed = !success;

    return success;
}

//
// Save the database to file
//
bool WCTIDDatabase::SaveToFile(const char *filename) const
{
    // if we hard-failed loading it, we do NOT re-write it.
    if(m_loadfailed == true)
        return false;

    // don't bother writing it if it's empty
    if(m_idmap.empty() == true)
        return true;

    bool success = false;
    try
    {
        Json::Value root;
        qstring id;
        for(const auto &pair : m_idmap)
        {
            id = qstring::ToString(int(pair.first), 16);
            root[id.c_str()] = pair.second.c_str();
        }
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        const std::string strout = Json::writeString(builder, root);

        const qstring tmpfn = qstring(filename) + ".tmp";
        if(M_WriteFile(tmpfn.c_str(), strout.c_str(), strout.length()) == 1)
        {
            std::remove(filename);
            if(std::rename(tmpfn.c_str(), filename) == 0)
                success = true;
        }
    }
    catch(...)
    {
    }

    return success;
}

// EOF
