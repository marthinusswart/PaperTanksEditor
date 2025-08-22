/*
 * Common graphics structures and definitions for AmigaOS 3.1
 */

#ifndef GRAPHICS_COMMON_H
#define GRAPHICS_COMMON_H

#include <exec/types.h>
#include <graphics/view.h>
#include <libraries/mui.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/filelogger.h"

/* Image palette structure */
typedef struct
{
    UBYTE red8[256];
    UBYTE green8[256];
    UBYTE blue8[256];
    BOOL hasTransparency;
    UBYTE transparentR8;
    UBYTE transparentG8;
    UBYTE transparentB8;
    UWORD transparentIndex;
    UWORD colorsAllocated;
} Img8BitPalette;

typedef struct
{
    UBYTE *data;          /* Pixel data (RGB or RGBA) */
    ULONG width;          /* Width of the image */
    ULONG height;         /* Height of the image */
    BOOL hasTransparency; /* Whether image has transparency */
} PNGImage;

/* Initialize a palette structure */
void initImg8BitPalette(Img8BitPalette **palette);

/* Free resources allocated for a palette */
void freeImg8BitPalette(Img8BitPalette *palette);

/* Find the MUI window object for a given MUI object */
Object *getWindowObject(Object *obj);

/* Find the ViewPort for a given MUI object */
BOOL getScreenViewport(Object *obj, Object *win, struct ViewPort **outVp);

#endif /* GRAPHICS_COMMON_H */