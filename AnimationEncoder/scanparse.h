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

//! @file scanparse.h Interface data and functions between scanner and parser productions.

#ifndef SCAN_PARSE_H
#define SCAN_PARSE_H

#include "ast.h"

struct ScannerData
{
    int m_iLine;
    int m_iNumber;
    std::string m_sText;
    Animation m_oAnimation;
    AnimationFrame m_oFrame;
    FrameElement m_oElement;
    FieldStorage m_oField;
    std::vector<AnimationFrame> m_vFrames;
    std::vector<FrameElement> m_vElements;
    std::vector<FieldStorage> m_vFields;
};

#define YYSTYPE ScannerData
extern YYSTYPE yylval;
int yylex();
int yyparse();
void yyerror(const char *msg);
void SetupScanner(const char *fname, FILE *new_file);

extern std::vector<Animation> g_vAnimations; ///< Loaded animations.

#endif
