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
    UBYTE *colorRegs;       /* Raw color registers (RGB triplets - R,G,B,R,G,B,...) */
    ULONG *colorTable;      /* ARGB color values if available */
    BOOL allocated;         /* Whether we allocated memory for colorRegs */
    BOOL hasTransparency;   /* Whether palette has transparency */
    UBYTE transparentColor; /* Index of transparent color in palette */
} ImgPalette;

typedef struct
{
    UBYTE *data;          /* Pixel data (RGB or RGBA) */
    ULONG width;          /* Width of the image */
    ULONG height;         /* Height of the image */
    BOOL hasTransparency; /* Whether image has transparency */
} PNGImage;

/* Initialize a palette structure */
void initImgPalette(ImgPalette *palette);

/* Free resources allocated for a palette */
void freeImgPalette(ImgPalette *palette);

/* Find the MUI window object for a given MUI object */
Object *getWindowObject(Object *obj);

#endif /* GRAPHICS_COMMON_H */