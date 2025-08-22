/*
 * Basic PNG image loading utilities for AmigaOS 3.1
 * Simple internal PNG decoder for PaperTanksEditor
 */

#ifndef IMGPNGUTILS_H
#define IMGPNGUTILS_H

#include <exec/types.h>
#include <graphics/view.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "../../utils/filelogger.h"
#include "../graphics.h"
#include "../imgpaletteutils.h"
#include "imgpngfilters.h"
#include "../../utils/zlib/zlibutils.h"
#include "../../../external/zlib/puff.h"

/* PNG chunk type identifiers */
#define PNG_CHUNK_IHDR 0x49484452 /* "IHDR" */
#define PNG_CHUNK_PLTE 0x504C5445 /* "PLTE" */
#define PNG_CHUNK_IDAT 0x49444154 /* "IDAT" */
#define PNG_CHUNK_IEND 0x49454E44 /* "IEND" */
#define PNG_CHUNK_TRNS 0x74524E53 /* "tRNS" */

/* PNG color types */
#define PNG_COLOR_TYPE_GRAYSCALE 0
#define PNG_COLOR_TYPE_RGB 2
#define PNG_COLOR_TYPE_PALETTE 3
#define PNG_COLOR_TYPE_GRAYSCALE_ALPHA 4
#define PNG_COLOR_TYPE_RGBA 6

/* PNG header structure */
typedef struct
{
    ULONG width;
    ULONG height;
    UBYTE bitDepth;
    UBYTE colorType;
    UBYTE compressionMethod;
    UBYTE filterMethod;
    UBYTE interlaceMethod;
} PNGHeader;

BOOL loadPNGToBitmapObject2(CONST_STRPTR filename, UBYTE **outImageData, ImgPalette **outPalette);

#endif /* IMGPNGUTILS_H */
