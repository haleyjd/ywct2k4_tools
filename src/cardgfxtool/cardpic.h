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

#include <array>
#include "../common/colors.h"
#include "../common/romoffsets.h"

class WCTCardPic final
{
public:
    using palette_t = std::array<WCTColor::gbacolor_t, WCTConstants::CARDPALETTE_NUMENTRIES>;

    using pixel_t = uint8_t;
    using rawdata_t = std::array<pixel_t, WCTConstants::CARDGFX_READ_SIZEOF>;
    using pixels_t  = std::array<pixel_t, WCTConstants::CARDGFX_PIXEL_COUNT>;

    // Read in a card picture from the ROM file
    bool ReadCardPic(FILE *f, uint16_t cardnum);

    // Write the card graphic out as a PNG
    bool WriteToPNG(const char *filename) const;

    // Read in a PNG file
    bool ReadFromPNG(const char *filename);

    // Write raw GBA data to a pair of files (.pix and .pal)
    bool WriteGBAData(const char *basefilename) const;

private:
    rawdata_t m_rawdata;
    palette_t m_palette;
    pixels_t  m_pixels;

    void UnpackPixels();
    void PackPixels();

    bool WritePixels(const char *basefilename) const;
    bool WritePalette(const char *basefilename) const;
};

// EOF
