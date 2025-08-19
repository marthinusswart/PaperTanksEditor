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
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <proto/graphics.h>

#include "../utils/windowlogger.h"
#include "../utils/filelogger.h"
#include "../../include/SDI_compiler.h"
#include "../../include/SDI_hook.h"

/*** MUI Defines ***/

#define MUIC_PTEImagePanel "PTEImagePanel.mcc"

#ifndef _rp
#define _rp(obj) (((struct MUIP_Draw *)msg)->rp)
#endif

extern struct MUI_CustomClass *pteImagePanelClass;
#define PTEImagePanelObject NewObject(pteImagePanelClass->mcc_Class, NULL

/* clang-format off */

#define PTEA_BorderColor    0x30400001
#define PTEA_DrawBorder     0x30400002
#define PTEA_BorderMargin   0x30400003
#define PTEA_ImageData      0x30400004
#define PTEA_ImageWidth     0x30400005
#define PTEA_ImageHeight    0x30400006
#define PTEA_EnableRGB      0x30400007

/* clang-format on */

struct PTEImagePanelData
{
    BYTE borderColor;
    BOOL drawBorder;
    WORD borderMargin;
    UBYTE *imageData;
    WORD imageWidth;
    WORD imageHeight;
    BOOL enableRGB;
};

extern void initializePTEImagePanel(void);
extern struct MUI_CustomClass *createPTEImagePanelClass(void);

#endif