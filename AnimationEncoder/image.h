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

//! @file image.h PNG file loader code.

#ifndef IMAGE_H
#define IMAGE_H

typedef unsigned char uint8;
typedef unsigned int uint32;

static const int TRANSPARENT = 0; ///< Value of a transparent pixel in the #CH_OPACITY channel.
static const int OPAQUE = 255;    ///< Value of a fully opaque pixel in the #CH_OPACITY channel.

/** 32bpp RGBA image storage. */
class Image32bpp
{
public:
    //! Constructor.
    /*!
        @param iWidth Width of the image.
        @param iHeight Height of the image.
    */
    Image32bpp(int iWidth, int iHeight);

    //! Copy constructor.
    /*!
        @param img Image to copy.
     */
    Image32bpp(const Image32bpp &img);
    ~Image32bpp();

    //! Get pixel colour at the given \a offset.
    /*!
        @param offset Offset of the pixel to retrieve, in pixels from the top-left, in horizontal rows.
     */
    uint32 Get(int offset) const;

    int iWidth;    ///< Width of the image in pixels.
    int iHeight;   ///< Height of the image in pixels.
    uint32 *pData; ///< Stored data, one value per pixel, in horizontal rows.
                   ///< Encoded using #MakeRGBA, decoding to channels with #GetR, #GetG, #GetB, #GetA.
};

/** 8bpp image storage, for recolouring layers. */
class Image8bpp
{
public:
    //! Constructor.
    /*!
        @param iWidth Width of the image.
        @param iHeight Height of the image.
    */
    Image8bpp(int iWidth, int iHeight);

    //! Copy constructor.
    /*!
        @param img Image to copy.
     */
    Image8bpp(const Image8bpp &img);
    ~Image8bpp();

    //! Get pixel colour at the given \a offset.
    /*!
        @param offset Offset of the pixel to retrieve, in pixels from the top-left, in horizontal rows.
     */
    uint8 Get(int offset) const;

    int iWidth;   ///< Width of the image in pixels.
    int iHeight;  ///< Height of the image in pixels.
    uint8 *pData; ///< Stored data, one value per pixel, in horizontal rows.
};

//! Get the red colour channel from the encoded \a rgba pixel.
/*!
    @param rgba Encoded pixel value.
    @return The amount of red colour.
 */
uint8 GetR(uint32 rgba);

//! Get the green colour channel from the encoded \a rgba pixel.
/*!
    @param rgba Encoded pixel value.
    @return The amount of green colour.
 */
uint8 GetG(uint32 rgba);

//! Get the blue colour channel from the encoded \a rgba pixel.
/*!
    @param rgba Encoded pixel value.
    @return The amount of blue colour.
 */
uint8 GetB(uint32 rgba);

//! Get the opacity channel from the encoded \a rgba pixel.
/*!
    @param rgba Encoded pixel value.
    @return The amount of opacity.
 */
uint8 GetA(uint32 rgba);

//! Load a 32bpp sprite from a file.
/*!
    @param sFilename Name of the sprite file to load.
    @param line Line number denoting the sprite definition in the input file.
    @param[inout] left Left-most column in the PNG that is part of the sprite. Updated in-place.
    @param[inout] width Number of columns in the sprite. Updated in-place.
    @param[inout] top Top-most row in the PNG that is part of the sprite. Updated in-place.
    @param[inout] height Number of rows in the sprite. Updated in-place.
    @param[inout] xoffset Horizontal offset for displaying the sprite relative to the farthest corner of the tile. Updated in-place.
    @param[inout] yoffset Vertical offset for displaying the sprite relative to the farthest corner of the tile. Updated in-place.
    @return The loaded sprite.
 */
Image32bpp *Load32Bpp(const std::string &sFilename, int line, int *left, int *width, int *top, int *height, int *xoffset, int *yoffset);

//! Load 8bpp recolour layer definition file.
/*!
    Recolour layer files should match with their 32bpp cousins. As such, positions and sizes are given rather than retrieved from the file.

    @param sFilename Name of the sprite file to load.
    @param line Line number denoting the sprite definition in the input file.
    @param left Left-most column in the PNG that is part of the sprite.
    @param width Number of columns in the sprite.
    @param top Top-most row in the PNG that is part of the sprite.
    @param height Number of rows in the sprite.
    @return The indicated area of the loaded sprite.
 */
Image8bpp *Load8Bpp(const std::string &sFilename, int line, int left, int width, int top, int height);

#endif

// vim: et sw=4 ts=4 sts=4
