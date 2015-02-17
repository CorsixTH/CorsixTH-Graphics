/*
Copyright (c) 2014 Albert "Alberth" Hofkamp

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

//! @file ast.cpp Abstract Syntax Tree (AST) classes for storing parsed information.

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include "ast.h"
#include "storage.h"
#include "image.h"

std::map<AnimationGroupKey, AnimationGroup> g_mapAnimGroups; ///< Available animation groups, point into #g_vAnimations.

/** Name of the program. */
#define PROGNAME "encoder"

static const int MAX_DISPLAY_COND = 12; ///< Max layer class.

//! Return the tile size to use if none was specified.
/*!
    @return The default tile size.
 */
static int GetDefaultTileSize()
{
    return 64;
}

//! Is the provided tile size valid?
/*!
    @param size Tile size to test.
    @return Whether the given tile size is valid.
 */
static bool IsValidTileSize(int size)
{
    return size == 64; // Extend condition to allow other tile sizes.
}

FieldStorage::FieldStorage()
{
    m_iNumber = FN_NOTHING;
}

FieldStorage::FieldStorage(int number, int value, int line)
{
    m_iNumber = number;
    m_iValue = value;
    m_iLine = line;
}

FieldStorage::FieldStorage(int number, int key, int value, int line)
{
    m_iNumber = number;
    m_iKey = key;
    m_iValue = value;
    m_iLine = line;
}

FieldStorage::FieldStorage(int number, std::string text, int line)
{
    m_iNumber = number;
    m_sText = text;
    m_iLine = line;
}

FieldStorage::FieldStorage(const FieldStorage &fs)
{
    m_iNumber = fs.m_iNumber;
    m_iKey = fs.m_iKey;
    m_iValue = fs.m_iValue;
    m_sText = fs.m_sText;
    m_iLine = fs.m_iLine;
}

FieldStorage &FieldStorage::operator=(const FieldStorage &fs)
{
    if (this != &fs)
    {
        m_iNumber = fs.m_iNumber;
        m_iKey = fs.m_iKey;
        m_iValue = fs.m_iValue;
        m_sText = fs.m_sText;
        m_iLine = fs.m_iLine;
    }
    return *this;
}

FrameElement::FrameElement()
{
}

FrameElement::FrameElement(int line)
{
    m_iLine = line;
    m_iTop = 0;
    m_iLeft = 0;
    m_iWidth = -1;
    m_iHeight = -1;
    m_iXoffset = 0;
    m_iYoffset = 0;
    m_sBaseImage = "";
    m_sRecolourImage = "";
    for (int i = 0; i < 256; i++)
        m_aNumber[i] = i;

    m_oDisplay.m_iKey = -1;
    m_iAlpha = 100;
    m_bHorFlip = false;
    m_bVertFlip = false;
}

FrameElement::FrameElement(const FrameElement &fe)
{
    m_iTop = fe.m_iTop;
    m_iLeft = fe.m_iLeft;
    m_iWidth = fe.m_iWidth;
    m_iHeight = fe.m_iHeight;
    m_iXoffset = fe.m_iXoffset;
    m_iYoffset = fe.m_iYoffset;
    m_sBaseImage = fe.m_sBaseImage;
    m_sRecolourImage = fe.m_sRecolourImage;
    for (int i = 0; i < 256; i++)
        m_aNumber[i] = fe.m_aNumber[i];

    m_oDisplay = fe.m_oDisplay;
    m_iAlpha = fe.m_iAlpha;
    m_bHorFlip = fe.m_bHorFlip;
    m_bVertFlip = fe.m_bVertFlip;
}

FrameElement &FrameElement::operator=(const FrameElement &fe)
{
    if (this != &fe)
    {
        m_iTop = fe.m_iTop;
        m_iLeft = fe.m_iLeft;
        m_iWidth = fe.m_iWidth;
        m_iHeight = fe.m_iHeight;
        m_iXoffset = fe.m_iXoffset;
        m_iYoffset = fe.m_iYoffset;
        m_sBaseImage = fe.m_sBaseImage;
        m_sRecolourImage = fe.m_sRecolourImage;
        for (int i = 0; i < 256; i++)
            m_aNumber[i] = fe.m_aNumber[i];

        m_oDisplay = fe.m_oDisplay;
        m_iAlpha = fe.m_iAlpha;
        m_bHorFlip = fe.m_bHorFlip;
        m_bVertFlip = fe.m_bVertFlip;
    }
    return *this;
}


//! Set the properties of the element.
/*!
    @param fields Properties to assign.
 */
void FrameElement::SetProperties(const std::vector<FieldStorage> &fields)
{
    int seen[FN_NUMBER_ENTRIES];
    for (int i = 0; i < FN_NUMBER_ENTRIES; i++)
        seen[i] = -1; // Haven't seen this field so far.

    for (int i = 0; i < static_cast<int>(fields.size()); i++)
    {
        const FieldStorage &fs = fields[i];

        // Check the assignment hasn't been done before.
        if (seen[fs.m_iNumber] >= 0)
        {
            fprintf(stderr, PROGNAME ", line %d: Field is already assigned "
                    "at line %d\n", fs.m_iLine, seen[fs.m_iNumber]);
            exit(1);
        }
        seen[fs.m_iNumber] = fs.m_iLine;

        // Copy field value into a data member.
        switch(fs.m_iNumber) {
        case FE_TOP:
            m_iTop = fs.m_iValue;
            break;

        case FE_LEFT:
            m_iLeft = fs.m_iValue;
            break;

        case FE_WIDTH:
            m_iWidth = fs.m_iValue;
            break;

        case FE_HEIGHT:
            m_iHeight = fs.m_iValue;
            break;

        case FE_XOFFSET:
            m_iXoffset = fs.m_iValue;
            break;

        case FE_YOFFSET:
            m_iYoffset = fs.m_iValue;
            break;

        case FE_IMAGE:
            m_sBaseImage = fs.m_sText;
            break;

        case FE_RECOLOUR:
            m_sRecolourImage = fs.m_sText;
            break;

        case FE_RECOLLAYER:
            m_aNumber[fs.m_iKey] = fs.m_iValue;
            break;

        case FE_DISPLAY:
            m_oDisplay.m_iKey = fs.m_iKey;
            m_oDisplay.m_iValue = fs.m_iValue;
            break;

        case FE_ALPHA:
            m_iAlpha = fs.m_iValue;
            break;

        case FE_HORFLIP:
            m_bHorFlip = true;
            break;

        case FE_VERTFLIP:
            m_bVertFlip = true;
            break;

        default:
            assert(false); // Unknown property found.
            break;
        }
    }
}

void FrameElement::Check()
{
    if (m_sBaseImage == "")
    {
        fprintf(stderr, PROGNAME ", line %d: Image name may no be empty\n", m_iLine);
        exit(1);
    }

    if (m_oDisplay.m_iKey != -1)
    {
        if (m_oDisplay.m_iKey < 0 || m_oDisplay.m_iKey > MAX_DISPLAY_COND)
        {
            fprintf(stderr, PROGNAME ", line %d: Display condition key must "
                    "be between 0 and %d\n", m_iLine, MAX_DISPLAY_COND);
            exit(1);
        }

        if (m_oDisplay.m_iValue < 0 || m_oDisplay.m_iValue > 255)
        {
            fprintf(stderr, PROGNAME ", line %d: Display condition value "
                    "must be between 0 and 255\n", m_iLine);
            exit(1);
        }
    }

    if (m_iAlpha != 100 && m_iAlpha != 75 && m_iAlpha != 50)
    {
        fprintf(stderr, PROGNAME ", line %d: Alpha value must be 50, 75, "
                "or 100\n", m_iLine);
        exit(1);
    }
}

//! Look ahead in the recolour bitmaps to check when the next recoloured pixels will occur.
/*!
    @param iCount Current index in the image.
    @param iEndCount End of the image.
    @param pLayer Recolouring bitmap (if available).
    @return Number of pixels to go before the next recoloured pixel (limited to 63 look ahead).
 */
static int GetDistanceToNextRecolour(const uint32 iCount, const uint32 iEndCount, const Image8bpp *pLayer)
{
    uint32 iLength = iEndCount - iCount;
    if (iLength > 63) iLength = 63; // No need to look ahead further.

    if (pLayer != NULL)
    {
        for (size_t i = 0; i < iLength; i++)
        {
            if (pLayer->Get(iCount + i) != 0) return i;
        }
    }
    return iLength;
}

//! Get the recolour table to use, and the number of pixels to recolour.
/*!
    @param iCount Current index in the image.
    @param iEndCount End of the image.
    @param pLayer Recolouring bitmap.
    @param pLayerNumber [out] Number of the recolouring table to use.
    @return Number of pixels to recolour from the current position.
 */
static int GetRecolourInformation(const uint32 iCount, const uint32 iEndCount, const Image8bpp *pLayer, uint8 *pLayerNumber)
{
    uint32 iLength = iEndCount - iCount;
    if (iLength > 63) iLength = 63; // No need to look ahead further.

    *pLayerNumber = pLayer->Get(iCount);
    for (size_t i = 1; i < iLength; i++)
    {
        if (pLayer->Get(iCount + i) != *pLayerNumber) return i;
    }
    return iLength;
}

//! Look ahead in the base image to check how many pixels from the current position have the same opacity.
/*!
    @param iCount Current index in the image.
    @param iEndCount End of the image.
    @param oBase Base image.
    @return Number of pixels to go before the opacity of the current pixel changes (limited to 63 look ahead).
 */
static int GetDistanceToNextTransparency(const uint32 iCount, const uint32 iEndCount, const Image32bpp &oBase)
{
    uint32 iLength = iEndCount - iCount;
    if (iLength > 63) iLength = 63; // No need to look ahead further.

    uint8 iOpacity = GetA(oBase.Get(iCount));
    for (uint i = 1; i < iLength; i++)
    {
        if (iOpacity != GetA(oBase.Get(iCount + i))) return i;
    }
    return iLength;
}

//! Write the RGB colour for the next \a iLength pixels, starting from the \a iCount offset.
/*!
    @param oBase Base image to encode.
    @param iCount Current index in the image.
    @param iLength Number of pixels to process.
    @param pDest Destination to write to.
 */
static void WriteColour(const Image32bpp &oBase, uint32 iCount, int iLength, Output *pDest)
{
    while (iLength > 0)
    {
        uint32 iColour = oBase.Get(iCount);
        iCount++;
        pDest->Uint8(GetR(iColour));
        pDest->Uint8(GetG(iColour));
        pDest->Uint8(GetB(iColour));
        iLength--;
    }
}

//! Write the table index for the next \a iLength pixels, starting from the \a iCount offset.
/*!
    @param oBase Base image to encode.
    @param iCount Current index in the image.
    @param iLength Number of pixels to process.
    @param pDest Destination to write to.
 */
static void WriteTableIndex(const Image32bpp &oBase, uint32 iCount, int iLength, Output *pDest)
{
    while (iLength > 0)
    {
        uint32 iColour = oBase.Get(iCount);
        iCount++;
        uint8 biggest = GetR(iColour);
        if (biggest < GetG(iColour)) biggest = GetG(iColour);
        if (biggest < GetB(iColour)) biggest = GetB(iColour);
        pDest->Uint8(biggest);
        iLength--;
    }
}

//! Encode a 32bpp image from the \a oBase image, and optionally the recolouring \a pLayer bitmap.
static void Encode32bpp(int iWidth, int iHeight, const Image32bpp &oBase, const Image8bpp *pLayer, Output *pDest, const unsigned char *pNumber)
{
    const uint32 iPixCount = iWidth * iHeight;
    uint32 iCount = 0;
    while (iCount < iPixCount)
    {
        int iLength = GetDistanceToNextRecolour(iCount, iPixCount, pLayer);
        int length2 = GetDistanceToNextTransparency(iCount, iPixCount, oBase);

        if (iLength > 63) iLength = 63;
        if (iLength == 0) { // Recolour layer.
            uint8 iTableNumber;
            iLength = GetRecolourInformation(iCount, iPixCount, pLayer, &iTableNumber);
            if (length2 < iLength) iLength = length2;
            assert(iLength > 0);

            pDest->Uint8(64 + 128 + iLength);
            pDest->Uint8(pNumber[iTableNumber]);
            pDest->Uint8(GetA(oBase.Get(iCount))); // Opacity.
            WriteTableIndex(oBase, iCount, iLength, pDest);
            iCount += iLength;
            continue;
        }
        if (length2 < iLength) iLength = length2;
        assert(iLength > 0);

        uint8 iOpacity = GetA(oBase.Get(iCount));
        if (iOpacity == OPAQUE) { // Fixed non-transparent 32bpp pixels (RGB).
            pDest->Uint8(iLength);
            WriteColour(oBase, iCount, iLength, pDest);
            iCount += iLength;
            continue;
        }
        if (iOpacity == TRANSPARENT) { // Fixed fully transparent pixels.
            pDest->Uint8(128 + iLength);
            iCount += iLength;
            continue;
        }
        /* Partially transparent 32bpp pixels (RGB). */
        pDest->Uint8(64 + iLength);
        pDest->Uint8(iOpacity);
        WriteColour(oBase, iCount, iLength, pDest);
        iCount += iLength;
        continue;
    }
}

bool FrameElement::WriteSprite(Output *pOut, int *iXoffset, int  *iYoffset) const
{
    int iLeft = m_iLeft;
    int iWidth = m_iWidth;
    int iTop = m_iTop;
    int iHeight = m_iHeight;
    *iXoffset = m_iXoffset;
    *iYoffset = m_iYoffset;

    Image32bpp *pBase = Load32Bpp(m_sBaseImage, m_iLine, &iLeft, &iWidth, &iTop, &iHeight, iXoffset, iYoffset);
    if (pBase == NULL)
    {
        fprintf(stderr, "Warning: Skipping sprite at line %d: Image load failed.\n", m_iLine);
        return false;
    }

    Image8bpp *pLayer = NULL;
    if (m_sRecolourImage != "")
        pLayer = Load8Bpp(m_sRecolourImage, m_iLine, iLeft, iWidth, iTop, iHeight);

    int iStart = pOut->Reserve(0);
    pOut->Uint8('S');
    pOut->Uint8('P');
    pOut->Uint16(iWidth);
    pOut->Uint16(iHeight);
    int iAddress = pOut->Reserve(4);
    int iEnd = pOut->Reserve(0);
    assert(iEnd - iStart == SPRITE_NON_DATA_SIZE); // Non-data size must match.

    Encode32bpp(iWidth, iHeight, *pBase, pLayer, pOut, m_aNumber);

    int iLength = pOut->Reserve(0) - (iAddress + 4); // Start counting after the length.
    pOut->Write32(iAddress, iLength);

    delete pLayer;
    delete pBase;
    return true;
}

AnimationFrame::AnimationFrame()
{
}

AnimationFrame::AnimationFrame(int line)
{
    m_iLine = line;
    m_iSound = -1;
    m_vElements.clear();
}

AnimationFrame::AnimationFrame(const AnimationFrame &af)
{
    m_iSound = af.m_iSound;
    m_vElements = af.m_vElements;
}
AnimationFrame &AnimationFrame::operator=(const AnimationFrame &af)
{
    if (this != &af)
    {
        m_iSound = af.m_iSound;
        m_vElements = af.m_vElements;
    }
    return *this;
}

//! Set the properties of the frame.
/*!
    @param field Property to set (if not empty).
 */
void AnimationFrame::SetProperty(const FieldStorage &field)
{
    if (field.m_iKey == AF_SOUND)
        m_iSound = field.m_iValue;
}

//! Set the elements of the frame.
/*!
    @param elements Elements to assign.
 */
void AnimationFrame::SetElements(const std::vector<FrameElement> &elements)
{
    m_vElements = elements;
}

//! Perform integrity checking on the supplied data.
void AnimationFrame::Check()
{
    if (m_vElements.size() == 0)
    {
        fprintf(stderr, PROGNAME ", line %d: Frame must have at least one "
                "element\n", m_iLine);
        exit(1);
    }

    for (ElementIterator iter = m_vElements.begin(); iter != m_vElements.end(); iter++)
    {
        (*iter).Check();
    }
}

Animation::Animation()
{
}

Animation::Animation(int line, const std::string &name)
{
    m_iLine = line;
    m_sName = name;

    m_iTileSize = -1;
    m_eViewDirection = VD_INVALID;
    m_vFrames.clear();
}

Animation::Animation(const Animation &an)
{
    m_iLine = an.m_iLine;
    m_sName = an.m_sName;
    m_iTileSize = an.m_iTileSize;
    m_eViewDirection = an.m_eViewDirection;
    m_vFrames = an.m_vFrames;
}

Animation &Animation::operator=(const Animation &an)
{
    if (this != &an)
    {
        m_iLine = an.m_iLine;
        m_sName = an.m_sName;
        m_iTileSize = an.m_iTileSize;
        m_eViewDirection = an.m_eViewDirection;
        m_vFrames = an.m_vFrames;
    }
    return *this;
}

//! Set the properties of the animation.
/*!
    @param fields Properties to assign.
 */
void Animation::SetProperties(const std::vector<FieldStorage> &fields)
{
    int seen[FN_NUMBER_ENTRIES];
    for (int i = 0; i < FN_NUMBER_ENTRIES; i++)
        seen[i] = -1; // Haven't seen this field so far.

    for (int i = 0; i < static_cast<int>(fields.size()); i++)
    {
        const FieldStorage &fs = fields[i];

        // Check the assignment hasn't been done before.
        if (seen[fs.m_iNumber] >= 0)
        {
            fprintf(stderr, PROGNAME ", line %d: Field is already assigned "
                    "at line %d\n", fs.m_iLine, seen[fs.m_iNumber]);
            exit(1);
        }
        seen[fs.m_iNumber] = fs.m_iLine;

        // Copy field value into a data member.
        switch(fs.m_iNumber) {
        case AP_TILESIZE:
            m_iTileSize = fs.m_iValue;
            break;
        case AP_VIEW:
            m_eViewDirection = static_cast<ViewDirection>(fs.m_iValue);
            break;
        default:
            assert(false); // Unknown property found.
            break;
        }
    }
}

//! Copy the frames into the animation.
/*!
    @param frames Frames to copy.
 */
void Animation::SetFrames(const std::vector<AnimationFrame> &frames)
{
    m_vFrames = frames;
}

//! Perform integrity checking on the supplied data.
void Animation::Check()
{
    if (m_iTileSize < 0)
        m_iTileSize = GetDefaultTileSize();
    if (!IsValidTileSize(m_iTileSize))
    {
        fprintf(stderr, PROGNAME ", line %d: Tilesize %d is not valid\n",
                m_iLine, m_iTileSize);
        exit(1);
    }

    if (m_sName == "")
    {
        fprintf(stderr, PROGNAME ", line %d: Name of the animation may not "
                "be empty\n", m_iLine);
        exit(1);
    }

    if (m_eViewDirection == VD_INVALID)
    {
        fprintf(stderr, PROGNAME ", line %d: Missing view direction\n",
                m_iLine);
        exit(1);
    }

    if (m_vFrames.size() == 0)
    {
        fprintf(stderr, PROGNAME ", line %d: Animation must have at least "
                "one frame\n", m_iLine);
        exit(1);
    }

    // Check all frames.
    for (FrameIterator iter = m_vFrames.begin(); iter != m_vFrames.end(); iter++)
    {
        (*iter).Check();
    }
}

AnimationGroupKey::AnimationGroupKey()
{
    m_sName = "";
    m_iTileSize = -1;
}

AnimationGroupKey::AnimationGroupKey(const std::string &name, int tilesize)
{
    m_sName = name;
    m_iTileSize = tilesize;
}

AnimationGroupKey::AnimationGroupKey(const AnimationGroupKey &agk)
{
    m_sName = agk.m_sName;
    m_iTileSize = agk.m_iTileSize;
}

AnimationGroupKey &AnimationGroupKey::operator=(const AnimationGroupKey &agk)
{
    if (this != &agk)
    {
        m_sName = agk.m_sName;
        m_iTileSize = agk.m_iTileSize;
    }
    return *this;
}

AnimationGroup::AnimationGroup()
{
    for (int i = 0; i < 4; i++)
        m_aAnims[i] = NULL;
}

AnimationGroup::AnimationGroup(const AnimationGroup &ag)
{
    for (int i = 0; i < 4; i++)
        m_aAnims[i] = ag.m_aAnims[i];
}

AnimationGroup &AnimationGroup::operator=(const AnimationGroup &ag)
{
    if (this != &ag)
    {
        for (int i = 0; i < 4; i++)
            m_aAnims[i] = ag.m_aAnims[i];
    }
    return *this;
}

void AnimationGroup::InsertAnimation(const Animation &an)
{
    if (m_aAnims[an.m_eViewDirection] != NULL)
    {
        fprintf(stderr, PROGNAME ": Animation at line %d could not "
                "be added, the animation at line %d was already used\n",
                an.m_iLine, m_aAnims[an.m_eViewDirection]->m_iLine);
        exit(1);
    }

    m_aAnims[an.m_eViewDirection] = &an;
}

void AnimationGroup::Check()
{
    int length = -1;
    int index = -1;
    for (int i = 0; i < 4; i++)
    {
        if (m_aAnims[i] == NULL)
            continue;

        if (length < 0)
        {
            length = m_aAnims[i]->m_vFrames.size();
            index = i;
        }

        // Length of animation should be the same in every view.
        if (length != static_cast<int>(m_aAnims[i]->m_vFrames.size()))
        {
            fprintf(stderr, PROGNAME ": Animation at line %d has a different "
                    "frame count than the animation at line %d\n",
                    m_aAnims[index]->m_iLine, m_aAnims[i]->m_iLine);
            exit(1);
        }
    }
}

// vim: et sw=4 ts=4 sts=4

