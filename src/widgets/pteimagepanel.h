#ifndef PTEIMAGEPANEL_H
#define PTEIMAGEPANEL_H

/*** Include stuff ***/

#include <exec/types.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <libraries/mui.h>
#include <clib/alib_protos.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/exec.h>
#include <string.h>
#include <stdarg.h>

#include "../utils/windowlogger.h"
#include "../utils/filelogger.h"
#include "../../include/SDI_compiler.h"
#include "../../include/SDI_hook.h"

/*** MUI Defines ***/

#define MUIC_PTEImagePanel "PTEImagePanel.mcc"
#define PTEImagePanelObject MUI_NewObject(MUIC_PTEImagePanel

struct PTEImagePanelData
{
    ULONG barPos;
    ULONG nbarPos;
    ULONG ibarPos;
};

extern struct MUI_CustomClass *createPTEImagePanelClass(void);

#endif