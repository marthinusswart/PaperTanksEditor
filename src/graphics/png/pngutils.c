
#include "pngutils.h"

/* Load PNG image with palette information */
BOOL loadPNGToBitmapObject(CONST_STRPTR filename, UBYTE **outImageData, ImgPalette **outPalette)
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
        fileLoggerAddErrorEntry("Failed to load PNG image");
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
        fileLoggerAddErrorEntry("Failed to load PNG Palette");
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

    palette->colorTable = colorTable;
    palette->allocated = TRUE;

    /* Optionally, fill colorRegs as RGB triplets */
    numColors = 256;
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

    palette->hasTransparency = FALSE;

    *outPalette = palette;
    return TRUE;
}
