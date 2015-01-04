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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ast.h"
#include "scanparse.h"
#include "storage.h"

/** Perform type checking of the parsed input. */
static void Check()
{
    AnimationIterator iter;
    GroupIterator grp;

    // 1. Individual animations.
    for (iter = g_vAnimations.begin(); iter != g_vAnimations.end(); iter++)
    {
        (*iter).Check();
    }

    // 2. Collect animations into groups.
    g_mapAnimGroups.clear();
    for (iter = g_vAnimations.begin(); iter != g_vAnimations.end(); iter++)
    {
        AnimationGroupKey agk((*iter).m_sName, (*iter).m_iTileSize);
        grp = g_mapAnimGroups.find(agk);
        if (grp == g_mapAnimGroups.end())
        {
            AnimationGroup ag;
            std::pair<AnimationGroupKey, AnimationGroup> p(agk, ag);
            grp = g_mapAnimGroups.insert(p).first;
        }

        (*grp).second.InsertAnimation(*iter);
    }

    // 3. Check animation groups.
    for (grp = g_mapAnimGroups.begin(); grp != g_mapAnimGroups.end(); grp++)
    {
        (*grp).second.Check();
    }
}


int main(int iArgc, char *pArgv[])
{
    // Perform argument processing.
    if (iArgc == 1)
    {
        SetupScanner(NULL, NULL);
    }
    else if (iArgc != 3 || strcmp(pArgv[1], "-h") == 0 || strcmp(pArgv[1], "--help") == 0)
    {
        printf("Usage: encode <animation-file> <output-file>\n");
        exit(1);
    }

    FILE *pInfile = fopen(pArgv[1], "r");
    SetupScanner(pArgv[1], pInfile);

    // Parse input file.
    int iRet = yyparse();
    if (pInfile != NULL) fclose(pInfile);

    if (iRet != 0)
    {
        exit(iRet);
    }

    // Check input, generate output.
    Check();
    Encode(pArgv[2]);

    exit(0);
}

// vim: et sw=4 ts=4 sts=4

