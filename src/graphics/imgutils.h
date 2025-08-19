#ifndef IMGUTILS_H
#define IMGUTILS_H

#include <datatypes/pictureclass.h>
#include <proto/datatypes.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <string.h>
#include <proto/muimaster.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <graphics/view.h>
#include "../utils/windowlogger.h"
#include "../utils/filelogger.h"
#include "graphics.h"

/* Load ILBM image with palette information */
BOOL loadILBMToBitmapObject(CONST_STRPTR filename, UBYTE **outImageData, ILBMPalette **outPalette);
BOOL loadILBMToBitmapObject2(CONST_STRPTR filename);
BOOL loadILBMToBitmapObjectRGB(CONST_STRPTR filename, UBYTE **outImageData);
BOOL loadILBMToBitmapObjectRGB2(CONST_STRPTR filename);
BOOL loadILBMToBitmapObjectRGB3(CONST_STRPTR filename, UBYTE **outImageData, ILBMPalette **outPalette);

#endif
