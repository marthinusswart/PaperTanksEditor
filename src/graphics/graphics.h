/*
 * Common graphics structures and definitions for AmigaOS 3.1
 */

#ifndef GRAPHICS_COMMON_H
#define GRAPHICS_COMMON_H

#include <exec/types.h>
#include <graphics/view.h>
#include <libraries/mui.h>

/* Image palette structure */
typedef struct
{
    UBYTE red8[255];
    UBYTE green8[255];
    UBYTE blue8[255];
    BOOL hasTransparency;
    UBYTE transparentR8;
    UBYTE transparentG8;
    UBYTE transparentB8;
    UBYTE colorsAllocated;
} Img8BitPalette;

typedef struct
{
    UBYTE *data;          /* Pixel data (RGB or RGBA) */
    ULONG width;          /* Width of the image */
    ULONG height;         /* Height of the image */
    BOOL hasTransparency; /* Whether image has transparency */
} PNGImage;

/* Initialize a palette structure */
void initImg8BitPalette(Img8BitPalette *palette);

/* Free resources allocated for a palette */
void freeImg8BitPalette(Img8BitPalette *palette);

/* Find the MUI window object for a given MUI object */
Object *getWindowObject(Object *obj);

#endif /* GRAPHICS_COMMON_H */