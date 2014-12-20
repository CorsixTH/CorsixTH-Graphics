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

%{
#include <cstdio>
#include <string>
#include "ast.h"
#include "scanparse.h"
#include "tokens.h"

int giLine = 1;
std::string gsFilename;
%}

%x IN_STRING
%x IN_COMMENT

%option noyywrap
%option nounput
%option noinput

%option never-interactive
%option nounistd


%%

"{"             { yylval.m_iLine = giLine; return CURLY_OPEN; }
"}"             { yylval.m_iLine = giLine; return CURLY_CLOSE; }
"="             { yylval.m_iLine = giLine; return EQUAL; }
";"             { yylval.m_iLine = giLine; return SEMICOL; }

"left"          { yylval.m_iLine = giLine; return LEFTKW; }
"top"           { yylval.m_iLine = giLine; return TOPKW; }
"width"         { yylval.m_iLine = giLine; return WIDTHKW; }
"height"        { yylval.m_iLine = giLine; return HEIGHTKW; }
"base"          { yylval.m_iLine = giLine; return BASE_IMGKW; }
"recolour"      { yylval.m_iLine = giLine; return RECOLOURKW; }
"layer"         { yylval.m_iLine = giLine; return LAYERKW; }
"alpha"         { yylval.m_iLine = giLine; return ALPHAKW; }
"hor_flip"      { yylval.m_iLine = giLine; return HOR_FLIPKW; }
"vert_flip"     { yylval.m_iLine = giLine; return VERT_FLIPKW; }
"x_offset"      { yylval.m_iLine = giLine; return X_OFFSETKW; }
"y_offset"      { yylval.m_iLine = giLine; return Y_OFFSETKW; }
"frame"         { yylval.m_iLine = giLine; return FRAMEKW; }
"element"       { yylval.m_iLine = giLine; return ELEMENTKW; }
"north"         { yylval.m_iLine = giLine; return NORTHKW; }
"east"          { yylval.m_iLine = giLine; return EASTKW; }
"south"         { yylval.m_iLine = giLine; return SOUTHKW; }
"west"          { yylval.m_iLine = giLine; return WESTKW; }
"tile_size"     { yylval.m_iLine = giLine; return TILE_SIZEKW; }
"view"          { yylval.m_iLine = giLine; return VIEWKW; }
"sound"         { yylval.m_iLine = giLine; return SOUNDKW; }
"animation"     { yylval.m_iLine = giLine; return ANIMATIONKW; }
"diplay_if"     { yylval.m_iLine = giLine; return DISPLAYKW; }

"0"             { yylval.m_iLine = giLine;
                  yylval.m_iNumber = 0;
                  return NUMBER; }

[1-9][0-9]*     { yylval.m_iLine = giLine;
                  yylval.m_iNumber = atoi(yytext);
                  return NUMBER; }
[-][1-9][0-9]*  { yylval.m_iLine = giLine;
                  yylval.m_iNumber = atoi(yytext);
                  return NUMBER; }

\"              { yylval.m_sText = "";
                  yylval.m_iLine = giLine;
                  BEGIN(IN_STRING); }

<IN_STRING>\"   { BEGIN(INITIAL);
                  return STRING; }

<IN_STRING>.    { yylval.m_sText += yytext; }

"//"            { BEGIN(IN_COMMENT); }

<IN_COMMENT>\n  { BEGIN(INITIAL);
                  giLine++; }

<IN_COMMENT>.   { }

" "             { }

\t              { }

\n              { giLine++; }

.               { fprintf(stderr, "Unrecognized character encountered at line %d\n", giLine);
                  exit(1); }

<IN_COMMENT><<EOF>> { BEGIN(INITIAL); }
<IN_STRING><<EOF>>  { BEGIN(INITIAL); }

%%

void SetupScanner(const char *fname, FILE *new_file)
{
    if (new_file == NULL || fname == NULL)
    {
        new_file = stdin;
    }
    yyrestart(new_file);
    BEGIN(INITIAL);

    gsFilename = (fname == NULL) ? "<stdin>" : fname;
    giLine = 1;
}

