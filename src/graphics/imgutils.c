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

BOOL loadILBMToBitmapObject(CONST_STRPTR filename, UBYTE **outImageData, ILBMPalette **outPalette)
{
    Object *dto = NULL;
    struct BitMap *bmp = NULL;
    ULONG *colorTable = NULL;
    UBYTE *colorRegs = NULL;
    ULONG numColors = 0;
    char logMessage[256];
    BOOL success = FALSE; // Track overall success for proper cleanup
    ILBMPalette *palette = NULL;

    snprintf(logMessage, sizeof(logMessage), "Loading ILBM (improved) from %s", filename);
    fileLoggerAddEntry(logMessage);

    if (!outImageData)
    {
        snprintf(logMessage, sizeof(logMessage), "Failed to load ILBM: outImageData is NULL for %s", filename);
        fileLoggerAddEntry(logMessage);
        return FALSE;
    }
    *outImageData = NULL;

    // Allocate and initialize palette if requested
    if (outPalette)
    {
        *outPalette = NULL; // Ensure null in case of error
        palette = (ILBMPalette *)malloc(sizeof(ILBMPalette));
        if (!palette)
        {
            snprintf(logMessage, sizeof(logMessage), "Failed to allocate memory for palette structure");
            fileLoggerAddEntry(logMessage);
            return FALSE;
        }
        initILBMPalette(palette);
        sprintf(logMessage, "Allocated new palette structure");
        fileLoggerAddEntry(logMessage);
    }

    sprintf(logMessage, "Using direct palette mode - no screen ColorMap needed");
    fileLoggerAddEntry(logMessage);

    // Open with specific attributes to get color information
    dto = NewDTObject(filename,
                      DTA_GroupID, GID_PICTURE,
                      PDTA_Remap, FALSE, // Don't remap colors
                      TAG_DONE);

    if (!dto)
    {
        snprintf(logMessage, sizeof(logMessage), "Failed to load DTO for %s", filename);
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
        sprintf(logMessage, "Bitmap not loaded from %s", filename);
        fileLoggerAddEntry(logMessage);
        DisposeDTObject(dto);
        return FALSE;
    }

    if (bmp)
    {
        // Use sprintf instead of snprintf (not available in AmigaOS 3.1)
        sprintf(logMessage, "Bitmap loaded: %lux%lu pixels, %lu bitplanes, %lu colors, ModeID: 0x%08lx",
                bmp_width, bmp_height, bmp_depth, numColors, modeid);
        fileLoggerAddEntry(logMessage);
    }

    // Save palette information if requested
    if (outPalette && palette)
    {
        palette->numColors = numColors;
        palette->colorTable = colorTable; // Just store the reference (owned by datatype)

        // Make a deep copy of the color registers if available
        if (colorRegs && numColors > 0)
        {
            ULONG colorRegsSize = numColors * 3; // RGB triplets
            palette->colorRegs = (UBYTE *)malloc(colorRegsSize);

            if (palette->colorRegs)
            {
                // Copy the color registers
                memcpy(palette->colorRegs, colorRegs, colorRegsSize);
                palette->allocated = TRUE; // We allocated this memory

                sprintf(logMessage, "Copied %lu colors (%lu bytes) to output palette",
                        numColors, colorRegsSize);
                fileLoggerAddEntry(logMessage);
            }
            else
            {
                sprintf(logMessage, "Failed to allocate memory for color registers");
                fileLoggerAddEntry(logMessage);
                // Continue without copying - just use the reference
                palette->colorRegs = colorRegs;
                palette->allocated = FALSE;
            }
        }
        else
        {
            // No color registers available
            palette->colorRegs = NULL;
            palette->allocated = FALSE;
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

        // Create a 1:1 identity mapping table - no remapping
        UBYTE penMap[256];
        for (ULONG i = 0; i < 256; i++)
        {
            penMap[i] = i; // Direct 1:1 mapping
        }

        sprintf(logMessage, "Using direct 1:1 color mapping (no remapping) for high-color display");
        fileLoggerAddEntry(logMessage);

        // Copy pen mapping to output palette if requested
        if (outPalette && palette)
        {
            memcpy(palette->penMap, penMap, 256);
            sprintf(logMessage, "Copied direct 1:1 pen mapping to output palette");
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
        sprintf(logMessage, "Bitmap converted to chunky data with pen remapping (%lu bytes)", imageDataSize);
        fileLoggerAddEntry(logMessage);

        // On success, assign the allocated palette to the output pointer
        if (outPalette && palette)
        {
            *outPalette = palette;
            sprintf(logMessage, "Assigned palette to output pointer");
            fileLoggerAddEntry(logMessage);
        }
    }
    else
    {
        sprintf(logMessage, "Failed to allocate memory for image conversion");
        fileLoggerAddEntry(logMessage);
    }

    // Set success flag based on image data allocation
    success = (imageData != NULL);

    // If image data is NULL, this is a failure - clean up palette
    if (!success)
    {
        if (palette)
        {
            sprintf(logMessage, "Image data allocation failed - cleaning up palette");
            fileLoggerAddEntry(logMessage);
            freeILBMPalette(palette);
            free(palette);
            if (outPalette)
            {
                *outPalette = NULL;
            }
        }
    }

    // Always clean up resources before returning
    DisposeDTObject(dto);

    return success;
}

BOOL loadILBMToBitmapObjectRGB(CONST_STRPTR filename, UBYTE **outImageData)
{
    Object *dto = NULL;
    struct BitMap *bmp = NULL;
    char logMessage[256];
    BOOL success = FALSE;
    UBYTE *imageData = NULL;
    UBYTE *colorRegs = NULL;
    ULONG numColors = 0;

    snprintf(logMessage, sizeof(logMessage), "Loading ILBM in RGB format from %s", filename);
    fileLoggerAddEntry(logMessage);

    if (!outImageData)
    {
        snprintf(logMessage, sizeof(logMessage), "Failed to load RGB ILBM: outImageData is NULL for %s", filename);
        fileLoggerAddEntry(logMessage);
        return FALSE;
    }
    *outImageData = NULL;

    dto = NewDTObject(filename,
                      DTA_GroupID, GID_PICTURE,
                      PDTA_Remap, FALSE, // Don't remap colors
                      TAG_DONE);

    if (!dto)
    {
        snprintf(logMessage, sizeof(logMessage), "Failed to load DTO for %s", filename);
        fileLoggerAddEntry(logMessage);
        return FALSE;
    }

    // Trigger layout/rendering
    DoDTMethod(dto, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);

    // Extract bitmap and related information
    struct BitMapHeader *bmhd;
    GetDTAttrs(dto,
               PDTA_BitMap, &bmp,
               PDTA_BitMapHeader, &bmhd,
               PDTA_NumColors, &numColors,
               PDTA_CRegs, &colorRegs, // Raw color registers (RGB triplets)
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
        sprintf(logMessage, "Bitmap not loaded from %s", filename);
        fileLoggerAddEntry(logMessage);
        DisposeDTObject(dto);
        return FALSE;
    }

    sprintf(logMessage, "Bitmap loaded: %lux%lu pixels, %lu bitplanes, %lu colors",
            bmp_width, bmp_height, bmp_depth, numColors);
    fileLoggerAddEntry(logMessage);

    // Allocate memory for RGB image data (3 bytes per pixel)
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

                if (colorRegs && pixel < numColors)
                {
                    // Get RGB values directly from the image's color registers
                    // colorRegs contains RGB triplets in sequence (R,G,B,R,G,B,...)
                    ULONG offset = pixel * 3;
                    UBYTE red = colorRegs[offset];
                    UBYTE green = colorRegs[offset + 1];
                    UBYTE blue = colorRegs[offset + 2];

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
        sprintf(logMessage, "Bitmap converted to RGB data using direct palette (%lu bytes)", imageDataSize);
        fileLoggerAddEntry(logMessage);
        success = TRUE;
    }
    else
    {
        sprintf(logMessage, "Failed to allocate memory for RGB image conversion");
        fileLoggerAddEntry(logMessage);
        success = FALSE;
    }

    // Always clean up resources before returning
    DisposeDTObject(dto);

    return success;
}
