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

#ifndef STORAGE_H
#define STORAGE_H

#define BUF_SIZE    100000

/** Block of data in the output file. */
class DataBlock
{
public:
    DataBlock();

    bool Full();
    void Add(unsigned char byte);
    void Write(FILE *handle);

    unsigned char buffer[BUF_SIZE];
    int m_iUsed;
    DataBlock *m_pNext;
};

/** Output file. */
class Output
{
public:
    Output();
    ~Output();

    void Write(const char *fname);

    void Uint8(unsigned char byte);
    void Uint16(int val);
    void Uint32(unsigned int val);
    void String(const std::string &str);
    void Write(int address, unsigned char byte);
    void Write32(int address, unsigned int iVal);
    int Reserve(int size);

    int GetSize();
    unsigned char *GetData();

    DataBlock *m_pFirst;
    DataBlock *m_pLast;
};


class EncodedSprite
{
public:
    EncodedSprite();
    EncodedSprite(unsigned char *address, int size);
    EncodedSprite(const EncodedSprite &es);
    EncodedSprite &operator=(const EncodedSprite &es);
    ~EncodedSprite();

    void CopyData(const unsigned char *data, int size);
    void TakeData(unsigned char *data, int size);

    unsigned char *m_pData; ///< Pointer to the data.
    int m_iSize;            ///< Length of the data.
    bool m_bOwned;          ///< Whether the object owns the data.
};

bool operator<(const EncodedSprite &es1, const EncodedSprite &e2);


void Encode(const char *outFname);

#endif

// vim: et sw=4 ts=4 sts=4
