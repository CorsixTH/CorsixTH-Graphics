/*
Copyright (c) 2013-2014 Albert "Alberth" Hofkamp

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//! @file image.cpp PNG file loader code.

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <string>
#include <png.h>
#include "image.h"

const int RGBA_CHANNELS_PER_PIXEL = 4;  ///< Number of colour channels in libpng for a single RGBA pixel.

/** Channel numbers of an RGBA pixel. */
enum RgbaChannelNumber
{
    CH_RED,     ///< Red colour channel index.
    CH_GREEN,   ///< Green colour channel index.
    CH_BLUE,    ///< Blue colour channel index.
    CH_OPACITY, ///< Opacity channel index.
};

/** Offsets in #uint32 for encoding a RGBA pixel. */
enum RgbaPixelEncoding
{
    PXENC_RED     = 0,  ///< Lowest bit containing the red colour channel information.
    PXENC_GREEN   = 8,  ///< Lowest bit containing the green colour channel information.
    PXENC_BLUE    = 16, ///< Lowest bit containing the blue colour channel information.
    PXENC_OPACITY = 24, ///< Lowest bit containing the opacity channel information.

    PXENC_CHANNEL_MASK = 0xFF, ///< Mask to apply to get a single channel from the encoded pixel.
};


uint32 MakeRGBA(uint8 r, uint8 g, uint8 b, uint8 a)
{
    assert(sizeof(uint32) == 4); // Not really the good place, but it has to be checked somewhere!

    uint32 ret;
    uint32 x;
    x = r; ret  = (x << PXENC_RED);
    x = g; ret |= (x << PXENC_GREEN);
    x = b; ret |= (x << PXENC_BLUE);
    x = a; ret |= (x << PXENC_OPACITY);
    return ret;
}

uint8 GetR(uint32 rgba) { return (rgba >> PXENC_RED)     & PXENC_CHANNEL_MASK; }
uint8 GetG(uint32 rgba) { return (rgba >> PXENC_GREEN)   & PXENC_CHANNEL_MASK; }
uint8 GetB(uint32 rgba) { return (rgba >> PXENC_BLUE)    & PXENC_CHANNEL_MASK; }
uint8 GetA(uint32 rgba) { return (rgba >> PXENC_OPACITY) & PXENC_CHANNEL_MASK; }

Image32bpp::Image32bpp(int iWidth, int iHeight)
{
    this->iWidth = iWidth;
    this->iHeight = iHeight;
    pData = (uint32 *)malloc(RGBA_CHANNELS_PER_PIXEL * iWidth * iHeight);
}

Image32bpp::Image32bpp(const Image32bpp &img)
{
    this->iWidth = img.iWidth;
    this->iHeight = img.iHeight;
    pData = (uint32 *)malloc(RGBA_CHANNELS_PER_PIXEL * iWidth * iHeight);
    memcpy(pData, img.pData, RGBA_CHANNELS_PER_PIXEL * iWidth * iHeight);
}

Image32bpp::~Image32bpp()
{
    free(pData);
}

uint32 Image32bpp::Get(int offset) const
{
    int x, y;
    y = offset / iWidth;
    x = offset - y * iWidth;
    assert(x >= 0 && x < iWidth && y >= 0 && y < iHeight);
    return pData[offset];
}


Image8bpp::Image8bpp(int iWidth, int iHeight)
{
    this->iWidth = iWidth;
    this->iHeight = iHeight;
    pData = (uint8 *)malloc(iWidth * iHeight);
}

Image8bpp::Image8bpp(const Image8bpp &img)
{
    this->iWidth = img.iWidth;
    this->iHeight = img.iHeight;
    pData = (uint8 *)malloc(iWidth * iHeight);
    memcpy(pData, img.pData, iWidth * iHeight);
}

Image8bpp::~Image8bpp()
{
    free(pData);
}

unsigned char Image8bpp::Get(int offset) const
{
    int x, y;
    y = offset / iWidth;
    x = offset - y * iWidth;
    assert(x >= 0 && x < iWidth && y >= 0 && y < iHeight);
    return pData[offset];
}

//! Open an image (.png) file.
/*!
    @param sFilename Filename of the file to open.
    @param [out] pngPtr Libpng data structure.
    @param [out] infoPtr Libpng info structure.
    @param [out] endInfo Libpng info structure.
    @param [out] pRows Rows of pixel channel information, read from the file.
 */
static void OpenFile(const std::string &sFilename, png_structp *pngPtr, png_infop *infoPtr, png_infop *endInfo, uint8 ***pRows)
{
    const int PNG_SIGNATURE_SIZE = 4; // Number of bytes to read from the header for checking the given file is a PNG file.

    FILE *pFile = fopen(sFilename.c_str(), "rb");
    if(pFile == NULL)
    {
        fprintf(stderr, "PNG file \"%s\" could not be opened.\n", sFilename.c_str());
        exit(1);
    }

    unsigned char header[PNG_SIGNATURE_SIZE];
    if(fread(header, 1, PNG_SIGNATURE_SIZE, pFile) != PNG_SIGNATURE_SIZE)
    {
        fprintf(stderr, "Could not read header of \"%s\".\n", sFilename.c_str());
        fclose(pFile);
        exit(1);
    }
    bool bIsPng = !png_sig_cmp(header, 0, PNG_SIGNATURE_SIZE);
    if(!bIsPng)
    {
        fprintf(stderr, "Header of \"%s\" indicates it is not a PNG file.\n", sFilename.c_str());
        fclose(pFile);
        exit(1);
    }

    *pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!*pngPtr)
    {
        fprintf(stderr, "Could not initialize PNG data.\n");
        fclose(pFile);
        exit(1);
    }
    *infoPtr = png_create_info_struct(*pngPtr);
    if(!*infoPtr)
    {
        fprintf(stderr, "Could not initialize PNG info data.\n");
        png_destroy_read_struct(pngPtr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(pFile);
        exit(1);
    }

    *endInfo = png_create_info_struct(*pngPtr);
    if(!*endInfo)
    {
        fprintf(stderr, "Could not initialize PNG end data.\n");
        png_destroy_read_struct(pngPtr, infoPtr, (png_infopp)NULL);
        fclose(pFile);
        exit(1);
    }

    /* Setup callback in case of errors. */
    if(setjmp(png_jmpbuf(*pngPtr))) {
        fprintf(stderr, "Error detected while reading PNG file.\n");
        png_destroy_read_struct(pngPtr, infoPtr, endInfo);
        fclose(pFile);
        exit(1);
    }

    /* Initialize for file reading. */
    png_init_io(*pngPtr, pFile);
    png_set_sig_bytes(*pngPtr, PNG_SIGNATURE_SIZE);

    png_read_png(*pngPtr, *infoPtr, PNG_TRANSFORM_IDENTITY, NULL);
    *pRows = png_get_rows(*pngPtr, *infoPtr);
    fclose(pFile);
}

//! Perform cropping on the image.
/*!
    @param pRows Data of the sprite, as loaded from the .PNG file.
    @param[inout] left_edge Coordinate of the left-most column of the sprite. Updated in-place.
    @param[inout] top_edge Coordinate of the top-most row of the sprite. Updated in-place.
    @param[inout] width Number of columns in the image. Updated in-place.
    @param[inout] height Number of rows in the image. Updated in-place.
    @param[inout] xoffset Horizontal offset for displaying the sprite relative to the farthest corner of the tile. Updated in-place.
    @param[inout] yoffset Vertical offset for displaying the sprite relative to the farthest corner of the tile. Updated in-place.
 */
static void PerformCropping(uint8 **pRows, int *left_edge, int *top_edge, int *width, int *height, int *xoffset, int *yoffset)
{
    int xpoint = *left_edge - *xoffset;
    int ypoint = *top_edge - *yoffset;
    int left = *left_edge;
    int top = *top_edge;
    int right = left + *width - 1;
    int bottom = top + *height - 1;

    while (left <= right)
    {
        bool ok = true;
        for (int y = top; y <= bottom; y++)
        {
            uint8 *pPixel = pRows[y] + left;
            if (pPixel[CH_OPACITY] != TRANSPARENT)
            {
                ok = false;
                break;
            }
        }
        if (!ok)
            break;

        left++;
    }
    while (left <= right)
    {
        bool ok = true;
        for (int y = top; y <= bottom; y++)
        {
            uint8 *pPixel = pRows[y] + right;
            if (pPixel[CH_OPACITY] != TRANSPARENT)
            {
                ok = false;
                break;
            }
        }
        if (!ok)
            break;

        right--;
    }

    while (top <= bottom)
    {
        bool ok = true;
        for (int x = left; x <= right; x++)
        {
            uint8 *pPixel = pRows[top] + x;
            if (pPixel[CH_OPACITY] != TRANSPARENT)
            {
                ok = false;
                break;
            }
        }
        if (!ok)
            break;

        top++;
    }
    while (top <= bottom)
    {
        bool ok = true;
        for (int x = left; x <= right; x++)
        {
            uint8 *pPixel = pRows[bottom] + x;
            if (pPixel[CH_OPACITY] != TRANSPARENT)
            {
                ok = false;
                break;
            }
        }
        if (!ok)
            break;

        bottom--;
    }

    *left_edge = left;
    *top_edge = top;
    *width = right - left + 1;
    *height = bottom - top + 1;
    *xoffset = left - xpoint;
    *yoffset = top - ypoint;
}

Image32bpp *Load32Bpp(const std::string &sFilename, int line, int *left, int *width, int *top, int *height, int *xoffset, int *yoffset)
{
    png_structp pngPtr;
    png_infop infoPtr;
    png_infop endInfo;
    uint8 **pRows;
    OpenFile(sFilename, &pngPtr, &infoPtr, &endInfo, &pRows);

    int iWidth = png_get_image_width(pngPtr, infoPtr);
    int iHeight = png_get_image_height(pngPtr, infoPtr);
    int iBitDepth = png_get_bit_depth(pngPtr, infoPtr);
    int iColorType = png_get_color_type(pngPtr, infoPtr);

    /* Initialize sprite width and height if not set, clamping at 0. */
    if (*width < 0)
        *width = iWidth - *left;
    if (*width < 0)
        *width = 0;

    if (*height < 0)
        *height = iHeight - *top;
    if (*height < 0)
        *height = 0;

    /* Test whether the sprite fits in the image. */
    if (iWidth < *left + *width)
    {
        fprintf(stderr, "Sprite at line %d: Sprite is not wide enough, require %d columns (%d + %d) while only %d columns are available.\n", line, *left + *width, *left, *width, iWidth);
        exit(1);
    }
    if (iHeight < *top + *height)
    {
        fprintf(stderr, "Sprite at line %d: Sprite is not high enough, require %d rows (%d + %d) while only %d rows are available.\n", line, *top + *height, *top, *height, iHeight);
        exit(1);
    }

    if (iBitDepth != 8)
    {
        fprintf(stderr, "Sprite at line %d: \"%s\" is not an 32bpp file (channels are not 8 bit wide)\n", line, sFilename.c_str());
        png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
        exit(1);
    }
    if (iColorType != PNG_COLOR_TYPE_RGB_ALPHA)
    {
        fprintf(stderr, "Sprite at line %d: \"%s\" is not an RGBA file\n", line, sFilename.c_str());
        png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
        exit(1);
    }

    PerformCropping(pRows, left, top, width, height, xoffset, yoffset);
    if (*width == 0 || *height == 0)
    {
        fprintf(stderr, "Sprite at line %d: \"%s\" is empty\n", line, sFilename.c_str());
        png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
        exit(1);
    }

    Image32bpp *img = new Image32bpp(*width, *height);
    uint32 *pData = img->pData;
    for (int i = 0; i < *height; i++)
    {
        uint8 *pRow = pRows[*top + i] + (*left * RGBA_CHANNELS_PER_PIXEL);
        for (int j = 0; j < *width; j++)
        {
            *pData++ = MakeRGBA(pRow[CH_RED], pRow[CH_GREEN], pRow[CH_BLUE], pRow[CH_OPACITY]);
            pRow += RGBA_CHANNELS_PER_PIXEL;
        }
    }

    png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
    return img;
}

Image8bpp *Load8Bpp(const std::string &sFilename, int line, int left, int width, int top, int height)
{
    png_structp pngPtr;
    png_infop infoPtr;
    png_infop endInfo;
    uint8 **pRows;
    OpenFile(sFilename, &pngPtr, &infoPtr, &endInfo, &pRows);

    int iWidth = png_get_image_width(pngPtr, infoPtr);
    int iHeight = png_get_image_height(pngPtr, infoPtr);
    int iBitDepth = png_get_bit_depth(pngPtr, infoPtr);
    int iColorType = png_get_color_type(pngPtr, infoPtr);

    if (iWidth < left + width)
    {
        fprintf(stderr, "Sprite at line %d: Sprite is not wide enough, require %d columns (%d + %d) while only %d columns are available.\n", line, left + width, left, width, iWidth);
        exit(1);
    }
    if (iHeight < top + height)
    {
        fprintf(stderr, "Sprite at line %d: Sprite is not high enough, require %d rows (%d + %d) while only %d rows are available.\n", line, top + height, top, height, iHeight);
        exit(1);
    }

    if (iBitDepth != 8)
    {
        fprintf(stderr, "Sprite at line %d: \"%s\" is not an 8bpp file (the channel is not 8 bit wide)\n", line, sFilename.c_str());
        png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
        exit(1);
    }
    if (iColorType != PNG_COLOR_TYPE_PALETTE)
    {
        fprintf(stderr, "Sprite at line %d: \"%s\" is not a palleted image file\n", line, sFilename.c_str());
        png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
        exit(1);
    }

    Image8bpp *img = new Image8bpp(width, height);
    uint8 *pData = img->pData;
    for (int i = 0; i < height; i++)
    {
        int y = top + i;
        for (int j = 0; j < width; j++)
        {
            int x = left + j;
            uint8 v = pRows[y][x];
            *pData = v;
            pData++;
        }
    }
    return img;
}

// vim: et sw=4 ts=4 sts=4
