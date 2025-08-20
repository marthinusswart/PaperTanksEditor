/*
 * Common graphics structures and definitions for AmigaOS 3.1
 */

#ifndef GRAPHICS_COMMON_H
#define GRAPHICS_COMMON_H

#include <exec/types.h>
#include <graphics/view.h>

/* Image palette structure */
typedef struct
{
    ULONG numColors;   /* Number of colors in the palette */
    UBYTE *colorRegs;  /* Raw color registers (RGB triplets - R,G,B,R,G,B,...) */
    ULONG *colorTable; /* ARGB color values if available */
    UBYTE penMap[256]; /* Mapping from source pens to system pens */
    BOOL allocated;    /* Whether we allocated memory for colorRegs */
} ImgPalette;

/* Initialize a palette structure */
void initImgPalette(ImgPalette *palette);

/* Free resources allocated for a palette */
void freeImgPalette(ImgPalette *palette);

#endif /* GRAPHICS_COMMON_H */
