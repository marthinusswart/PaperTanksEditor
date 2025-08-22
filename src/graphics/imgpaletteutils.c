/*
 * Implementation of Image Palette Utilities for AmigaOS 3.1
 */

#include "imgpaletteutils.h"

BOOL loadPaletteFromPNGImage(PNGImage *image, Img8BitPalette **img8BitPalette)
{
    if (!image || !img8BitPalette)
        return FALSE;

    Img8BitPalette *palette = (Img8BitPalette *)malloc(sizeof(Img8BitPalette));
    if (!palette)
    {
        fileLoggerAddErrorEntry("loadPaletteFromPNGImage: Failed to allocate memory for Img8BitPalette");
        *img8BitPalette = NULL;
        return FALSE;
    }

    // Initialize palette to black
    for (int i = 0; i < 255; i++)
    {
        palette->red8[i] = 0;
        palette->green8[i] = 0;
        palette->blue8[i] = 0;
    }
    palette->hasTransparency = FALSE;
    palette->transparentR8 = 0;
    palette->transparentG8 = 0;
    palette->transparentB8 = 0;
    palette->colorsAllocated = 1;

    *img8BitPalette = palette;
    return TRUE;
}
