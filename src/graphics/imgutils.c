#include "imgutils.h"
#include "imgpaletteutils.h"
#include "graphics.h"

BOOL loadILBMToBitmapObject2(CONST_STRPTR filename)
{
    UBYTE **outImageData = NULL;
    return loadILBMToBitmapObject(filename, &outImageData, NULL);
}

BOOL loadILBMToBitmapObjectRGB2(CONST_STRPTR filename)
{
    UBYTE **outImageData = NULL;
    return loadILBMToBitmapObjectRGB(filename, &outImageData);
}

BOOL loadILBMToBitmapObject(CONST_STRPTR filename, UBYTE **outImageData, ILBMPalette *outPalette)
{
    Object *dto = NULL;
    struct BitMap *bmp = NULL;
    struct BitMapHeader *bmHeader = NULL;
    struct Screen *scr = NULL;
    struct ColorMap *colorMap = NULL;
    ULONG *colorTable = NULL;
    UBYTE *colorRegs = NULL;
    ULONG numColors = 0;
    char logMessage[256];

    snprintf(logMessage, sizeof(logMessage), "Loading ILBM (improved) from %s\n", filename);
    fileLoggerAddEntry(logMessage);

    if (!outImageData)
    {
        snprintf(logMessage, sizeof(logMessage), "Failed to load ILBM: outImageData is NULL for %s\n", filename);
        fileLoggerAddEntry(logMessage);
        return FALSE;
    }
    *outImageData = NULL;

    /* Initialize palette if provided */
    if (outPalette)
    {
        initILBMPalette(outPalette);
    }

    // Open with specific attributes to get color information
    dto = NewDTObject(filename,
                      DTA_GroupID, GID_PICTURE,
                      PDTA_Remap, FALSE, // Don't remap colors
                      TAG_DONE);

    if (!dto)
    {
        snprintf(logMessage, sizeof(logMessage), "Failed to load DTO for %s\n", filename);
        fileLoggerAddEntry(logMessage);
        return FALSE;
    }

    // Trigger layout/rendering
    DoDTMethod(dto, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);

    // Extract bitmap and all possible color information
    struct BitMapHeader *bmhd = NULL;
    ULONG modeid = 0;

    GetDTAttrs(dto,
               PDTA_BitMap, &bmp,
               PDTA_BitMapHeader, &bmhd,
               PDTA_NumColors, &numColors,
               PDTA_CRegs, &colorRegs,       // Raw color registers (RGB triplets)
               PDTA_ColorTable, &colorTable, // ARGB color values
               PDTA_ModeID, &modeid,         // Display mode ID
               TAG_END);

    ULONG bmp_width = 0, bmp_height = 0, bmp_depth = 0;

    if (bmhd)
    {
        bmp_width = bmhd->bmh_Width;
        bmp_height = bmhd->bmh_Height;
        bmp_depth = bmhd->bmh_Depth;

        // Calculate number of colors from bit depth if not provided
        if (numColors == 0 && bmp_depth > 0)
        {
            numColors = 1 << bmp_depth; // 2^depth
        }
    }

    if (!bmp)
    {
        sprintf(logMessage, "Bitmap not loaded from %s\n", filename);
        fileLoggerAddEntry(logMessage);
        DisposeDTObject(dto);
        return FALSE;
    }

    // Get color map from the default public screen (A1200 with AGA chipset)
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
        DisposeDTObject(dto);
        return FALSE;
    }

    if (bmp)
    {
        // Use sprintf instead of snprintf (not available in AmigaOS 3.1)
        sprintf(logMessage, "Bitmap loaded: %lux%lu pixels, %lu bitplanes, %lu colors, ModeID: 0x%08lx\n",
                bmp_width, bmp_height, bmp_depth, numColors, modeid);
        fileLoggerAddEntry(logMessage);
    }

    // Define a fallback palette for common Amiga colors
    // This will be used if we can't get the colors from the file
    struct
    {
        UBYTE r, g, b;
    } defaultPalette[16] = {
        {0, 0, 0},       // 0: Black
        {255, 255, 255}, // 1: White
        {255, 0, 0},     // 2: Red
        {0, 255, 0},     // 3: Green
        {0, 0, 255},     // 4: Blue
        {255, 255, 0},   // 5: Yellow
        {0, 255, 255},   // 6: Cyan
        {255, 0, 255},   // 7: Magenta
        {170, 170, 170}, // 8: Light gray
        {102, 102, 102}, // 9: Dark gray
        {255, 128, 128}, // 10: Light red
        {128, 255, 128}, // 11: Light green
        {128, 128, 255}, // 12: Light blue
        {255, 128, 0},   // 13: Orange
        {128, 0, 128},   // 14: Purple
        {128, 128, 0}    // 15: Brown
    };

    // Log color information if available
    logPaletteInfo(colorRegs, numColors, colorTable);

    // Save palette information if requested
    if (outPalette)
    {
        outPalette->numColors = numColors;
        outPalette->colorTable = colorTable; // Just store the reference (owned by datatype)

        // Make a deep copy of the color registers if available
        if (colorRegs && numColors > 0)
        {
            ULONG colorRegsSize = numColors * 3; // RGB triplets
            outPalette->colorRegs = (UBYTE *)malloc(colorRegsSize);

            if (outPalette->colorRegs)
            {
                // Copy the color registers
                memcpy(outPalette->colorRegs, colorRegs, colorRegsSize);
                outPalette->allocated = TRUE; // We allocated this memory

                sprintf(logMessage, "Copied %lu colors (%lu bytes) to output palette\n",
                        numColors, colorRegsSize);
                fileLoggerAddEntry(logMessage);
            }
            else
            {
                sprintf(logMessage, "Failed to allocate memory for color registers\n");
                fileLoggerAddEntry(logMessage);
                // Continue without copying - just use the reference
                outPalette->colorRegs = colorRegs;
                outPalette->allocated = FALSE;
            }
        }
        else
        {
            // No color registers available
            outPalette->colorRegs = NULL;
            outPalette->allocated = FALSE;
        }
    }

    // Convert bitmap to byte array for MUI 3.8 ImageObject
    UBYTE *imageData = NULL;
    ULONG imageDataSize;

    // Calculate size needed for chunky pixel data
    imageDataSize = bmp_width * bmp_height;
    imageData = (UBYTE *)malloc(imageDataSize);

    if (imageData)
    {
        // Clear memory
        memset(imageData, 0, imageDataSize);

        struct RastPort tempRP;
        UBYTE *pixelPtr = imageData;
        ULONG x, y;

        // Initialize temporary RastPort
        InitRastPort(&tempRP);
        tempRP.BitMap = bmp;

        // Create a mapping table from source pens to system pens
        UBYTE penMap[256];

        // Create the pen mapping, optimizing for duplicate colors and using ColorMap for accurate matches
        UBYTE uniqueColorCount = createPenMapping(penMap, colorRegs, numColors, TRUE, colorMap);

        // Log our pen mappings
        logPenMappingInfo(penMap, colorRegs, numColors);

        // Copy pen mapping to output palette if requested
        if (outPalette)
        {
            memcpy(outPalette->penMap, penMap, 256);
            sprintf(logMessage, "Copied pen mapping to output palette (uniqueColorCount: %u)\n", uniqueColorCount);
            fileLoggerAddEntry(logMessage);
        }

        // Read pixels from bitmap and convert to chunky format with pen remapping
        for (y = 0; y < bmp_height; y++)
        {
            for (x = 0; x < bmp_width; x++)
            {
                // Read pixel from bitmap (returns pen number)
                UBYTE sourcePen = ReadPixel(&tempRP, x, y);

                // Apply pen mapping
                UBYTE mappedPen = penMap[sourcePen];

                *pixelPtr++ = mappedPen;
            }
        }

        *outImageData = imageData; // Store pointer to free later
        sprintf(logMessage, "Bitmap converted to chunky data with pen remapping (%lu bytes)\n", imageDataSize);
        fileLoggerAddEntry(logMessage);
    }
    else
    {
        sprintf(logMessage, "Failed to allocate memory for image conversion\n");
        fileLoggerAddEntry(logMessage);
    }

    // Clean up
    if (scr)
    {
        UnlockPubScreen(NULL, scr);
    }
    DisposeDTObject(dto);

    // If image data is NULL, this is a failure - clean up palette
    if (!imageData && outPalette)
    {
        freeILBMPalette(outPalette);
    }

    return (imageData != NULL);
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
                    UBYTE red = ((rgb4 >> 8) & 0xF) * 17; // Scale 0-15 to 0-255
                    UBYTE green = ((rgb4 >> 4) & 0xF) * 17;
                    UBYTE blue = (rgb4 & 0xF) * 17;

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
