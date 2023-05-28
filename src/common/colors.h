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

namespace WCTColor
{
    using gbacolor_t = uint16_t;

    // Get red component of RGB555
    static inline constexpr uint8_t R5(gbacolor_t color)
    {
        return uint8_t(color & 0x1F);
    }

    // Get green component of RGB555
    static inline constexpr uint8_t G5(gbacolor_t color)
    {
        return uint8_t((color >> 5) & 0x1F);
    }

    // Get blue component of RGB555
    static inline constexpr uint8_t B5(gbacolor_t color)
    {
        return uint8_t((color >> 10) & 0x1F);
    }

    static inline constexpr uint8_t Expand5To8(uint8_t component)
    {
        return ((component << 3) | ((component >> 2) & 7));
    }

    // Convert 16-bit RGB555 to 24-bit RGB8
    static inline constexpr uint32_t RGB555ToRGB8(gbacolor_t color)
    {
        return ((uint32_t(R5(color)) << 19) | (uint32_t(G5(color)) << 11) | (uint32_t(B5(color)) << 3));
    }

    // Color-correct RGB555-to-RGB8 output for display purposes
    static inline constexpr uint32_t Resaturate(uint32_t color)
    {
        return (color | ((color >> 5) & 0x070707u));
    }

    // Convert 24-bit RGB8 to 16-bit RGB555
    static inline constexpr gbacolor_t RGB8ToRGB555(uint32_t color)
    {
        return gbacolor_t(((color & 0xF8) >> 3) | ((color & 0xF800) >> 6) | ((color & 0xF80000) >> 9));
    }

    // Convert 8-bit separate R/G/B to RGB555
    static inline constexpr gbacolor_t RGBToRGB555(uint8_t r, uint8_t g, uint8_t b)
    {
        return ((r >> 3) | (gbacolor_t(g >> 3) << 5) | (gbacolor_t(b >> 3) << 10));
    }
};

// EOF
