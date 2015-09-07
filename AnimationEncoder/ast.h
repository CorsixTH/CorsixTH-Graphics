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

//! @file ast.h Abstract Syntax Tree (AST) classes for storing parsed information.

#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <map>
#include <string>

class Output;

//! Number of bytes in a sprite block excluding the actual sprite data.
static const int SPRITE_NON_DATA_SIZE = 10;

//! Enumeration listing the fields of the data structures.
enum FieldNumber
{
    FN_NOTHING,    ///< Empty storage.

    FE_TOP,        ///< Top
    FE_LEFT,       ///< Left
    FE_WIDTH,      ///< Width
    FE_HEIGHT,     ///< Height
    FE_XOFFSET,    ///< X offset
    FE_YOFFSET,    ///< Y offset
    FE_IMAGE,      ///< Image name
    FE_RECOLOUR,   ///< Recolour image name
    FE_RECOLLAYER, ///< Recolour layer field
    FE_DISPLAY,    ///< Display condition (layer-class and layer-id)
    FE_ALPHA,      ///< Amount of alpha
    FE_HORFLIP,    ///< Horizontal flip
    FE_VERTFLIP,   ///< Vertical flip

    AF_SOUND,      ///< Sound index

    AP_TILESIZE,   ///< Tile size
    AP_VIEW,       ///< View direction

    FN_NUMBER_ENTRIES, ///< Number of fields.
};

//! Temporary storage of an assigned field.
class FieldStorage
{
public:
    FieldStorage();
    FieldStorage(int number, int value, int line);
    FieldStorage(int number, int key, int value, int line);
    FieldStorage(int number, std::string text, int line);
    FieldStorage(const FieldStorage &fs);
    FieldStorage &operator=(const FieldStorage &fs);

    int m_iNumber;       ///< Field number being stored.

    int m_iKey;          ///< Key to store.
    int m_iValue;        ///< Value to store.
    std::string m_sText; ///< Text to store.

    int m_iLine;         ///< Line number with the assignment.
};

//! Pair of integer numbers.
class PairInt
{
public:
    PairInt() : m_iKey(FN_NOTHING), m_iValue(0) { }

    PairInt(int key, int value) : m_iKey(key), m_iValue(value) { }

    PairInt(const PairInt &pi) : m_iKey(pi.m_iKey), m_iValue(pi.m_iValue) { }

    PairInt &operator=(const PairInt &pi)
    {
        if (this != &pi) {
            m_iKey = pi.m_iKey;
            m_iValue = pi.m_iValue;
        }
        return *this;
    }

    int m_iKey;   ///< Key of the pair.
    int m_iValue; ///< Value of the pair.
};

//! An element (a sprite) in a frame.
class FrameElement
{
public:
    FrameElement();
    FrameElement(int line);
    FrameElement(const FrameElement &fe);
    FrameElement &operator=(const FrameElement &fe);

    void SetProperties(const std::vector<FieldStorage> &fields);

    void Check();
    bool WriteSprite(Output *pOut, int *iXoffset, int *iYoffset) const;

    int m_iLine;                  ///< Line number of the frame element.

    int m_iTop;                   ///< Top edge of the uncropped sprite in the image (by default, \c 0).
    int m_iLeft;                  ///< Left edge of the uncropped sprite in the image (by default, \c 0).
    int m_iWidth;                 ///< Horizontal size of the uncropped sprite (\c -1 means the entire image).
    int m_iHeight;                ///< Vertical size of the uncropped sprite (\c -1 means the entire image).
    int m_iXoffset;               ///< Horizontal adjustment of the top-left edge of the sprite.
    int m_iYoffset;               ///< Vertical adjustment of the top-left edge of the sprite.
    std::string m_sBaseImage;     ///< Full-colour RGBA base image.
    std::string m_sRecolourImage; ///< Name of overlay image (if set).
    unsigned char m_aNumber[256]; ///< Layer number of the recolouring.

    PairInt m_oDisplay;           ///< Display condition (layer class + layer id) of the element.
    int m_iAlpha;                 ///< Amount of alpha.
    bool m_bHorFlip;              ///< Whether to flip the sprite horizontally.
    bool m_bVertFlip;             ///< Whether to flip the sprite vertically.
};

//! A frame in an animation.
class AnimationFrame
{
public:
    AnimationFrame();
    AnimationFrame(int line);
    AnimationFrame(const AnimationFrame &af);
    AnimationFrame &operator=(const AnimationFrame &af);

    void SetProperty(const FieldStorage &field);
    void SetElements(const std::vector<FrameElement> &elements);

    void Check();

    int m_iLine;                           ///< Line number defining the frame.

    int m_iSound;                          ///< Sound index, if not \c -1.
    std::vector<FrameElement> m_vElements; ///< Frame elements.
};

//! Animation view direction.
enum ViewDirection
{
    VD_NORTH,   ///< Viewing north.
    VD_EAST,    ///< Viewing east.
    VD_SOUTH,   ///< Viewing south.
    VD_WEST,    ///< Viewing west.

    VD_INVALID, ///< Invalid direction (not specified).
};

//! An animation.
class Animation
{
public:
    Animation();
    Animation(int line, const std::string &name);
    Animation(const Animation &an);
    Animation &operator=(const Animation &an);

    void SetProperties(const std::vector<FieldStorage> &fields);
    void SetFrames(const std::vector<AnimationFrame> &frames);

    void Check();

    int m_iLine;                           ///< Line number defining the animation.
    std::string m_sName;                   ///< Name of the animation.
    int m_iTileSize;                       ///< Tile size.
    ViewDirection m_eViewDirection;        ///< Animation view direction.
    std::vector<AnimationFrame> m_vFrames; ///< Frames of the animation.
};

//! An animation with a name, a tile size, and 1 to 4 viewing directions.
class AnimationGroup
{
public:
    AnimationGroup();
    AnimationGroup(const AnimationGroup &ag);
    AnimationGroup &operator=(const AnimationGroup &ag);

    void InsertAnimation(const Animation &an);
    void Check();

    const Animation *m_aAnims[4]; ///< Collected animations (if not \c null).
};

//! Key of an animation group.
class AnimationGroupKey
{
public:
    AnimationGroupKey();
    AnimationGroupKey(const std::string &name, int tilesize);
    AnimationGroupKey(const AnimationGroupKey &agk);
    AnimationGroupKey &operator=(const AnimationGroupKey &agk);

    std::string m_sName;    ///< Name of the animations.
    int m_iTileSize;        ///< Tile size.
};

//! Comparator for storing animation groups in a set.
/*!
    @param agk1 First key to compare.
    @param agk2 Second key to compare.
    @return \c true iff first key should be put before the second key.
 */
inline bool operator<(const AnimationGroupKey &agk1, const AnimationGroupKey &agk2)
{
    if (agk1.m_iTileSize != agk2.m_iTileSize)
        return agk1.m_iTileSize < agk2.m_iTileSize;

    return agk1.m_sName < agk2.m_sName;
}

typedef std::vector<FrameElement>::iterator ElementIterator;
typedef std::vector<FrameElement>::const_iterator ElementConstIterator;

typedef std::vector<AnimationFrame>::iterator FrameIterator;
typedef std::vector<AnimationFrame>::const_iterator FrameConstIterator;

typedef std::vector<Animation>::iterator AnimationIterator;
typedef std::vector<Animation>::const_iterator AnimationConstIterator;

typedef std::map<AnimationGroupKey, AnimationGroup>::iterator GroupIterator;

extern std::map<AnimationGroupKey, AnimationGroup> g_mapAnimGroups;

#endif

// vim: et sw=4 ts=4 sts=4
