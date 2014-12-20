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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include "ast.h"
#include "storage.h"

static std::map<EncodedSprite, int> g_mapSprites; ///< All encoded sprites.
static int g_iNumberWrittenFrames; ///< Number of frames in the file.
static int g_iTotalElements; ///< Total number of sprite elements.
static int g_iTotalSpriteSize; ///< Total size of all sprites.

DataBlock::DataBlock()
{
    m_iUsed = 0;
    m_pNext = NULL;
}

bool DataBlock::Full()
{
    return m_iUsed == BUF_SIZE;
}

void DataBlock::Add(unsigned char byte)
{
    assert(m_iUsed < BUF_SIZE);
    buffer[m_iUsed++] = byte;
}

void DataBlock::Write(FILE *handle)
{
    if (fwrite(buffer, 1, m_iUsed, handle) != (size_t)m_iUsed)
    {
        fprintf(stderr, "Writing output failed!\n");
        exit(1);
    }
}

Output::Output()
{
    m_pFirst = NULL;
    m_pLast = NULL;
}

Output::~Output()
{
    DataBlock *pBlk = m_pFirst;
    while (pBlk != NULL)
    {
        DataBlock *pBlk2 = pBlk->m_pNext;
        delete pBlk;
        pBlk = pBlk2;
    }
}

void Output::Uint8(unsigned char byte)
{
    if (m_pLast == NULL || m_pLast->Full())
    {
        DataBlock *pBlk = new DataBlock();
        if (m_pLast == NULL)
        {
            m_pFirst = pBlk;
            m_pLast = pBlk;
        }
        else
        {
            m_pLast->m_pNext = pBlk;
            m_pLast = pBlk;
        }
    }
    m_pLast->Add(byte);
}

void Output::Uint16(int iValue)
{
    Uint8(iValue & 0xFF);
    Uint8((iValue >> 8) & 0xFF);
}

void Output::Uint32(unsigned int iValue)
{
    Uint8(iValue & 0xff);
    Uint8((iValue >> 8) & 0xFF);
    Uint8((iValue >> 16) & 0xFF);
    Uint8((iValue >> 24) & 0xFF);
}

void Output::String(const std::string &str)
{
    const char *txt = str.c_str();
    int length = strlen(txt);
    if (length > 255)
    {
        fprintf(stderr, "Warning: String \"%s\" is too long, writing the first 255 characters\n", txt);
        length = 255;
    }
    Uint8(length);
    while (length > 0)
    {
        Uint8(*txt);
        txt++;
        length--;
    }
}

/**
 * Reserve some space at the end of the output file.
 * @param iSize Length of the space to reserve.
 * @return Address of the space in the file.
 */
int Output::Reserve(int iSize)
{
    // Count current size.
    int iLength = 0;
    DataBlock *pBlk = m_pFirst;
    while (pBlk != NULL)
    {
        iLength += pBlk->m_iUsed;
        pBlk = pBlk->m_pNext;
    }

    // Add 'size' bytes as reserved space.
    for (int i = 0; i < iSize; i++)
    {
        Uint8(0);
    }
    return iLength;
}

/**
 * Overwrite a byte at a given address in the file.
 * @param iAddress Address to overwrite.
 * @param iValue (New) value to write.
 */
void Output::Write(int iAddress, unsigned char iValue)
{
    int iOffset = 0;
    DataBlock *pBlk = m_pFirst;
    while (pBlk != NULL && iOffset + pBlk->m_iUsed < iAddress)
    {
        iOffset += pBlk->m_iUsed;
        pBlk = pBlk->m_pNext;
    }
    assert(pBlk != NULL);
    iAddress -= iOffset;
    assert(iAddress >= 0 && iAddress < BUF_SIZE);
    pBlk->buffer[iAddress] = iValue;
}

void Output::Write32(int iAddress, unsigned int iValue)
{
    Write(iAddress, iValue & 0xFF);
    Write(iAddress + 1, (iValue >> 8) & 0xFF);
    Write(iAddress + 2, (iValue >> 16) & 0xFF);
    Write(iAddress + 3, (iValue >> 24) & 0xFF);
}

void Output::Write(const char *fname)
{
    FILE *handle = fopen(fname, "wb");
    DataBlock *pBlk = m_pFirst;
    while (pBlk != NULL)
    {
        pBlk->Write(handle);
        pBlk = pBlk->m_pNext;
    }
    fclose(handle);
}

int Output::GetSize()
{
    int iTotal = 0;
    DataBlock *pBlk = m_pFirst;
    while (pBlk != NULL)
    {
        iTotal += pBlk->m_iUsed;
        pBlk = pBlk->m_pNext;
    }
    return iTotal;
}

unsigned char *Output::GetData()
{
    int iSize = GetSize();
    unsigned char *pData = static_cast<unsigned char *>(malloc(iSize));

    unsigned char *pDest = pData;
    DataBlock *pBlk = m_pFirst;
    while (pBlk != NULL)
    {
        memcpy(pDest, pBlk->buffer, pBlk->m_iUsed);
        pDest += pBlk->m_iUsed;
        pBlk = pBlk->m_pNext;
    }
    return pData;
}


EncodedSprite::EncodedSprite()
{
    m_pData = NULL;
    m_iSize = 0;
    m_bOwned = false;
}

EncodedSprite::EncodedSprite(unsigned char *address, int size)
{
    m_bOwned = false;
    CopyData(address, size);
}

EncodedSprite::EncodedSprite(const EncodedSprite &es)
{
    if (es.m_bOwned)
    {
        m_bOwned = false;
        CopyData(es.m_pData, es.m_iSize);
    }
    else
    {
        m_bOwned = false;
        m_pData = es.m_pData;
        m_iSize = es.m_iSize;
    }
}

EncodedSprite &EncodedSprite::operator=(const EncodedSprite &es)
{
    if (this != &es)
    {
        if (m_bOwned)
            free(m_pData);

        m_bOwned = false;
        if (es.m_bOwned)
        {
            CopyData(es.m_pData, es.m_iSize);
        }
        else
        {
            m_pData = es.m_pData;
            m_iSize = es.m_iSize;
        }
    }
    return *this;
}

EncodedSprite::~EncodedSprite()
{
    if (m_bOwned)
        free(m_pData);
}

/**
 * Take ownership of the  provided data.
 * @param data Data to take.
 * @param size Size of the taken data.
 */
void EncodedSprite::TakeData(unsigned char *data, int size)
{
    if (m_bOwned)
        free(m_pData);

    m_bOwned = true;
    m_pData = data;
    m_iSize = size;
}

/**
 * Copy the data into the sprite.
 * @param data Pointer to the data to copy.
 * @param size Length of the data.
 */
void EncodedSprite::CopyData(const unsigned char *data, int size)
{
    if (m_bOwned)
        free(m_pData);

    unsigned char *pData = (unsigned char *)malloc(size);
    m_pData = static_cast<unsigned char *>(memcpy(pData, data, size));
    m_iSize = size;
    m_bOwned = true;
}

bool operator<(const EncodedSprite &es1, const EncodedSprite &es2)
{
    assert(es1.m_pData != NULL);
    assert(es2.m_pData != NULL);

    if (es1.m_iSize != es2.m_iSize)
        return es1.m_iSize < es2.m_iSize;

    for (int idx = 0; idx < es1.m_iSize; idx++)
        if (es1.m_pData[idx] != es2.m_pData[idx])
            return es1.m_pData[idx] < es2.m_pData[idx];

    return false;
}

class SpriteElement
{
public:
    SpriteElement()
    {
    }

    SpriteElement(const SpriteElement &se)
    {
        m_iSprite = se.m_iSprite;
        m_iXoffset = se.m_iXoffset;
        m_iYoffset = se.m_iYoffset;
        m_iLayerclass = se.m_iLayerclass;
        m_iLayerId = se.m_iLayerId;
        m_iFlags = se.m_iFlags;
    }

    SpriteElement &operator=(const SpriteElement &se)
    {
        if (this != &se)
        {
            m_iSprite = se.m_iSprite;
            m_iXoffset = se.m_iXoffset;
            m_iYoffset = se.m_iYoffset;
            m_iLayerclass = se.m_iLayerclass;
            m_iLayerId = se.m_iLayerId;
            m_iFlags = se.m_iFlags;
        }
        return *this;
    }

    int m_iSprite;
    int m_iXoffset;
    int m_iYoffset;
    int m_iLayerclass;
    int m_iLayerId;
    int m_iFlags;
};

static int EncodeSprite(const FrameElement &fe, int *iXoffset, int *iYoffset, Output *output)
{
    // Encode the sprite.
    Output out;
    if (!fe.WriteSprite(&out, iXoffset, iYoffset))
    {
        fprintf(stderr, "Warning: Sprite \"%s\" cannot be created, using sprite 0\n", fe.m_sBaseImage.c_str());
        return 0;
    }

    EncodedSprite oEncSprite;
    oEncSprite.TakeData(out.GetData(), out.GetSize());

    // Find sprite in written sprites.
    std::map<EncodedSprite, int>::iterator iter;
    iter = g_mapSprites.find(oEncSprite);
    if (iter != g_mapSprites.end())
        return (*iter).second;

    // Write sprite block.
    g_iTotalSpriteSize += oEncSprite.m_iSize - SPRITE_NON_DATA_SIZE; // Subtract header length.
    for (int idx = 0; idx < oEncSprite.m_iSize; idx++)
        output->Uint8(oEncSprite.m_pData[idx]);

    // Store sprite for future re-use.
    std::pair<EncodedSprite, int> p(oEncSprite, g_mapSprites.size());
    iter = g_mapSprites.insert(p).first;
    return (*iter).second;
}

/**
 * Encode the frames of the provided animation
 * @param an Animation with the frames to encode.
 * @param output Output stream to write to.
 * @return Number of the first frame written.
 */
static int EncodeFrames(const Animation &an, Output *output)
{
    int first_frame = g_iNumberWrittenFrames;

    for (FrameConstIterator fri = an.m_vFrames.begin(); fri != an.m_vFrames.end(); fri++)
    {
        const AnimationFrame &fr = *fri;

        // Encode the elements of the frame, storing the results temporarily.
        std::vector<SpriteElement> elements;
        for (ElementConstIterator ei = fr.m_vElements.begin(); ei != fr.m_vElements.end(); ei++)
        {
            SpriteElement se;
            se.m_iSprite = EncodeSprite(*ei, &se.m_iXoffset, &se.m_iYoffset, output);
            if ((*ei).m_oDisplay.m_iKey < 0)
            {
                se.m_iLayerclass = 0;
                se.m_iLayerId = 0;
            }
            else
            {
                se.m_iLayerclass = (*ei).m_oDisplay.m_iKey;
                se.m_iLayerId = (*ei).m_oDisplay.m_iValue;
            }
            se.m_iFlags = 0;
            if ((*ei).m_bVertFlip)    se.m_iFlags |= 0x1;
            if ((*ei).m_bHorFlip)     se.m_iFlags |= 0x2;
            if ((*ei).m_iAlpha == 50) se.m_iFlags |= 0x4;
            if ((*ei).m_iAlpha == 75) se.m_iFlags |= 0x8;

            elements.push_back(se);
        }

        // Encode the frame.
        output->Uint8('F');
        output->Uint8('R');
        output->Uint16((fr.m_iSound < 0) ? 0 : fr.m_iSound);
        output->Uint16(elements.size());
        for (std::vector<SpriteElement>::iterator iter = elements.begin(); iter != elements.end(); iter++)
        {
            output->Uint32((*iter).m_iSprite);
            output->Uint16((*iter).m_iXoffset);
            output->Uint16((*iter).m_iYoffset);
            output->Uint8((*iter).m_iLayerclass);
            output->Uint8((*iter).m_iLayerId);
            output->Uint16((*iter).m_iFlags);

            g_iTotalElements++;
        }

        g_iNumberWrittenFrames++;
    }
    return first_frame;
}

/**
 * Encode an animation group.
 * @param ag Animation group to encode.
 * @param output Output stream to write to.
 */
static void EncodeAnimationGroup(const AnimationGroup &ag, Output *output)
{
    unsigned int first_frames[4];

    // Find an animation.
    const Animation *an = NULL; // Find an animation.
    for (int idx = 0; idx < 4; idx++)
    {
        if (ag.m_aAnims[idx] == NULL)
            continue;

        an = ag.m_aAnims[idx];
        break;
    }
    if (an == NULL)
    {
        fprintf(stderr, "Animation group without animation, skipping it\n");
        return;
    }

    // Encode all frames
    for (int idx = 0; idx < 4; idx++)
    {
        if (ag.m_aAnims[idx] == NULL)
        {
            first_frames[idx] = 0xFFFFFFFF;
        }
        else
        {
            first_frames[idx] = EncodeFrames(*ag.m_aAnims[idx], output);
        }
    }

    // Output grouped animation.
    output->Uint8('C');
    output->Uint8('A');
    output->Uint16(an->m_iTileSize);
    output->Uint32(an->m_vFrames.size());
    output->String(an->m_sName);
    for (int idx = 0; idx < 4; idx++)
        output->Uint32(first_frames[idx]);
}

void Encode(const char *outFname)
{
    g_iNumberWrittenFrames = 0;
    g_iTotalElements = 0;
    g_mapSprites.clear();
    g_iTotalSpriteSize = 0;

    Output output;

    output.Uint8('C');
    output.Uint8('T');
    output.Uint8('H');
    output.Uint8('G');
    output.Uint16(512+1);
    output.Uint32(g_mapAnimGroups.size());
    int iAddrTotalFrames = output.Reserve(4);
    int iAddrTotalElements = output.Reserve(4);
    int iAddrTotalSprites = output.Reserve(4);
    int iAddrSumSpriteSize = output.Reserve(4);

    for (GroupIterator grp = g_mapAnimGroups.begin(); grp != g_mapAnimGroups.end(); grp++)
    {
        EncodeAnimationGroup((*grp).second, &output);
    }

    output.Write32(iAddrTotalFrames, g_iNumberWrittenFrames);
    output.Write32(iAddrTotalElements, g_iTotalElements);
    output.Write32(iAddrTotalSprites, g_mapSprites.size());
    output.Write32(iAddrSumSpriteSize, g_iTotalSpriteSize);

    output.Write(outFname);
}

// vim: et sw=4 ts=4 sts=4
