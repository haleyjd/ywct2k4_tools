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

    virtual bool Apply() const override;

protected:
    uint32_t m_offset;      // offset to write at
    qstring  m_value;       // value to write there
    bool     m_allowLonger; // if true, value can be longer than what it is replacing
    uint32_t m_howmuch;     // if longer replacement is allowed, this is how much tolerance exists (usually one or two bytes at most)
};

// EOF
