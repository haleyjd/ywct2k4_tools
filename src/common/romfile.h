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
#include <array>
#include <vector>

namespace WCTROMFile
{
    bool VerifyROM(FILE *f);

    template<typename T>
    std::optional<T> GetData(FILE *f)
    {
        std::optional<T> ret {};
        T value;
        if(std::fread(&value, sizeof(T), 1, f) == 1)
            ret = value;
        return ret;
    }

    template<typename T>
    std::optional<T> GetDataFromOffset(FILE *f, uint32_t offs)
    {
        std::optional<T> ret {};
        T value;
        if(std::fseek(f, long(offs), SEEK_SET) == 0 &&
           std::fread(&value, sizeof(T), 1, f) == 1)
        {
            ret = value;
        }
        return ret;            
    }

    template<typename T>
    inline bool GetArray(FILE *f, T *buf, size_t numelems)
    {
        return (std::fread(buf, sizeof(T), numelems, f) == numelems);
    }

    template<typename T>
    inline bool GetArrayFromOffset(FILE *f, uint32_t offset, T *buf, size_t numelems)
    {
        return 
            (std::fseek(f, long(offset), SEEK_SET) == 0) 
            && GetArray<T>(f, buf, numelems);
    }
    
    template<typename T, size_t N>
    inline bool GetCArray(FILE *f, T (&buf)[N])
    {
        return (std::fread(buf, sizeof(T), N, f) == N);
    }

    template<typename T, size_t N>
    inline bool GetCArrayFromOffset(FILE *f, uint32_t offset, T (&buf)[N])
    {
        return 
            (std::fseek(f, long(offset), SEEK_SET) == 0)
            && GetCArray<T>(f, buf);
    }

    template<typename T>
    inline bool GetVector(FILE *f, std::vector<T> &vec)
    {
        return GetArray<T>(f, vec.data(), vec.size());
    }

    template<typename T>
    inline bool GetVectorFromOffset(FILE *f, uint32_t offset, std::vector<T> &vec)
    {
        return GetArrayFromOffset<T>(f, offset, vec.data(), vec.size());
    }

    template<typename T, size_t S>
    inline bool GetStdArray(FILE *f, std::array<T, S> &arr)
    {
        return GetArray<T>(f, arr.data(), S);
    }

    template<typename T, size_t S>
    inline bool GetStdArrayFromOffset(FILE *f, uint32_t offset, std::array<T, S> &arr)
    {
        return GetArrayFromOffset<T>(f, offset, arr.data(), S);
    }

} // end namespace WCTROMFile

// EOF
