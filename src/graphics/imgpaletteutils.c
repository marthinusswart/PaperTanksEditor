
/*
 * Implementation of Image Palette Utilities for AmigaOS 3.1
 */

#include "imgpaletteutils.h"

/* Forward Declares */
int countUniqueColorsInPNGImage32(PNGImage *image, Img8BitPalette *palette);
/********************/

/* 32 bit has no pallete so will need to approximate one */
BOOL loadPaletteFromPNGImage32(PNGImage *image, Img8BitPalette **img8BitPalette)
{
    if (!image || !img8BitPalette)
    {
        fileLoggerAddErrorEntry("loadPaletteFromPNGImage: Invalid image or palette pointer");
        return FALSE;
    }

    Img8BitPalette *palette = NULL;
    initImg8BitPalette(&palette);
    if (!palette)
    {
        fileLoggerAddErrorEntry("loadPaletteFromPNGImage: Failed to allocate memory for Img8BitPalette");
        *img8BitPalette = NULL;
        return FALSE;
    }

    int colorCount = countUniqueColorsInPNGImage32(image, palette);

    char debugMsg[64];
    sprintf(debugMsg, "Unique RGBA color count: %ld", (LONG)colorCount);
    fileLoggerAddDebugEntry(debugMsg);

    *img8BitPalette = palette;
    return TRUE;
}

// Counts unique RGBA colors in a PNGImage (expects 32-bit RGBA: 4 bytes per pixel)
int countUniqueColorsInPNGImage32(PNGImage *image, Img8BitPalette *palette)
{
    if (!image || !image->data)
    {
        fileLoggerAddErrorEntry("countUniqueColorsInPNGImage: Invalid image or data");
        return 0;
    }

    ULONG pixelCount = image->width * image->height;
    ULONG *uniqueColors = (ULONG *)AllocMem(4096 * sizeof(ULONG), MEMF_PUBLIC);
    int currentColorIndex = 8; // Start after the system colors

    if (!uniqueColors)
    {
        fileLoggerAddErrorEntry("countUniqueColorsInPNGImage: Failed to allocate memory for uniqueColors");
        return 0;
    }
    int uniqueCount = 0;

    for (ULONG i = 0; i < pixelCount; i++)
    {
        // ULONG rgb = 0;
        ULONG rgb = ((ULONG)image->data[i * 4 + 0] << 16) |
                    ((ULONG)image->data[i * 4 + 1] << 8) |
                    ((ULONG)image->data[i * 4 + 2]);

        int found = 0;
        for (int j = 0; j < uniqueCount; j++)
        {
            if (uniqueColors[j] == rgb)
            {
                found = 1;
                break;
            }
            else if (currentColorIndex < 256)
            {
                palette->red8[currentColorIndex] = (UBYTE)image->data[i * 4 + 0];
                palette->green8[currentColorIndex] = (UBYTE)image->data[i * 4 + 1];
                palette->blue8[currentColorIndex] = (UBYTE)image->data[i * 4 + 2];
                currentColorIndex++;
                palette->colorsAllocated = currentColorIndex;
            }
        }
        if (!found && uniqueCount < 4096)
        {
            uniqueColors[uniqueCount++] = rgb;
        }
    }

    char debugMsg[64];
    sprintf(debugMsg, "Unique RGBA color count: %ld", (LONG)uniqueCount);
    fileLoggerAddDebugEntry(debugMsg);

    FreeMem(uniqueColors, 4096 * sizeof(ULONG));
    return uniqueCount;
}
