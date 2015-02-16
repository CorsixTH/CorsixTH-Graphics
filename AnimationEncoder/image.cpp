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
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <string>
#include <png.h>
#include "image.h"

/** Number of colour channels in libpng for a single RGBA pixel. */
const int RGBA_CHANNELS_PER_PIXEL = 4;

uint32 MakeRGBA(uint8 r, uint8 g, uint8 b, uint8 a)
{
    assert(sizeof(uint32) == 4); // Not really the good place, but it has to be checked somewhere!

    uint32 ret;
    uint32 x;
    x = r; ret = x;
    x = g; ret |= (x << 8);
    x = b; ret |= (x << 16);
    x = a; ret |= (x << 24);
    return ret;
}

uint8 GetR(uint32 rgba) { return  rgba        & 0xFF; }
uint8 GetG(uint32 rgba) { return (rgba >> 8)  & 0xFF; }
uint8 GetB(uint32 rgba) { return (rgba >> 16) & 0xFF; }
uint8 GetA(uint32 rgba) { return (rgba >> 24) & 0xFF; }


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

static void OpenFile(const std::string &sFilename, png_structp *pngPtr, png_infop *infoPtr, png_infop *endInfo, uint8 ***pRows)
{
    FILE *pFile = fopen(sFilename.c_str(), "rb");
    if(pFile == NULL)
    {
        fprintf(stderr, "PNG file \"%s\" could not be opened.\n", sFilename.c_str());
        exit(1);
    }

    unsigned char header[4];
    if(fread(header, 1, 4, pFile) != 4)
    {
        fprintf(stderr, "Could not read header of \"%s\".\n", sFilename.c_str());
        fclose(pFile);
        exit(1);
    }
    bool bIsPng = !png_sig_cmp(header, 0, 4);
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
    png_set_sig_bytes(*pngPtr, 4);

    png_read_png(*pngPtr, *infoPtr, PNG_TRANSFORM_IDENTITY, NULL);
    *pRows = png_get_rows(*pngPtr, *infoPtr);
    fclose(pFile);
}

/** Perform cropping on the image.
 * @param pRows Data of the sprite, as loaded from the .PNG file.
 * @param[inout] left_edge Coordinate of the left-most column of the sprite. Updated in-place.
 * @param[inout] top_edge Coordinate of the top-most row of the sprite. Updated in-place.
 * @param[inout] width Number of columns in the image. Updated in-place.
 * @param[inout] height Number of rows in the image. Updated in-place.
 * @param[inout] xoffset Horizontal offset for displaying the sprite relative to the farthest corner of the tile. Updated in-place.
 * @param[inout] yoffset Vertical offset for displaying the sprite relative to the farthest corner of the tile. Updated in-place.
 */
void PerformCropping(uint8 **pRows, int *left_edge, int *top_edge, int *width, int *height, int *xoffset, int *yoffset)
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
            if (pPixel[3] != 0)
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
            if (pPixel[3] != 0)
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
            if (pPixel[3] != 0)
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
            if (pPixel[3] != 0)
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

/**
 * Load a 32bpp sprite from a file.
 * @param sFilename Name of the sprite file to load.
 * @param line Line number denoting the sprite definition in the input file.
 * @param[inout] left Left-most column in the PNG that is part of the sprite. Updated in-place.
 * @param[inout] width Number of columns in the sprite. Updated in-place.
 * @param[inout] top Top-most row in the PNG that is part of the sprite. Updated in-place.
 * @param[inout] height Number of rows in the sprite. Updated in-place.
 * @param[inout] xoffset Horizontal offset for displaying the sprite relative to the farthest corner of the tile. Updated in-place.
 * @param[inout] yoffset Vertical offset for displaying the sprite relative to the farthest corner of the tile. Updated in-place.
 * @return The loaded sprite.
 */
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
            *pData++ = MakeRGBA(pRow[0], pRow[1], pRow[2], pRow[3]);
            pRow += 4;
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
        fprintf(stderr, "Sprite at line %d: \"%s\" is not an 8bpp file (the channel is not 8 bit wide\n", line, sFilename.c_str());
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
