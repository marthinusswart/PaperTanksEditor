#ifndef PTECOLORPALETTEPANEL_H
#define PTECOLORPALETTEPANEL_H

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
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <proto/graphics.h>
#include <clib/muimaster_protos.h>

#include "../utils/windowlogger.h"
#include "../utils/filelogger.h"
#include "../../include/SDI_compiler.h"
#include "../../include/SDI_hook.h"
#include "../graphics/graphics.h"
#include "widgetconstants.h"

/*** MUI Defines ***/

#define MUIC_PTEColorPalettePanel "PTEColorPalettePanel.mcc"

#ifndef _rp
#define _rp(obj) (((struct MUIP_Draw *)msg)->rp)
#endif

#ifndef STACKARGS
#ifdef __VBCC__
#define STACKARGS
#endif
#endif

extern struct MUI_CustomClass *pteColorPalettePanelClass;
#define PTEColorPalettePanelObject NewObject(pteColorPalettePanelClass->mcc_Class, NULL

typedef struct
{
    BYTE borderColor;
    BOOL drawBorder;
    WORD borderMargin;
    struct Img8BitPalette *colorPalette;
} PTEColorPalettePanelData;

extern void initializePTEColorPalettePanel(void);
extern struct MUI_CustomClass *createPTEColorPalettePanelClass(void);

#endif
