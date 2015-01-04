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
#include "ast.h"
#include "scanparse.h"

std::vector<Animation> g_vAnimations;
%}


%token CURLY_OPEN CURLY_CLOSE EQUAL SEMICOL
%token<m_iLine> LEFTKW TOPKW WIDTHKW HEIGHTKW BASE_IMGKW RECOLOURKW LAYERKW
%token<m_iLine> ALPHAKW HOR_FLIPKW VERT_FLIPKW

%token<m_iLine> X_OFFSETKW Y_OFFSETKW ANIMATIONKW FRAMEKW TILE_SIZEKW VIEWKW SOUNDKW
%token<m_iLine> NORTHKW WESTKW SOUTHKW EASTKW ELEMENTKW DISPLAYKW

%token<m_iNumber> NUMBER
%token<m_sText> STRING

%type<m_oAnimation> Animation
%type<m_oFrame> AnimationFrame
%type<m_vFrames> AnimationFrames
%type<m_oElement> FrameElement
%type<m_vElements> FrameElements
%type<m_vFields> AnimationProperties ElementFields
%type<m_oField> AnimationProperty Direction FrameProperty ElementField

%start Program


%%

Program : /* empty */
          {
              g_vAnimations.clear();
          }
        | Program Animation
          {
              g_vAnimations.push_back($2);
          }
        ;

Animation : ANIMATIONKW STRING CURLY_OPEN AnimationProperties AnimationFrames CURLY_CLOSE
            {
                Animation an($1, $2);
                an.SetProperties($4);
                an.SetFrames($5);
                $$ = an;
            }
          ;

AnimationProperties : AnimationProperty
                      {
                          std::vector<FieldStorage> fss;
                          $$ = fss;
                          $$.push_back($1);
                      }
                    | AnimationProperties AnimationProperty
                      {
                          $$ = $1;
                          $$.push_back($2);
                      }
                    ;

AnimationProperty : TILE_SIZEKW EQUAL NUMBER SEMICOL
                    {
                        FieldStorage fs(AP_TILESIZE, $3, $1);
                        $$ = fs;
                    }
                  | VIEWKW EQUAL Direction SEMICOL
                    {
                        $$ = $3;
                        $$.m_iLine = $1;
                    }
                  ;

Direction : NORTHKW
            {
                FieldStorage fs(AP_VIEW, VD_NORTH, $1);
                $$ = fs;
            }
          | EASTKW
            {
                FieldStorage fs(AP_VIEW, VD_EAST, $1);
                $$ = fs;
            }
          | SOUTHKW
            {
                FieldStorage fs(AP_VIEW, VD_SOUTH, $1);
                $$ = fs;
            }
          | WESTKW
            {
                FieldStorage fs(AP_VIEW, VD_WEST, $1);
                $$ = fs;
            }
          ;

AnimationFrames : AnimationFrame
                  {
                      std::vector<AnimationFrame> elements;
                      $$ = elements;
                      $$.push_back($1);
                  }
                | AnimationFrames AnimationFrame
                  {
                      $$ = $1;
                      $$.push_back($2);
                  }
                ;

AnimationFrame : FRAMEKW CURLY_OPEN FrameProperty FrameElements CURLY_CLOSE
                 {
                     AnimationFrame af($1);
                     af.SetProperty($3);
                     af.SetElements($4);
                     $$ = af;
                 }
               ;

FrameProperty : /* empty */
                {
                    FieldStorage fs;
                    $$ = fs;
                }
              | SOUNDKW EQUAL NUMBER SEMICOL
                {
                    FieldStorage fs(AF_SOUND, $3, $1);
                    $$ = fs;
                }
              ;

FrameElements : FrameElement
                {
                    std::vector<FrameElement> elements;
                    $$ = elements;
                    $$.push_back($1);
                }
              | FrameElements FrameElement
                {
                    $$ = $1;
                    $$.push_back($2);
                }
              ;

FrameElement : ELEMENTKW CURLY_OPEN ElementFields CURLY_CLOSE
               {
                   FrameElement fe($1);
                   fe.SetProperties($3);
                   $$ = fe;
               }
             ;

ElementFields : ElementField
                {
                    std::vector<FieldStorage> fss;
                    $$ = fss;
                    $$.push_back($1);
                }
              | ElementFields ElementField
                {
                    $$ = $1;
                    $$.push_back($2);
                }
              ;

ElementField : TOPKW EQUAL NUMBER SEMICOL
               {
                   FieldStorage fs(FE_TOP, $3, $1);
                   $$ = fs;
               }
             | LEFTKW EQUAL NUMBER SEMICOL
               {
                   FieldStorage fs(FE_LEFT, $3, $1);
                   $$ = fs;
               }
             | WIDTHKW EQUAL NUMBER SEMICOL
               {
                   FieldStorage fs(FE_WIDTH, $3, $1);
                   $$ = fs;
               }
             | HEIGHTKW EQUAL NUMBER SEMICOL
               {
                   FieldStorage fs(FE_HEIGHT, $3, $1);
                   $$ = fs;
               }
             | X_OFFSETKW EQUAL NUMBER SEMICOL
               {
                   FieldStorage fs(FE_XOFFSET, $3, $1);
                   $$ = fs;
               }
             | Y_OFFSETKW EQUAL NUMBER SEMICOL
               {
                   FieldStorage fs(FE_YOFFSET, $3, $1);
                   $$ = fs;
               }
             | BASE_IMGKW EQUAL STRING SEMICOL
               {
                   FieldStorage fs(FE_IMAGE, $3, $1);
                   $$ = fs;
               }
             | RECOLOURKW EQUAL STRING SEMICOL
               {
                   FieldStorage fs(FE_RECOLOUR, $3, $1);
                   $$ = fs;
               }
             | LAYERKW NUMBER EQUAL NUMBER SEMICOL
               {
                   FieldStorage fs(FE_RECOLLAYER, $2, $4, $1);
                   $$ = fs;
               }
             | DISPLAYKW NUMBER EQUAL NUMBER SEMICOL
               {
                   FieldStorage fs(FE_DISPLAY, $2, $4, $1);
                   $$ = fs;
               }
             | ALPHAKW EQUAL NUMBER SEMICOL
               {
                   FieldStorage fs(FE_ALPHA, $3, $1);
                   $$ = fs;
               }
             | HOR_FLIPKW
               {
                   FieldStorage fs(FE_HORFLIP, 1, $1);
                   $$ = fs;
               }
             | VERT_FLIPKW
               {
                   FieldStorage fs(FE_VERTFLIP, 1, $1);
                   $$ = fs;
               }
             ;

%%

void yyerror(const char *msg)
{
    fprintf(stderr, "Parse error at line %d: %s\n", yylval.m_iLine, msg);
    exit(1);
}

// vim: et sts=4 sw=4
