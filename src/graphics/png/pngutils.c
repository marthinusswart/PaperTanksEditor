#include <stdlib.h>
#include <string.h>
#include "pngutils.h"
#include "../../../external/lodepng/lodepng.h"

/* Load PNG image with palette information */
BOOL loadPNGToBitmapObject2(CONST_STRPTR filename, UBYTE **outImageData, ImgPalette **outPalette)
{
    if (!filename || !outImageData || !outPalette)
        return FALSE;

    unsigned char *image = NULL;
    unsigned width = 0, height = 0;
    unsigned error = 0;

    /* Decode PNG to RGBA */
    error = lodepng_decode32_file(&image, &width, &height, filename);
    if (error || !image)
    {
        *outImageData = NULL;
        *outPalette = NULL;
        return FALSE;
    }

    /* Allocate output image data */
    *outImageData = image;

    /* Create palette from image data (extract unique colors, up to 256) */
    ImgPalette *palette = (ImgPalette *)malloc(sizeof(ImgPalette));
    if (!palette)
    {
        free(image);
        *outImageData = NULL;
        *outPalette = NULL;
        return FALSE;
    }
    initImgPalette(palette);

    /* Build palette: scan RGBA pixels, collect unique colors */
    ULONG maxColors = 256;
    ULONG numColors = 0;
    ULONG *colorTable = (ULONG *)malloc(maxColors * sizeof(ULONG));
    if (!colorTable)
    {
        free(image);
        free(palette);
        *outImageData = NULL;
        *outPalette = NULL;
        return FALSE;
    }

    for (ULONG i = 0; i < width * height; i++)
    {
        ULONG rgba = ((ULONG)image[i * 4 + 0] << 24) | ((ULONG)image[i * 4 + 1] << 16) |
                     ((ULONG)image[i * 4 + 2] << 8) | ((ULONG)image[i * 4 + 3]);
        BOOL found = FALSE;
        for (ULONG c = 0; c < numColors; c++)
        {
            if (colorTable[c] == rgba)
            {
                found = TRUE;
                break;
            }
        }
        if (!found && numColors < maxColors)
        {
            colorTable[numColors++] = rgba;
        }
    }

    palette->numColors = numColors;
    palette->colorTable = colorTable;
    palette->allocated = TRUE;

    /* Optionally, fill colorRegs as RGB triplets */
    palette->colorRegs = (UBYTE *)malloc(numColors * 3);
    if (palette->colorRegs)
    {
        for (ULONG c = 0; c < numColors; c++)
        {
            ULONG rgba = colorTable[c];
            palette->colorRegs[c * 3 + 0] = (UBYTE)((rgba >> 24) & 0xFF); // R
            palette->colorRegs[c * 3 + 1] = (UBYTE)((rgba >> 16) & 0xFF); // G
            palette->colorRegs[c * 3 + 2] = (UBYTE)((rgba >> 8) & 0xFF);  // B
        }
    }

    /* Transparency: find if any color has alpha < 255 */
    palette->hasTransparency = FALSE;
    for (ULONG c = 0; c < numColors; c++)
    {
        ULONG rgba = colorTable[c];
        UBYTE alpha = (UBYTE)(rgba & 0xFF);
        if (alpha < 255)
        {
            palette->hasTransparency = TRUE;
            palette->transparentColor = (UBYTE)c;
            break;
        }
    }

    *outPalette = palette;
    return TRUE;
}
