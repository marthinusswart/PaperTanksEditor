#include "imgutils.h"

BOOL loadILBMToBitmapObject2(CONST_STRPTR filename)
{
    UBYTE **outImageData = NULL;
    return loadILBMToBitmapObject(filename, &outImageData);
}

BOOL loadILBMToBitmapObject(CONST_STRPTR filename, UBYTE **outImageData)
{
    Object *dto = NULL;
    struct BitMap *bmp = NULL;
    struct BitMapHeader *bmHeader = NULL;
    char logMessage[256];

    // snprintf(logMessage, sizeof(logMessage), "Loading ILBM from %s\n", filename);
    // loggerAddEntry(logMessage);

    // if (!outImageData)
    // {
    //     snprintf(logMessage, sizeof(logMessage), "Failed to load ILBM: outImageData is NULL for %s\n", filename);
    //     loggerAddEntry(logMessage);
    //     return FALSE;
    // }
    // *outImageData = NULL;

    // dto = NewDTObject(filename, DTA_GroupID, GID_PICTURE, TAG_DONE);

    // if (!dto)
    // {
    //     snprintf(logMessage, sizeof(logMessage), "Failed to load DTO for %s\n", filename);
    //     loggerAddEntry(logMessage);
    //     return FALSE;
    // }

    // // Trigger layout/rendering
    // DoDTMethod(dto, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);

    // // Extract bitmap - AmigaOS 3.1 compatible method
    // // Get the bitmap through the object's instance data
    // struct BitMapHeader *bmhd;
    // GetDTAttrs(dto, PDTA_BitMap, &bmp, PDTA_BitMapHeader, &bmhd, TAG_END);

    // ULONG bmp_width, bmp_height, bmp_depth;

    // if (bmhd)
    // {
    //     bmp_width = bmhd->bmh_Width;
    //     bmp_height = bmhd->bmh_Height;
    //     bmp_depth = bmhd->bmh_Depth;
    // }

    // if (!bmp)
    // {
    //     sprintf(logMessage, "Bitmap not loaded from %s\n", filename);
    //     loggerAddEntry(logMessage);
    //     DisposeDTObject(dto);
    //     return FALSE;
    // }

    // if (bmp)
    // {
    //     // Use sprintf instead of snprintf (not available in AmigaOS 3.1)
    //     sprintf(logMessage, "Bitmap loaded: %lux%lu pixels\n", bmp_width, bmp_height);
    //     loggerAddEntry(logMessage);
    // }

    // // Convert bitmap to byte array for MUI 3.8 ImageObject
    // UBYTE *imageData = NULL;
    // ULONG imageDataSize;

    // // Calculate size needed for chunky pixel data
    // imageDataSize = bmp_width * bmp_height;
    // imageData = (UBYTE *)malloc(imageDataSize);

    // if (imageData)
    // {
    //     // Manually clear the memory since MEMF_CLEAR may not be available
    //     memset(imageData, 0, imageDataSize);

    //     struct RastPort tempRP;
    //     UBYTE *pixelPtr = imageData;
    //     ULONG x, y;

    //     // Initialize temporary RastPort
    //     InitRastPort(&tempRP);
    //     tempRP.BitMap = bmp;

    //     // Read pixels from bitmap and convert to chunky format
    //     for (y = 0; y < bmp_height; y++)
    //     {
    //         for (x = 0; x < bmp_width; x++)
    //         {
    //             // Read pixel from bitmap (returns pen number)
    //             UBYTE pixel = ReadPixel(&tempRP, x, y);
    //             *pixelPtr++ = pixel;
    //         }
    //     }

    //     *outImageData = imageData; // Store pointer to free later
    //     sprintf(logMessage, "Bitmap converted to chunky data (%lu bytes)\n", imageDataSize);
    //     loggerAddEntry(logMessage);
    //     DisposeDTObject(dto);
    // }
    // else
    // {
    //     sprintf(logMessage, "Failed to allocate memory for image conversion\n");
    //     loggerAddEntry(logMessage);
    //     DisposeDTObject(dto);
    // }

    return TRUE;
}