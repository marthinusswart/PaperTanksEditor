#ifndef PTEIMAGEPANEL_H
#define PTEIMAGEPANEL_H
/**
 * @file pteimagepanel.h
 * @brief Header for the PTEImagePanel MUI custom class for AmigaOS.
 *
 * This module provides the interface and data structures for the PTEImagePanel,
 * a custom MUI class designed to display 24-bit PNG images and other image formats
 * in an AmigaOS GUI application. It supports drawing borders, handling transparency,
 * and direct rendering to the screen using Amiga graphics APIs.
 *
 * Features:
 *   - Custom MUI class creation and dispatcher
 *   - Image data and palette management
 *   - Border drawing and margin support
 *   - PNG transparency handling
 *   - Logging via filelogger and windowlogger
 *   - Utility macros for Amiga/MUI compatibility
 *
 * See pteimagepanel.c for implementation details, including:
 *   - Class instantiation and attribute parsing
 *   - Drawing logic for images and borders
 *   - Transparency and palette support
 *   - Integration with AmigaOS graphics and MUI APIs
 */

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

/*** MUI Defines ***/

#define MUIC_PTEImagePanel "PTEImagePanel.mcc"

#ifndef _rp
#define _rp(obj) (((struct MUIP_Draw *)msg)->rp)
#endif

#ifndef STACKARGS
#ifdef __VBCC__
#define STACKARGS
#endif
#endif

extern struct MUI_CustomClass *pteImagePanelClass;
#define PTEImagePanelObject NewObject(pteImagePanelClass->mcc_Class, NULL

/* clang-format off */

#define PTEA_BorderColor     0x30400001
#define PTEA_DrawBorder      0x30400002
#define PTEA_BorderMargin    0x30400003
#define PTEA_ImageData       0x30400004
#define PTEA_ImageWidth      0x30400005
#define PTEA_ImageHeight     0x30400006
#define PTEA_HasTransparency 0x30400007
#define PTEA_IsPNG           0x3040000A

/* clang-format on */

struct PTEImagePanelData
{
    BYTE borderColor;
    BOOL drawBorder;
    WORD borderMargin;
    UBYTE *imageData;
    WORD imageWidth;
    WORD imageHeight;
    BOOL isPNG;
    BOOL hasTransparency;
};

extern void initializePTEImagePanel(void);
extern struct MUI_CustomClass *createPTEImagePanelClass(void);

#endif