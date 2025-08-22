#ifndef MAINMENU_H
#define MAINMENU_H

#include <exec/types.h>
#include <proto/muimaster.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <clib/alib_protos.h>
#include <libraries/mui.h>

#include <proto/dos.h>

#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <stdio.h>

#include <proto/graphics.h>

enum
{
    MEN_PROJECT = 1,
    MEN_ABOUT,
    MEN_QUIT,
    MEN_EDIT,
    MEN_CUT,
    MEN_COPY,
    MEN_PASTE
};

/* clang-format off */
static struct NewMenu MainMenuData[] =
    {
    
        
        {NM_TITLE, (char *)"File", NULL, 0, 0, (APTR)MEN_EDIT},
            {NM_ITEM, (char *)"Load", (char *)"L", 0, 0, (APTR)MEN_CUT},
            {NM_ITEM, (char *)"Save", (char *)"S", 0, 0, (APTR)MEN_COPY},
            {NM_ITEM, NM_BARLABEL, NULL, 0, 0, (APTR)0},
            {NM_ITEM, (char *)"Quit", (char *)"Q", 0, 0, (APTR)MEN_QUIT},        

        {NM_TITLE, (char *)"Help", (char *)"P", 0, 0, (APTR)MEN_PROJECT},
            {NM_ITEM, (char *)"About", (char *)"?", 0, 0, (APTR)MEN_ABOUT},

        {NM_END, NULL, NULL, 0, 0, (APTR)0},

    
};
/* clang-format on */

#endif