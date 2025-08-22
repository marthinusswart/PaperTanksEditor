
/*
 * Implementation of Image Palette Utilities for AmigaOS 3.1
 */

#include "imgpaletteutils.h"

/* Forward Declares */
void assignSystemColorsToPalette(Img8BitPalette *palette);
/********************/

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

    assignSystemColorsToPalette(palette);

    *img8BitPalette = palette;
    return TRUE;
}

// Assign first 8 palette colors to common system colors
void assignSystemColorsToPalette(Img8BitPalette *palette)
{
    if (!palette)
        return;

    // System colors: black, white, red, green, blue, yellow, cyan, magenta
    // clang-format off
    palette->red8[0] = 0;       palette->green8[0] = 0;     palette->blue8[0] = 0; // Black
    palette->red8[1] = 255;     palette->green8[1] = 255;   palette->blue8[1] = 255; // White
    palette->red8[2] = 255;     palette->green8[2] = 0;     palette->blue8[2] = 0; // Red
    palette->red8[3] = 0;       palette->green8[3] = 255;   palette->blue8[3] = 0; // Green
    palette->red8[4] = 0;       palette->green8[4] = 0;     palette->blue8[4] = 255; // Blue
    palette->red8[5] = 255;     palette->green8[5] = 255;   palette->blue8[5] = 0; // Yellow
    palette->red8[6] = 0;       palette->green8[6] = 255;   palette->blue8[6] = 255; // Cyan
    palette->red8[7] = 255;     palette->green8[7] = 0;     palette->blue8[7] = 255; // Magenta
    // clang-format on
}
