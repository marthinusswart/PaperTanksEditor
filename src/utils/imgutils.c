#include "imgutils.h"

BOOL loadILBMToBitmapObject2(CONST_STRPTR filename)
{
    UBYTE **outImageData = NULL;
    return loadILBMToBitmapObject(filename, &outImageData);
}

BOOL loadILBMToBitmapObjectRGB2(CONST_STRPTR filename)
{
    UBYTE **outImageData = NULL;
    return loadILBMToBitmapObjectRGB(filename, &outImageData);
}

BOOL loadILBMToBitmapObject(CONST_STRPTR filename, UBYTE **outImageData)
{
    Object *dto = NULL;
    struct BitMap *bmp = NULL;
    struct BitMapHeader *bmHeader = NULL;
    char logMessage[256];

    snprintf(logMessage, sizeof(logMessage), "Loading ILBM from %s\n", filename);
    fileLoggerAddEntry(logMessage);

    if (!outImageData)
    {
        snprintf(logMessage, sizeof(logMessage), "Failed to load ILBM: outImageData is NULL for %s\n", filename);
        fileLoggerAddEntry(logMessage);
        return FALSE;
    }
    *outImageData = NULL;

    dto = NewDTObject(filename, DTA_GroupID, GID_PICTURE, TAG_DONE);

    if (!dto)
    {
        snprintf(logMessage, sizeof(logMessage), "Failed to load DTO for %s\n", filename);
        fileLoggerAddEntry(logMessage);
        return FALSE;
    }

    // Trigger layout/rendering
    DoDTMethod(dto, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);

    // Extract bitmap - AmigaOS 3.1 compatible method
    // Get the bitmap through the object's instance data
    struct BitMapHeader *bmhd;
    GetDTAttrs(dto, PDTA_BitMap, &bmp, PDTA_BitMapHeader, &bmhd, TAG_END);

    ULONG bmp_width, bmp_height, bmp_depth;

    if (bmhd)
    {
        bmp_width = bmhd->bmh_Width;
        bmp_height = bmhd->bmh_Height;
        bmp_depth = bmhd->bmh_Depth;
    }

    if (!bmp)
    {
        sprintf(logMessage, "Bitmap not loaded from %s\n", filename);
        fileLoggerAddEntry(logMessage);
        DisposeDTObject(dto);
        return FALSE;
    }

    if (bmp)
    {
        // Use sprintf instead of snprintf (not available in AmigaOS 3.1)
        sprintf(logMessage, "Bitmap loaded: %lux%lu pixels\n", bmp_width, bmp_height);
        fileLoggerAddEntry(logMessage);
    }

    // Convert bitmap to byte array for MUI 3.8 ImageObject
    UBYTE *imageData = NULL;
    ULONG imageDataSize;

    // Calculate size needed for chunky pixel data
    imageDataSize = bmp_width * bmp_height;
    imageData = (UBYTE *)malloc(imageDataSize);

    if (imageData)
    {
        // Manually clear the memory since MEMF_CLEAR may not be available
        memset(imageData, 0, imageDataSize);

        struct RastPort tempRP;
        UBYTE *pixelPtr = imageData;
        ULONG x, y;

        // Initialize temporary RastPort
        InitRastPort(&tempRP);
        tempRP.BitMap = bmp;

        // Read pixels from bitmap and convert to chunky format
        for (y = 0; y < bmp_height; y++)
        {
            for (x = 0; x < bmp_width; x++)
            {
                // Read pixel from bitmap (returns pen number)
                UBYTE pixel = ReadPixel(&tempRP, x, y);
                *pixelPtr++ = pixel;
            }
        }

        *outImageData = imageData; // Store pointer to free later
        sprintf(logMessage, "Bitmap converted to chunky data (%lu bytes)\n", imageDataSize);
        fileLoggerAddEntry(logMessage);
        DisposeDTObject(dto);
    }
    else
    {
        sprintf(logMessage, "Failed to allocate memory for image conversion\n");
        fileLoggerAddEntry(logMessage);
        DisposeDTObject(dto);
    }

    return TRUE;
}

BOOL loadILBMToBitmapObjectRGB(CONST_STRPTR filename, UBYTE **outImageData)
{
    Object *dto = NULL;
    struct BitMap *bmp = NULL;
    struct Screen *scr = NULL;
    struct ColorMap *colorMap = NULL;
    char logMessage[256];

    snprintf(logMessage, sizeof(logMessage), "Loading ILBM in RGB format from %s\n", filename);
    fileLoggerAddEntry(logMessage);

    if (!outImageData)
    {
        snprintf(logMessage, sizeof(logMessage), "Failed to load RGB ILBM: outImageData is NULL for %s\n", filename);
        fileLoggerAddEntry(logMessage);
        return FALSE;
    }
    *outImageData = NULL;

    dto = NewDTObject(filename, DTA_GroupID, GID_PICTURE, TAG_DONE);

    if (!dto)
    {
        snprintf(logMessage, sizeof(logMessage), "Failed to load DTO for %s\n", filename);
        fileLoggerAddEntry(logMessage);
        return FALSE;
    }

    // Trigger layout/rendering
    DoDTMethod(dto, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);

    // Extract bitmap and related information
    struct BitMapHeader *bmhd;
    GetDTAttrs(dto, PDTA_BitMap, &bmp, PDTA_BitMapHeader, &bmhd, TAG_END);

    ULONG bmp_width = 0, bmp_height = 0, bmp_depth = 0;

    if (bmhd)
    {
        bmp_width = bmhd->bmh_Width;
        bmp_height = bmhd->bmh_Height;
        bmp_depth = bmhd->bmh_Depth;
    }

    if (!bmp)
    {
        sprintf(logMessage, "Bitmap not loaded from %s\n", filename);
        fileLoggerAddEntry(logMessage);
        DisposeDTObject(dto);
        return FALSE;
    }

    // Get color map from the default public screen
    scr = LockPubScreen(NULL);
    if (scr)
    {
        colorMap = scr->ViewPort.ColorMap;
        sprintf(logMessage, "Using ColorMap from default public screen\n");
        fileLoggerAddEntry(logMessage);
    }
    else
    {
        sprintf(logMessage, "Warning: Could not lock public screen for ColorMap\n");
        fileLoggerAddEntry(logMessage);
    }

    sprintf(logMessage, "Bitmap loaded: %lux%lu pixels, %lu bitplanes\n", bmp_width, bmp_height, bmp_depth);
    fileLoggerAddEntry(logMessage);

    // Allocate memory for RGB image data (3 bytes per pixel)
    UBYTE *imageData = NULL;
    ULONG imageDataSize = bmp_width * bmp_height * 3; // 3 bytes per pixel (R,G,B)
    imageData = (UBYTE *)malloc(imageDataSize);

    if (imageData)
    {
        // Clear memory
        memset(imageData, 0, imageDataSize);

        struct RastPort tempRP;
        ULONG x, y;
        UBYTE *pixelPtr = imageData;

        // Initialize temporary RastPort
        InitRastPort(&tempRP);
        tempRP.BitMap = bmp;

        // Calculate number of colors based on bit depth
        ULONG numColors = 1 << bmp_depth; // 2^depth colors

        // Read pixels from bitmap and convert to RGB format
        for (y = 0; y < bmp_height; y++)
        {
            for (x = 0; x < bmp_width; x++)
            {
                // Read pixel from bitmap (returns pen number)
                UBYTE pixel = ReadPixel(&tempRP, x, y);
                
                if (colorMap && pixel < numColors)
                {
                    // Get RGB value from the color map (4 bits per component)
                    ULONG rgb4 = GetRGB4(colorMap, pixel);
                    
                    // Extract individual components (4 bits each)
                    UBYTE red   = ((rgb4 >> 8) & 0xF) * 17; // Scale 0-15 to 0-255
                    UBYTE green = ((rgb4 >> 4) & 0xF) * 17;
                    UBYTE blue  = (rgb4 & 0xF) * 17;
                    
                    *pixelPtr++ = red;   // Red
                    *pixelPtr++ = green; // Green
                    *pixelPtr++ = blue;  // Blue
                }
                else
                {
                    // Fallback grayscale based on pen value
                    UBYTE gray = (pixel < numColors) ? (pixel * 255 / (numColors - 1)) : 0;
                    *pixelPtr++ = gray; // Red
                    *pixelPtr++ = gray; // Green
                    *pixelPtr++ = gray; // Blue
                }
            }
        }

        *outImageData = imageData; // Store pointer to free later
        sprintf(logMessage, "Bitmap converted to RGB data (%lu bytes)\n", imageDataSize);
        fileLoggerAddEntry(logMessage);
    }
    else
    {
        sprintf(logMessage, "Failed to allocate memory for RGB image conversion\n");
        fileLoggerAddEntry(logMessage);
    }

    // Clean up
    if (scr)
    {
        UnlockPubScreen(NULL, scr);
    }
    DisposeDTObject(dto);
    return (imageData != NULL);
}