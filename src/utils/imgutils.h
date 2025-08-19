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
#include "windowlogger.h"
#include "filelogger.h"

BOOL loadILBMToBitmapObject(CONST_STRPTR filename, UBYTE **outImageData);
BOOL loadILBMToBitmapObject2(CONST_STRPTR filename);
BOOL loadILBMToBitmapObject3(CONST_STRPTR filename, UBYTE **outImageData);
BOOL loadILBMToBitmapObjectRGB(CONST_STRPTR filename, UBYTE **outImageData);
BOOL loadILBMToBitmapObjectRGB2(CONST_STRPTR filename);

#endif