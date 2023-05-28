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

#include <algorithm>
#include "png.h"

#include "elib/elib.h"
#include "elib/misc.h"
#include "elib/qstring.h"
#include "cardpic.h"
#include "../common/colors.h"
#include "../common/romfile.h"

//
// Automatically releases libpng resources at scope exit
//
class WCTPNGAutoRelease final
{
public:
    enum class Mode
    {
        WRITE,
        READ
    };

    explicit WCTPNGAutoRelease(png_structp p, Mode mode = Mode::WRITE) : m_pngptr(p), m_mode(mode) {}

    ~WCTPNGAutoRelease()
    {
        if(m_mode == Mode::READ)
            png_destroy_read_struct(&m_pngptr, &m_infoptr, nullptr);
        else
            png_destroy_write_struct(&m_pngptr, &m_infoptr);
    }

    void SetInfoPtr(png_infop info) { m_infoptr = info; }

private:
    png_structp m_pngptr  = nullptr;
    png_infop   m_infoptr = nullptr;
    Mode        m_mode    = Mode::WRITE;
};

//
// Translate GBA color palette to PNG
//
static void TranslatePalette(png_colorp outcolors, const WCTCardPic::palette_t &incolors)
{
    // clear to zero
    std::memset(outcolors, 0, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));

    // translate entries from GBA palette RGB555 values
    for(WCTColor::gbacolor_t gbacol : incolors)
    {
        png_color &outcol = *outcolors;

        outcol.red   = WCTColor::Expand5To8(WCTColor::R5(gbacol));
        outcol.green = WCTColor::Expand5To8(WCTColor::G5(gbacol));
        outcol.blue  = WCTColor::Expand5To8(WCTColor::B5(gbacol));

        ++outcolors;
    }
}

//
// Unpack 6bpp tiled data into a linear image
//
void WCTCardPic::UnpackPixels()
{
    const uint16_t *src  = reinterpret_cast<uint16_t *>(m_rawdata.data());
    uint8_t *const  base = m_pixels.data();

    constexpr uint32_t tilepitch = (WCTConstants::CARDGFX_TILE_HEIGHT_PX * WCTConstants::CARDGFX_FULLWIDTH_PX);

    for(uint32_t ty = 0; ty < WCTConstants::CARDGFX_TILEMAP_HEIGHT; ty++)
    {
        for(uint32_t tx = 0; tx < WCTConstants::CARDGFX_TILEMAP_WIDTH; tx++)
        {
            uint8_t  *dst   = base + ty * tilepitch + tx * WCTConstants::CARDGFX_TILE_WIDTH_PX;
            uint32_t  count = WCTConstants::CARDGFX_TILE_HEIGHT_PX;
            do
            {
                const uint16_t data0 = *src++;
                const uint16_t data1 = *src++;
                const uint16_t data2 = *src++;

                *(dst + 0) = uint8_t(data0 & 63);                          // 0:----|------|xxxxxx
                *(dst + 1) = uint8_t((data0 & 0xFC0) >> 6);                // 0:----|xxxxxx|------
                *(dst + 2) = uint8_t((data0 >> 12) | ((data1 & 3) << 4));  // 0:xxxx|------|------  + 1:--|------|------|xx
                *(dst + 3) = uint8_t((data1 & 0xFC) >> 2);                 // 1:--|------|xxxxxx|--
                *(dst + 4) = uint8_t((data1 >> 8) & 63);                   // 1:--|xxxxxx|------|--
                *(dst + 5) = uint8_t((data1 >> 14) | ((data2 & 15) << 2)); // 1:xx|------|------|-- + 2:------|------|xxxx
                *(dst + 6) = uint8_t((data2 >> 4) & 63);                   // 2:------|xxxxxx|----
                *(dst + 7) = uint8_t((data2 & 0xFC00) >> 10);              // 2:xxxxxx|------|----

                dst += WCTConstants::CARDGFX_FULLWIDTH_PX;
            }
            while(--count != 0);
        }
    }

    // if we had tile memory like on GBA, you could just do this...
#if 0
    uint32_t count = WCTConstants::CARDGFX_READ_SIZEOF / (2 * 3);
    do
    {
        const uint16_t data0 = *src++;
        const uint16_t data1 = *src++;
        const uint16_t data2 = *src++;

        uint8_t *dst = base;

        *dst++ = uint8_t(data0 & 63);
        *dst++ = uint8_t((data0 & 0xFC0) >> 6);
        *dst++ = uint8_t((data0 >> 12) | ((data1 & 3) << 4));
        *dst++ = uint8_t((data1 & 0xFC) >> 2);
        *dst++ = uint8_t((data1 >> 8) & 63);
        *dst++ = uint8_t((data1 >> 14) | ((data2 & 15) << 2));
        *dst++ = uint8_t((data2 >> 4) & 63);
        *dst++ = uint8_t((data2 & 0xFC00) >> 10);
    }
    while(--count != 0);
#endif
}

//
// Read in a card picture from the ROM file
//
bool WCTCardPic::ReadCardPic(FILE *f, uint16_t cardnum)
{
    if(f == nullptr)
        return false;

    // get palette
    const uint32_t paletteoffset = WCTConstants::OFFS_CARDPALETTES_START + (cardnum * WCTConstants::CARDPALETTE_READ_SIZEOF);
    if(WCTROMFile::GetStdArrayFromOffset(f, paletteoffset, m_palette) == false)
        return false;

    // get raw graphics data
    const uint32_t gfxoffset = WCTConstants::OFFS_CARDGFX_START + (cardnum * WCTConstants::CARDGFX_READ_SIZEOF);
    if(WCTROMFile::GetStdArrayFromOffset(f, gfxoffset, m_rawdata) == false)
        return false;

    UnpackPixels();

    return true;
}

//
// Write the card graphic out as a PNG
//
bool WCTCardPic::WriteToPNG(const char *filename) const
{
    // open file for output
    const EAutoFile upFile { std::fopen(filename, "wb") };
    if(upFile == nullptr)
        return false;

    // create write struct
    png_structp pngptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if(pngptr == nullptr)
        return false;
    WCTPNGAutoRelease cRel { pngptr };

    // create png info struct
    png_infop infoptr = png_create_info_struct(pngptr);
    if(infoptr == nullptr)
        return false;
    cRel.SetInfoPtr(infoptr);

    // translate palette
    static png_color palette[PNG_MAX_PALETTE_LENGTH];
    TranslatePalette(palette, m_palette);

    // setup row pointers
    std::array<png_const_bytep, WCTConstants::CARDGFX_FULLHEIGHT_PX> rowptrs;
    for(euint k = 0; k < WCTConstants::CARDGFX_FULLHEIGHT_PX; k++)
        rowptrs[k] = m_pixels.data() + k * WCTConstants::CARDGFX_FULLWIDTH_PX;

    // setup error handling - no C++ objects in this scope!
    if(setjmp(png_jmpbuf(pngptr)) == 0)
    {
        // init output
        png_init_io(pngptr, upFile.get());
        
        // set IHDR information
        png_set_IHDR(
            pngptr, infoptr, 
            WCTConstants::CARDGFX_FULLWIDTH_PX, WCTConstants::CARDGFX_FULLHEIGHT_PX, 
            8,
            PNG_COLOR_TYPE_PALETTE,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
        );
        
        // set palette
        png_set_PLTE(pngptr, infoptr, palette, PNG_MAX_PALETTE_LENGTH);
        
        // write header info
        png_write_info(pngptr, infoptr);
        
        // write image data
        for(png_const_bytep row : rowptrs)
            png_write_row(pngptr, row);
        
        // finish write
        png_write_end(pngptr, infoptr);
    }
    else
    {
        return false;
    }

    // done
    return true;
}

//
// Translate PNG color palette to GBA
//
static void TranslatePaletteReverse(WCTCardPic::palette_t &outcolors, png_const_colorp incolors, int numcolors)
{
    outcolors.fill(0);
    const size_t len = std::min<>(outcolors.size(), size_t(numcolors));
    for(size_t i = 0; i < len; i++)
        outcolors[i] = WCTColor::RGBToRGB555(incolors[i].red, incolors[i].green, incolors[i].blue);
}

//
// Pack linear 8-bit image into 6bpp tiled data
//
void WCTCardPic::PackPixels()
{
    uint16_t             *dst  = reinterpret_cast<uint16_t *>(m_rawdata.data());
    const uint8_t *const  base = m_pixels.data();

    constexpr uint32_t tilepitch = (WCTConstants::CARDGFX_TILE_HEIGHT_PX * WCTConstants::CARDGFX_FULLWIDTH_PX);

    for(uint32_t ty = 0; ty < WCTConstants::CARDGFX_TILEMAP_HEIGHT; ty++)
    {
        for(uint32_t tx = 0; tx < WCTConstants::CARDGFX_TILEMAP_WIDTH; tx++)
        {
            const uint8_t *src   = base + ty * tilepitch + tx * WCTConstants::CARDGFX_TILE_WIDTH_PX;
            uint32_t       count = WCTConstants::CARDGFX_TILE_HEIGHT_PX;
            do
            {
                // pack 8 pixels into 3 words
                *dst++ =  (src[0] & 63)       | (uint16_t(src[1] & 63) << 6) | (uint16_t(src[2] & 15) << 12);
                *dst++ = ((src[2] & 48) >> 4) | (uint16_t(src[3] & 63) << 2) | (uint16_t(src[4] & 63) <<  8) | (uint16_t(src[5] & 3) << 14);
                *dst++ = ((src[5] & 60) >> 2) | (uint16_t(src[6] & 63) << 4) | (uint16_t(src[7] & 63) << 10);

                src += WCTConstants::CARDGFX_FULLWIDTH_PX;
            }
            while(--count != 0);
        }
    }
}

//
// Read in a PNG file
//
bool WCTCardPic::ReadFromPNG(const char *filename)
{
    // open file for input
    const EAutoFile upFile { std::fopen(filename, "rb") };
    if(upFile == nullptr)
        return false;

    // create read struct
    png_structp pngptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if(pngptr == nullptr)
        return false;
    WCTPNGAutoRelease cRel { pngptr, WCTPNGAutoRelease::Mode::READ };

    // allocate/initialize memory for image information
    png_infop infoptr = png_create_info_struct(pngptr);
    if(infoptr == nullptr)
        return false;
    cRel.SetInfoPtr(infoptr);

    // setup error handling - no C++ objects in this scope!
    if(setjmp(png_jmpbuf(pngptr)) == 0)
    {
        // init input
        png_init_io(pngptr, upFile.get());

        // read image info without any transforms
        png_read_png(pngptr, infoptr, PNG_TRANSFORM_IDENTITY, nullptr);

        // format assertions
        if(png_get_bit_depth(pngptr, infoptr) != 8) // 8bpp only
            return false;
        if(png_get_color_type(pngptr, infoptr) != PNG_COLOR_TYPE_PALETTE) // paletted only
            return false;
        if(png_get_image_width(pngptr, infoptr) != WCTConstants::CARDGFX_FULLWIDTH_PX) // must be 72px wide
            return false;
        if(png_get_image_height(pngptr, infoptr) != WCTConstants::CARDGFX_FULLHEIGHT_PX) // must be 80px tall
            return false;

        // get palette
        png_colorp pPalette   = nullptr;
        int        numPalette = 0;
        png_get_PLTE(pngptr, infoptr, &pPalette, &numPalette);
        if(pPalette == nullptr) // expecting valid 8bpp palette (only first 64 indices are used)
            return false;

        // translate to GBA palette format
        TranslatePaletteReverse(m_palette, pPalette, numPalette);

        // copy pixels to m_pixels
        png_bytepp rowptrs = png_get_rows(pngptr, infoptr);
        for(euint row = 0; row < WCTConstants::CARDGFX_FULLHEIGHT_PX; row++)
        {
            uint8_t *const dst = m_pixels.data() + row * WCTConstants::CARDGFX_FULLWIDTH_PX;
            std::memcpy(dst, rowptrs[row], WCTConstants::CARDGFX_FULLWIDTH_PX);
        }

        // translate pixels to 6bpp packed tiles
        PackPixels();
    }
    else
    {
        return false;
    }

    // done
    return true;
}

//
// Write out the raw GBA pixel data - 4320 bytes
//
bool WCTCardPic::WritePixels(const char *basefilename) const
{
    qstring outpath { basefilename };
    outpath.stripExtension();
    outpath += ".pix";

    return M_WriteFile(outpath.c_str(), m_rawdata.data(), m_rawdata.size()) != 0;
}

//
// Write out the GBA palette - 64 shorts (128 bytes)
//
bool WCTCardPic::WritePalette(const char *basefilename) const
{
    qstring outpath { basefilename };
    outpath.stripExtension();
    outpath += ".pal";

    return M_WriteFile(outpath.c_str(), m_palette.data(), m_palette.size() * sizeof(WCTColor::gbacolor_t)) != 0;
}

//
// Write raw GBA data to a pair of files (.pix and .pal)
//
bool WCTCardPic::WriteGBAData(const char *basefilename) const
{
    bool res;
    res = WritePixels(basefilename);
    res = WritePalette(basefilename) && res;
    return res;
}

// EOF
