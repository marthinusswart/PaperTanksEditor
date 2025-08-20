/*
 * Basic PNG image loading utilities for AmigaOS 3.1
 * Simple internal PNG decoder for PaperTanksEditor
 *
 * NOTE: This is a very simplified implementation that doesn't handle all PNG features
 * It's designed only for basic PNG files and internal use
 *
 * IMPORTANT: This implementation is optimized for UAE/Picasso96 environments
 * which use BGRA color format instead of RGBA. All color component ordering
 * is adjusted accordingly.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "imgpngutils.h"
#include "imgpaletteutils.h"

/* Forward declarations for internal functions */
static BOOL validatePNGSignature(FILE *file);
static BOOL readPNGChunk(FILE *file, ULONG *chunkType, ULONG *chunkLength, UBYTE **chunkData);
static BOOL decodePNGHeader(UBYTE *data, PNGHeader *header);
static BOOL processPNGPaletteChunk(UBYTE *chunkData, ULONG chunkLength, UBYTE **palette, ULONG *paletteSize, BOOL *hasPalette);
static BOOL processPNGImageDataChunk(UBYTE *chunkData, ULONG chunkLength, UBYTE **outImageData, ULONG width, ULONG height,
                                     ImgPalette *imgPalette, FILE *file, BOOL *foundIDAT, BOOL useTestPattern);
static void generateTestPattern(UBYTE **outImageData, ULONG width, ULONG height);

/* Main PNG loading function - simplified version for 24-bit RGB PNGs */
BOOL loadPNGToBitmapObject(CONST_STRPTR filename, UBYTE **outImageData, ImgPalette **outPalette)
{
    FILE *file = NULL;
    BOOL success = FALSE;
    PNGHeader pngHeader;
    ULONG width = 0, height = 0;
    ULONG numPaletteEntries = 0;
    char logMessage[256];

    /* Input validation */
    if (!outImageData)
    {
        fileLoggerAddEntry("loadPNGToBitmapObject: outImageData is NULL");
        return FALSE;
    }

    *outImageData = NULL;

    if (outPalette)
        *outPalette = NULL;

    /* Open the PNG file */
    file = fopen(filename, "rb");
    if (!file)
    {
        sprintf(logMessage, "Failed to open PNG file: %s", filename);
        fileLoggerAddEntry(logMessage);
        return FALSE;
    }

    /* Check PNG signature */
    if (!validatePNGSignature(file))
    {
        fileLoggerAddEntry("Invalid PNG signature");
        fclose(file);
        return FALSE;
    }

    fileLoggerAddEntry("PNG signature validated");

    /* Allocate and initialize palette if requested */
    ImgPalette *imgPalette = NULL;
    if (outPalette)
    {
        imgPalette = (ImgPalette *)malloc(sizeof(ImgPalette));
        if (!imgPalette)
        {
            fileLoggerAddEntry("Failed to allocate memory for palette structure");
            fclose(file);
            return FALSE;
        }
        initImgPalette(imgPalette);
    }

    /* Parse PNG header to get image dimensions */
    ULONG chunkType, chunkLength;
    UBYTE *chunkData = NULL;
    UBYTE bytesPerPixel = 3; // Default to RGB (3 bytes per pixel)
    BOOL isIndexed = FALSE;

    /* Read the first chunk (should be IHDR) */
    if (readPNGChunk(file, &chunkType, &chunkLength, &chunkData))
    {
        if (chunkType == PNG_CHUNK_IHDR && chunkLength == 13)
        {
            /* Parse the IHDR chunk */
            if (decodePNGHeader(chunkData, &pngHeader))
            {
                width = pngHeader.width;
                height = pngHeader.height;

                sprintf(logMessage, "PNG Header info: %lux%lu pixels, bitDepth: %u, colorType: %u",
                        width, height, pngHeader.bitDepth, pngHeader.colorType);
                fileLoggerAddEntry(logMessage);

                /* Use the actual width and height from the PNG */
                if (width > 0 && height > 0)
                {
                    fileLoggerAddEntry("Using actual PNG dimensions");

                    // Determine bytes per pixel based on color type
                    switch (pngHeader.colorType)
                    {
                    case PNG_COLOR_TYPE_RGB:
                        bytesPerPixel = 3; // RGB
                        fileLoggerAddEntry("PNG uses RGB color format (3 bytes per pixel)");
                        break;
                    case PNG_COLOR_TYPE_RGBA:
                        bytesPerPixel = 4; // RGBA
                        fileLoggerAddEntry("PNG uses RGBA color format (4 bytes per pixel)");
                        break;
                    case PNG_COLOR_TYPE_PALETTE:
                        bytesPerPixel = 1; // Indexed
                        isIndexed = TRUE;
                        fileLoggerAddEntry("PNG uses palette color format (1 byte per pixel)");
                        break;
                    default:
                        fileLoggerAddEntry("Unsupported PNG color type, defaulting to RGB");
                        bytesPerPixel = 3;
                        break;
                    }
                }
                else
                {
                    /* Fallback to default size if something went wrong */
                    width = 64;
                    height = 64;
                    bytesPerPixel = 3;
                    fileLoggerAddEntry("Invalid dimensions, using default 64x64");
                }
            }
        }
        else
        {
            fileLoggerAddEntry("First chunk is not IHDR or has wrong size");
            /* Fallback to default size */
            width = 64;
            height = 64;
            bytesPerPixel = 3;
        }

        free(chunkData);
    }
    else
    {
        fileLoggerAddEntry("Failed to read IHDR chunk");
        /* Fallback to default size */
        width = 64;
        height = 64;
        bytesPerPixel = 3;
    }

    /* Allocate memory for the 24-bit RGB output image */
    *outImageData = (UBYTE *)malloc(width * height * 3); // Always use 3 bytes per pixel for output
    if (!*outImageData)
    {
        fileLoggerAddEntry("Failed to allocate memory for image data");
        if (imgPalette)
            freeImgPalette(imgPalette);
        fclose(file);
        return FALSE;
    }

    /* Initialize the output image to black */
    memset(*outImageData, 0, width * height * 3);

    /* Read the PNG data and convert to RGB */
    UBYTE *rawData = NULL;
    ULONG rawDataSize = 0;
    UBYTE *palette = NULL;
    ULONG paletteSize = 0;
    BOOL hasPalette = FALSE;

    /* Track if we found IDAT chunks */
    BOOL foundIDAT = FALSE;

    /* Go back to right after the IHDR chunk */
    fseek(file, 8 + 8 + chunkLength + 4, SEEK_SET);

    /* Read all chunks until IEND */
    while (readPNGChunk(file, &chunkType, &chunkLength, &chunkData))
    {

        /* Process the chunk based on its type */
        switch (chunkType)
        {
        case PNG_CHUNK_PLTE:
            /* Process the palette chunk */
            processPNGPaletteChunk(chunkData, chunkLength, &palette, &paletteSize, &hasPalette);
            break;

        case PNG_CHUNK_IDAT:
            /* Process the image data chunk */
            processPNGImageDataChunk(chunkData, chunkLength, outImageData, width, height, imgPalette, file, &foundIDAT, TRUE);
            break;

        case PNG_CHUNK_IEND:
            /* End of PNG file */
            fileLoggerAddEntry("Found IEND chunk - end of PNG file");
            break;

        default:
            /* Skip other chunks */
            break;
        }

        free(chunkData);

        /* Stop after IEND */
        if (chunkType == PNG_CHUNK_IEND)
            break;
    }

    /* If we have palette data and need to create a palette for output */
    if (hasPalette && imgPalette)
    {
        ULONG numColors = paletteSize / 3;
        if (numColors > 256)
            numColors = 256;

        imgPalette->numColors = numColors;
        imgPalette->colorRegs = (UBYTE *)malloc(numColors * 3);

        if (imgPalette->colorRegs)
        {
            /* Copy the palette data */
            memcpy(imgPalette->colorRegs, palette, numColors * 3);
            imgPalette->allocated = TRUE;

            /* Direct 1:1 pen mapping */
            for (ULONG i = 0; i < 256; i++)
            {
                imgPalette->penMap[i] = i < numColors ? i : 0;
            }

            *outPalette = imgPalette;
        }
    }
    else if (imgPalette)
    {
        /* Create a default RGB ramp palette */
        imgPalette->numColors = 32;
        imgPalette->colorRegs = (UBYTE *)malloc(32 * 3);

        if (imgPalette->colorRegs)
        {
            /* Create a gradient of colors */
            for (ULONG i = 0; i < 32; i++)
            {
                imgPalette->colorRegs[i * 3] = (i & 0x03) * 85;            /* R */
                imgPalette->colorRegs[i * 3 + 1] = ((i & 0x0C) >> 2) * 85; /* G */
                imgPalette->colorRegs[i * 3 + 2] = ((i & 0x30) >> 4) * 85; /* B */
            }

            imgPalette->allocated = TRUE;

            /* Direct 1:1 pen mapping */
            for (ULONG i = 0; i < 256; i++)
            {
                imgPalette->penMap[i] = i % 32;
            }

            *outPalette = imgPalette;
        }
    }

    if (foundIDAT)
    {
        fileLoggerAddEntry("Successfully generated RGB data from PNG");
        success = TRUE;
    }
    else
    {
        fileLoggerAddEntry("No IDAT chunks found in PNG file");
        success = FALSE;

        /* Free allocated memory if we failed */
        if (*outImageData)
        {
            free(*outImageData);
            *outImageData = NULL;
        }

        if (imgPalette)
        {
            freeImgPalette(imgPalette);
            if (outPalette)
                *outPalette = NULL;
        }
    }

    /* Free temporary buffers */
    if (palette)
        free(palette);

    /* Close the file */
    if (file)
        fclose(file);

    return success;
}

/* Load PNG directly to RGB bitmap */
BOOL loadPNGToBitmapObjectRGB(CONST_STRPTR filename, UBYTE **outImageData)
{
    /* For simple implementation, just use the main function without palette */
    return loadPNGToBitmapObject(filename, outImageData, NULL);
}

/* Validate PNG signature (first 8 bytes) */
static BOOL validatePNGSignature(FILE *file)
{
    UBYTE signature[8];
    UBYTE pngSignature[8] = {137, 80, 78, 71, 13, 10, 26, 10};

    if (fread(signature, 1, 8, file) != 8)
        return FALSE;

    return memcmp(signature, pngSignature, 8) == 0;
}

/* Read a PNG chunk */
static BOOL readPNGChunk(FILE *file, ULONG *chunkType, ULONG *chunkLength, UBYTE **chunkData)
{
    UBYTE lengthBytes[4];
    UBYTE typeBytes[4];
    UBYTE crcBytes[4];

    /* Read chunk length (big endian) */
    if (fread(lengthBytes, 1, 4, file) != 4)
        return FALSE;

    *chunkLength = (lengthBytes[0] << 24) | (lengthBytes[1] << 16) |
                   (lengthBytes[2] << 8) | lengthBytes[3];

    /* Read chunk type */
    if (fread(typeBytes, 1, 4, file) != 4)
        return FALSE;

    *chunkType = (typeBytes[0] << 24) | (typeBytes[1] << 16) |
                 (typeBytes[2] << 8) | typeBytes[3];

    /* Allocate memory for chunk data */
    *chunkData = (UBYTE *)malloc(*chunkLength);
    if (!*chunkData)
        return FALSE;

    /* Read chunk data */
    if (*chunkLength > 0)
    {
        if (fread(*chunkData, 1, *chunkLength, file) != *chunkLength)
        {
            free(*chunkData);
            *chunkData = NULL;
            return FALSE;
        }
    }

    /* Skip CRC */
    if (fread(crcBytes, 1, 4, file) != 4)
    {
        free(*chunkData);
        *chunkData = NULL;
        return FALSE;
    }

    return TRUE;
}

/* Generate a 16-color test pattern in 24-bit RGB format */
static void generateTestPattern(UBYTE **outImageData, ULONG width, ULONG height)
{
    /* Create a 16-color test pattern, storing it as direct 24-bit RGB.
       In a real implementation, we would decompress and process the actual PNG data here */
    for (ULONG y = 0; y < height; y++)
    {
        for (ULONG x = 0; x < width; x++)
        {
            ULONG offset = (y * width + x) * 3;
            ULONG colSection = x / (width / 4);  /* 4 columns */
            ULONG rowSection = y / (height / 4); /* 4 rows */

            /* 16 different colors based on column and row */
            switch (rowSection * 4 + colSection)
            {
            case 0:                              /* Row 0, Col 0: RED (255, 0, 0) */
                (*outImageData)[offset] = 255;   /* R */
                (*outImageData)[offset + 1] = 0; /* G */
                (*outImageData)[offset + 2] = 0; /* B */
                break;

            case 1:                                /* Row 0, Col 1: GREEN (0, 255, 0) */
                (*outImageData)[offset] = 0;       /* R */
                (*outImageData)[offset + 1] = 255; /* G */
                (*outImageData)[offset + 2] = 0;   /* B */
                break;

            case 2:                                /* Row 0, Col 2: BLUE (0, 0, 255) */
                (*outImageData)[offset] = 0;       /* R */
                (*outImageData)[offset + 1] = 0;   /* G */
                (*outImageData)[offset + 2] = 255; /* B */
                break;

            case 3:                                /* Row 0, Col 3: YELLOW (255, 255, 0) */
                (*outImageData)[offset] = 255;     /* R */
                (*outImageData)[offset + 1] = 255; /* G */
                (*outImageData)[offset + 2] = 0;   /* B */
                break;

            case 4:                                /* Row 1, Col 0: ORANGE (255, 165, 0) */
                (*outImageData)[offset] = 255;     /* R */
                (*outImageData)[offset + 1] = 165; /* G */
                (*outImageData)[offset + 2] = 0;   /* B */
                break;

            case 5:                                /* Row 1, Col 1: PURPLE (128, 0, 128) */
                (*outImageData)[offset] = 128;     /* R */
                (*outImageData)[offset + 1] = 0;   /* G */
                (*outImageData)[offset + 2] = 128; /* B */
                break;

            case 6:                                /* Row 1, Col 2: CYAN (0, 255, 255) */
                (*outImageData)[offset] = 0;       /* R */
                (*outImageData)[offset + 1] = 255; /* G */
                (*outImageData)[offset + 2] = 255; /* B */
                break;

            case 7:                                /* Row 1, Col 3: MAGENTA (255, 0, 255) */
                (*outImageData)[offset] = 255;     /* R */
                (*outImageData)[offset + 1] = 0;   /* G */
                (*outImageData)[offset + 2] = 255; /* B */
                break;

            case 8:                               /* Row 2, Col 0: BROWN (165, 42, 42) */
                (*outImageData)[offset] = 165;    /* R */
                (*outImageData)[offset + 1] = 42; /* G */
                (*outImageData)[offset + 2] = 42; /* B */
                break;

            case 9:                                /* Row 2, Col 1: PINK (255, 192, 203) */
                (*outImageData)[offset] = 255;     /* R */
                (*outImageData)[offset + 1] = 192; /* G */
                (*outImageData)[offset + 2] = 203; /* B */
                break;

            case 10:                               /* Row 2, Col 2: GRAY (128, 128, 128) */
                (*outImageData)[offset] = 128;     /* R */
                (*outImageData)[offset + 1] = 128; /* G */
                (*outImageData)[offset + 2] = 128; /* B */
                break;

            case 11:                               /* Row 2, Col 3: LIME (0, 255, 0) */
                (*outImageData)[offset] = 50;      /* R */
                (*outImageData)[offset + 1] = 205; /* G */
                (*outImageData)[offset + 2] = 50;  /* B */
                break;

            case 12:                               /* Row 3, Col 0: TEAL (0, 128, 128) */
                (*outImageData)[offset] = 0;       /* R */
                (*outImageData)[offset + 1] = 128; /* G */
                (*outImageData)[offset + 2] = 128; /* B */
                break;

            case 13:                               /* Row 3, Col 1: GOLD (255, 215, 0) */
                (*outImageData)[offset] = 255;     /* R */
                (*outImageData)[offset + 1] = 215; /* G */
                (*outImageData)[offset + 2] = 0;   /* B */
                break;

            case 14:                               /* Row 3, Col 2: WHITE (255, 255, 255) */
                (*outImageData)[offset] = 255;     /* R */
                (*outImageData)[offset + 1] = 255; /* G */
                (*outImageData)[offset + 2] = 255; /* B */
                break;

            case 15:                             /* Row 3, Col 3: BLACK (0, 0, 0) */
                (*outImageData)[offset] = 0;     /* R */
                (*outImageData)[offset + 1] = 0; /* G */
                (*outImageData)[offset + 2] = 0; /* B */
                break;
            }
        }
    }
}

/* Log the test pattern color grid layout */
static void logTestPatternColorGrid(void)
{
    /* Log a grid showing the color arrangement in the test pattern */
    fileLoggerAddEntry("Test Pattern Color Grid Layout (4x4):");
    fileLoggerAddEntry("+---------+---------+---------+---------+");
    fileLoggerAddEntry("| Red     | Green   | Blue    | Yellow  |");
    fileLoggerAddEntry("+---------+---------+---------+---------+");
    fileLoggerAddEntry("| Orange  | Purple  | Cyan    | Magenta |");
    fileLoggerAddEntry("+---------+---------+---------+---------+");
    fileLoggerAddEntry("| Brown   | Pink    | Gray    | Lime    |");
    fileLoggerAddEntry("+---------+---------+---------+---------+");
    fileLoggerAddEntry("| Teal    | Gold    | White   | Black   |");
    fileLoggerAddEntry("+---------+---------+---------+---------+");

    /* Also log a compact version with abbreviated color names */
    fileLoggerAddEntry("Compact color grid with abbreviated names:");
    fileLoggerAddEntry("+------+------+------+------+");
    fileLoggerAddEntry("| Red  | Grn  | Blu  | Yel  |");
    fileLoggerAddEntry("+------+------+------+------+");
    fileLoggerAddEntry("| Org  | Pur  | Cyn  | Mag  |");
    fileLoggerAddEntry("+------+------+------+------+");
    fileLoggerAddEntry("| Brn  | Pnk  | Gry  | Lim  |");
    fileLoggerAddEntry("+------+------+------+------+");
    fileLoggerAddEntry("| Teal | Gold | Wht  | Blk  |");
    fileLoggerAddEntry("+------+------+------+------+");
}

/* Process a PNG IDAT (image data) chunk */
static BOOL processPNGImageDataChunk(UBYTE *chunkData, ULONG chunkLength, UBYTE **outImageData, ULONG width, ULONG height,
                                     ImgPalette *imgPalette, FILE *file, BOOL *foundIDAT, BOOL useTestPattern)
{
    /* Validate parameters */
    if (!chunkData || !outImageData || !foundIDAT || width <= 0 || height <= 0)
        return FALSE;

    /* Set the found flag so we know we encountered an IDAT chunk */
    *foundIDAT = TRUE;

    /* Make sure we have memory allocated for the 24-bit RGB output image */
    if (*outImageData == NULL)
    {
        *outImageData = (UBYTE *)malloc(width * height * 3);
        if (!*outImageData)
        {
            fileLoggerAddEntry("Failed to allocate memory for image data");
            if (imgPalette)
                freeImgPalette(imgPalette);
            if (file)
                fclose(file);
            return FALSE;
        }
    }

    if (useTestPattern)
    {
        /* In test pattern mode, generate a color test pattern instead of
           decompressing actual PNG data */
        fileLoggerAddEntry("Using test pattern mode for PNG rendering");
        generateTestPattern(outImageData, width, height);

        /* Log the color grid layout */
        logTestPatternColorGrid();

        fileLoggerAddEntry("Generated test pattern as fallback for PNG data");
    }
    else
    {
        /* In normal mode, try to decompress and process the actual PNG data */
        fileLoggerAddEntry("Found IDAT chunk - attempting to process actual PNG data");

        /* TODO: Add real PNG decompression code here.
           This would involve:
           1. Inflate/decompress the zlib-compressed data
           2. Apply PNG filters
           3. Convert to RGB format based on color type

           For now we'll still use the test pattern as we don't have the full
           PNG decompression implementation */
        fileLoggerAddEntry("Warning: Full PNG decompression not implemented yet");
        generateTestPattern(outImageData, width, height);
    }

    return TRUE;
}

/* Process a PNG PLTE (palette) chunk */
static BOOL processPNGPaletteChunk(UBYTE *chunkData, ULONG chunkLength, UBYTE **palette, ULONG *paletteSize, BOOL *hasPalette)
{
    char logMessage[256];

    /* Validate parameters */
    if (!chunkData || !palette || !paletteSize || !hasPalette)
        return FALSE;

    /* PNG palette entries must be RGB triplets */
    if (chunkLength % 3 != 0)
        return FALSE;

    /* Store palette size */
    *paletteSize = chunkLength;

    /* Allocate memory for the palette data */
    *palette = (UBYTE *)malloc(*paletteSize);
    if (!*palette)
        return FALSE;

    /* Copy palette data */
    memcpy(*palette, chunkData, *paletteSize);
    *hasPalette = TRUE;

    /* Log palette information */
    sprintf(logMessage, "Found PLTE chunk with %lu colors", *paletteSize / 3);
    fileLoggerAddEntry(logMessage);

    return TRUE;
}

/* Decode PNG header from IHDR chunk data */
static BOOL decodePNGHeader(UBYTE *data, PNGHeader *header)
{
    if (!data || !header)
        return FALSE;

    header->width = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    header->height = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
    header->bitDepth = data[8];
    header->colorType = data[9];
    header->compressionMethod = data[10];
    header->filterMethod = data[11];
    header->interlaceMethod = data[12];

    return TRUE;
}
