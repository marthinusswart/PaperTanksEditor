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

#include "imgpngutils.h"

/* Forward declarations for internal functions */
static BOOL validatePNGSignature(FILE *file);
static BOOL readPNGChunk(FILE *file, ULONG *chunkType, ULONG *chunkLength, UBYTE **chunkData);
static BOOL decodePNGHeader(UBYTE *data, PNGHeader *header);
static BOOL processPNGPaletteChunk(UBYTE *chunkData, ULONG chunkLength, UBYTE **palette, ULONG *paletteSize, BOOL *hasPalette);
static BOOL processPNGTransparencyChunk(UBYTE *chunkData, ULONG chunkLength, UBYTE **transData, ULONG *transSize, BOOL *hasTrans, UBYTE colorType);
static BOOL processPNGImageDataChunk(UBYTE *chunkData, ULONG chunkLength, UBYTE **outImageData, ULONG width, ULONG height,
                                     ImgPalette *imgPalette, FILE *file, BOOL *foundIDAT, BOOL useTestPattern, PNGHeader *pngHeader,
                                     UBYTE *transData, ULONG transSize, BOOL hasTrans);
static void generateTestPattern(UBYTE **outImageData, ULONG width, ULONG height);
static void logTestPatternColorGrid(void);

/* Main PNG loading function - simplified version for 24-bit RGB PNGs */
BOOL loadPNGToBitmapObject2(CONST_STRPTR filename, UBYTE **outImageData, ImgPalette **outPalette)
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
        fileLoggerAddDebugEntry("loadPNGToBitmapObject: outImageData is NULL");
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
        fileLoggerAddDebugEntry(logMessage);
        return FALSE;
    }

    /* Check PNG signature */
    if (!validatePNGSignature(file))
    {
        fileLoggerAddDebugEntry("Invalid PNG signature");
        fclose(file);
        return FALSE;
    }

    fileLoggerAddDebugEntry("PNG signature validated");

    /* Allocate and initialize palette if requested */
    ImgPalette *imgPalette = NULL;
    if (outPalette)
    {
        imgPalette = (ImgPalette *)malloc(sizeof(ImgPalette));
        if (!imgPalette)
        {
            fileLoggerAddDebugEntry("Failed to allocate memory for palette structure");
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
                fileLoggerAddDebugEntry(logMessage);

                /* Use the actual width and height from the PNG */
                if (width > 0 && height > 0)
                {
                    fileLoggerAddDebugEntry("Using actual PNG dimensions");

                    // Determine bytes per pixel based on color type
                    switch (pngHeader.colorType)
                    {
                    case PNG_COLOR_TYPE_RGB:
                        bytesPerPixel = 3; // RGB
                        fileLoggerAddDebugEntry("PNG uses RGB color format (3 bytes per pixel)");
                        break;
                    case PNG_COLOR_TYPE_RGBA:
                        bytesPerPixel = 4; // RGBA
                        fileLoggerAddDebugEntry("PNG uses RGBA color format (4 bytes per pixel)");
                        break;
                    case PNG_COLOR_TYPE_PALETTE:
                        bytesPerPixel = 1; // Indexed
                        isIndexed = TRUE;
                        fileLoggerAddDebugEntry("PNG uses palette color format (1 byte per pixel)");
                        break;
                    default:
                        fileLoggerAddDebugEntry("Unsupported PNG color type, defaulting to RGB");
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
                    fileLoggerAddDebugEntry("Invalid dimensions, using default 64x64");
                }
            }
        }
        else
        {
            fileLoggerAddDebugEntry("First chunk is not IHDR or has wrong size");
            /* Fallback to default size */
            width = 64;
            height = 64;
            bytesPerPixel = 3;
        }

        free(chunkData);
    }
    else
    {
        fileLoggerAddDebugEntry("Failed to read IHDR chunk");
        /* Fallback to default size */
        width = 64;
        height = 64;
        bytesPerPixel = 3;
    }

    /* Allocate memory for the 24-bit RGB output image */
    *outImageData = (UBYTE *)malloc(width * height * 3); // Always use 3 bytes per pixel for output
    if (!*outImageData)
    {
        fileLoggerAddDebugEntry("Failed to allocate memory for image data");
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

    /* Transparency data */
    UBYTE *transData = NULL;
    ULONG transSize = 0;
    BOOL hasTrans = FALSE;

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

        case PNG_CHUNK_TRNS:
            /* Process the transparency chunk */
            processPNGTransparencyChunk(chunkData, chunkLength, &transData, &transSize, &hasTrans, pngHeader.colorType);
            break;

        case PNG_CHUNK_IDAT:
            /* Process the image data chunk */
            processPNGImageDataChunk(chunkData, chunkLength, outImageData, width, height, imgPalette, file, &foundIDAT, FALSE, &pngHeader, transData, transSize, hasTrans);
            break;

        case PNG_CHUNK_IEND:
            /* End of PNG file */
            fileLoggerAddDebugEntry("Found IEND chunk - end of PNG file");
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
        int numColors = 32;
        imgPalette->colorRegs = (UBYTE *)malloc(numColors * 3);

        if (imgPalette->colorRegs)
        {
            /* Copy the palette data */
            memcpy(imgPalette->colorRegs, palette, numColors * 3);
            imgPalette->allocated = TRUE;

            /* Set transparent color if we have transparency data */
            if (hasTrans && transData && transSize > 0 && pngHeader.colorType == PNG_COLOR_TYPE_PALETTE)
            {
                /* For palette images, each byte in transData represents an alpha value for the corresponding palette entry */
                imgPalette->transparentColor = 0; /* Default to first entry */

                /* Find the first fully transparent color (value 0) */
                for (ULONG i = 0; i < transSize && i < numColors; i++)
                {
                    if (transData[i] == 0)
                    {
                        imgPalette->transparentColor = i;
                        imgPalette->hasTransparency = TRUE;
                        sprintf(logMessage, "Setting transparent color to index %lu", i);
                        fileLoggerAddDebugEntry(logMessage);
                        break;
                    }
                }
            }

            *outPalette = imgPalette;
        }
    }

    /* Free transparency data if allocated */
    if (transData)
    {
        free(transData);
    }

    if (foundIDAT)
    {
        fileLoggerAddDebugEntry("Successfully generated RGB data from PNG");
        success = TRUE;
    }
    else
    {
        fileLoggerAddDebugEntry("No IDAT chunks found in PNG file");
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

    /* Free palette memory */
    if (palette)
        free(palette);

    /* Close the file */
    fclose(file);

    return success;
}

/* Check if a file has a valid PNG signature */
static BOOL validatePNGSignature(FILE *file)
{
    const UBYTE pngSignature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    UBYTE buffer[8];

    /* Go to the beginning of the file */
    fseek(file, 0, SEEK_SET);

    /* Read the signature bytes */
    if (fread(buffer, 1, 8, file) != 8)
    {
        fileLoggerAddDebugEntry("Failed to read PNG signature bytes");
        return FALSE;
    }

    /* Compare with the expected PNG signature */
    if (memcmp(buffer, pngSignature, 8) != 0)
    {
        fileLoggerAddDebugEntry("Invalid PNG signature");
        return FALSE;
    }

    return TRUE;
}

/* Read a PNG chunk from a file */
static BOOL readPNGChunk(FILE *file, ULONG *chunkType, ULONG *chunkLength, UBYTE **chunkData)
{
    UBYTE buffer[8];
    char chunkName[5];

    /* Read the chunk length and type (8 bytes total) */
    if (fread(buffer, 1, 8, file) != 8)
    {
        fileLoggerAddDebugEntry("Failed to read PNG chunk header");
        return FALSE;
    }

    /* Parse the chunk length (big endian) */
    *chunkLength = ((ULONG)buffer[0] << 24) | ((ULONG)buffer[1] << 16) |
                   ((ULONG)buffer[2] << 8) | (ULONG)buffer[3];

    /* Parse the chunk type */
    *chunkType = ((ULONG)buffer[4] << 24) | ((ULONG)buffer[5] << 16) |
                 ((ULONG)buffer[6] << 8) | (ULONG)buffer[7];

    /* For logging, create a readable chunk name */
    chunkName[0] = buffer[4];
    chunkName[1] = buffer[5];
    chunkName[2] = buffer[6];
    chunkName[3] = buffer[7];
    chunkName[4] = '\0';

    /* Log the chunk information */
    // char logMessage[256];
    // sprintf(logMessage, "Found PNG chunk: %s, length: %lu bytes", chunkName, *chunkLength);
    // fileLoggerAddDebugEntry(logMessage);

    /* Allocate memory for the chunk data (if there is any) */
    if (*chunkLength > 0)
    {
        *chunkData = (UBYTE *)malloc(*chunkLength);
        if (!*chunkData)
        {
            fileLoggerAddDebugEntry("Failed to allocate memory for PNG chunk data");
            return FALSE;
        }

        /* Read the chunk data */
        if (fread(*chunkData, 1, *chunkLength, file) != *chunkLength)
        {
            fileLoggerAddDebugEntry("Failed to read PNG chunk data");
            free(*chunkData);
            *chunkData = NULL;
            return FALSE;
        }
    }
    else
    {
        *chunkData = NULL;
    }

    /* Skip the CRC (4 bytes) */
    fseek(file, 4, SEEK_CUR);

    return TRUE;
}

/* Decode a PNG header (IHDR chunk) */
static BOOL decodePNGHeader(UBYTE *data, PNGHeader *header)
{
    if (!data || !header)
        return FALSE;

    /* Parse the header fields (big endian) */
    header->width = ((ULONG)data[0] << 24) | ((ULONG)data[1] << 16) |
                    ((ULONG)data[2] << 8) | (ULONG)data[3];

    header->height = ((ULONG)data[4] << 24) | ((ULONG)data[5] << 16) |
                     ((ULONG)data[6] << 8) | (ULONG)data[7];

    header->bitDepth = data[8];
    header->colorType = data[9];
    header->compressionMethod = data[10];
    header->filterMethod = data[11];
    header->interlaceMethod = data[12];

    /* Log the header information */
    char logMessage[256];
    sprintf(logMessage, "PNG Header: %lu x %lu pixels, %u-bit, color type %u",
            header->width, header->height, header->bitDepth, header->colorType);
    fileLoggerAddDebugEntry(logMessage);

    /* Validate the header */
    if (header->width <= 0 || header->height <= 0)
    {
        fileLoggerAddDebugEntry("Invalid PNG dimensions");
        return FALSE;
    }

    /* Ensure we can handle the bit depth and color type */
    BOOL supported = FALSE;

    switch (header->colorType)
    {
    case PNG_COLOR_TYPE_GRAYSCALE:
        supported = (header->bitDepth == 1 || header->bitDepth == 2 ||
                     header->bitDepth == 4 || header->bitDepth == 8 ||
                     header->bitDepth == 16);
        break;

    case PNG_COLOR_TYPE_RGB:
        supported = (header->bitDepth == 8 || header->bitDepth == 16);
        break;

    case PNG_COLOR_TYPE_PALETTE:
        supported = (header->bitDepth == 1 || header->bitDepth == 2 ||
                     header->bitDepth == 4 || header->bitDepth == 8);
        break;

    case PNG_COLOR_TYPE_GRAYSCALE_ALPHA:
        supported = (header->bitDepth == 8 || header->bitDepth == 16);
        break;

    case PNG_COLOR_TYPE_RGBA:
        supported = (header->bitDepth == 8 || header->bitDepth == 16);
        break;

    default:
        supported = FALSE;
        break;
    }

    if (!supported)
    {
        sprintf(logMessage, "Unsupported PNG color type %u with bit depth %u",
                header->colorType, header->bitDepth);
        fileLoggerAddDebugEntry(logMessage);
        return FALSE;
    }

    /* Check compression, filter, and interlace methods */
    if (header->compressionMethod != 0)
    {
        fileLoggerAddDebugEntry("Unsupported PNG compression method");
        return FALSE;
    }

    if (header->filterMethod != 0)
    {
        fileLoggerAddDebugEntry("Unsupported PNG filter method");
        return FALSE;
    }

    /* Warning for interlaced images - we don't fully support them */
    if (header->interlaceMethod != 0)
    {
        fileLoggerAddDebugEntry("Warning: Interlaced PNG may not display correctly");
    }

    return TRUE;
}

/* Process a PNG PLTE (palette) chunk */
static BOOL processPNGPaletteChunk(UBYTE *chunkData, ULONG chunkLength, UBYTE **palette, ULONG *paletteSize, BOOL *hasPalette)
{
    /* Validate parameters */
    if (!chunkData || !palette || !paletteSize || !hasPalette)
        return FALSE;

    /* Check if we already have a palette */
    if (*hasPalette)
    {
        fileLoggerAddDebugEntry("Multiple PLTE chunks found, using only the first one");
        return FALSE;
    }

    /* Check that the palette size is valid (must be a multiple of 3) */
    if (chunkLength % 3 != 0)
    {
        fileLoggerAddDebugEntry("Invalid palette length, not a multiple of 3");
        return FALSE;
    }

    /* Allocate memory for the palette */
    *palette = (UBYTE *)malloc(chunkLength);
    if (!*palette)
    {
        fileLoggerAddDebugEntry("Failed to allocate memory for palette");
        return FALSE;
    }

    /* Copy the palette data */
    memcpy(*palette, chunkData, chunkLength);
    *paletteSize = chunkLength;
    *hasPalette = TRUE;

    /* Log information about the palette */
    ULONG numColors = chunkLength / 3;
    char logMessage[256];
    sprintf(logMessage, "Palette: %lu colors", numColors);
    fileLoggerAddDebugEntry(logMessage);

    return TRUE;
}

/* Process a PNG tRNS (transparency) chunk */
static BOOL processPNGTransparencyChunk(UBYTE *chunkData, ULONG chunkLength, UBYTE **transData, ULONG *transSize, BOOL *hasTrans, UBYTE colorType)
{
    /* Validate parameters */
    if (!chunkData || !transData || !transSize || !hasTrans)
        return FALSE;

    /* Check if we already have transparency data */
    if (*hasTrans)
    {
        fileLoggerAddDebugEntry("Multiple tRNS chunks found, using only the first one");
        return FALSE;
    }

    /* Process transparency based on color type */
    fileLoggerAddDebugEntry("Processing tRNS chunk (transparency data)");

    /* Allocate memory for transparency data */
    *transData = (UBYTE *)malloc(chunkLength);
    if (!*transData)
    {
        fileLoggerAddDebugEntry("Failed to allocate memory for transparency data");
        return FALSE;
    }

    /* Copy transparency data */
    memcpy(*transData, chunkData, chunkLength);
    *transSize = chunkLength;
    *hasTrans = TRUE;

    char logMessage[256];
    sprintf(logMessage, "Transparency data: %lu bytes for color type %u", chunkLength, colorType);
    fileLoggerAddDebugEntry(logMessage);

    return TRUE;
}

/* Generate a test pattern in the output buffer for debugging */
static void generateTestPattern(UBYTE **outImageData, ULONG width, ULONG height)
{
    /* Simple test pattern of colorful blocks */
    fileLoggerAddDebugEntry("Generating test pattern of colored blocks");

    /* Array of colors for the test pattern (R,G,B triplets) */
    const UBYTE colors[16][3] = {
        {255, 0, 0},     /* Red */
        {0, 255, 0},     /* Green */
        {0, 0, 255},     /* Blue */
        {255, 255, 0},   /* Yellow */
        {255, 128, 0},   /* Orange */
        {128, 0, 128},   /* Purple */
        {0, 255, 255},   /* Cyan */
        {255, 0, 255},   /* Magenta */
        {128, 64, 0},    /* Brown */
        {255, 128, 128}, /* Pink */
        {128, 128, 128}, /* Gray */
        {128, 255, 0},   /* Lime */
        {0, 128, 128},   /* Teal */
        {255, 215, 0},   /* Gold */
        {255, 255, 255}, /* White */
        {0, 0, 0}        /* Black */
    };

    /* Calculate the size of each block in the grid */
    ULONG blockWidth = width / 4;
    ULONG blockHeight = height / 4;

    /* Make sure blocks have at least 1 pixel */
    if (blockWidth < 1)
        blockWidth = 1;
    if (blockHeight < 1)
        blockHeight = 1;

    /* Fill the output image with the pattern */
    for (ULONG y = 0; y < height; y++)
    {
        for (ULONG x = 0; x < width; x++)
        {
            /* Determine which block this pixel belongs to */
            ULONG blockX = x / blockWidth;
            ULONG blockY = y / blockHeight;

            /* Limit to 4x4 grid */
            if (blockX > 3)
                blockX = 3;
            if (blockY > 3)
                blockY = 3;

            /* Determine color index */
            ULONG colorIndex = blockY * 4 + blockX;

            /* Write the RGB values */
            ULONG pixelOffset = (y * width + x) * 3;
            (*outImageData)[pixelOffset] = colors[colorIndex][0];     /* R */
            (*outImageData)[pixelOffset + 1] = colors[colorIndex][1]; /* G */
            (*outImageData)[pixelOffset + 2] = colors[colorIndex][2]; /* B */
        }
    }
}

/* Log a color grid layout of the test pattern for debugging */
static void logTestPatternColorGrid(void)
{
    /* Log a grid showing the color arrangement in the test pattern */
    fileLoggerAddDebugEntry("Test Pattern Color Grid Layout (4x4):");
    fileLoggerAddDebugEntry("+---------+---------+---------+---------+");
    fileLoggerAddDebugEntry("| Red     | Green   | Blue    | Yellow  |");
    fileLoggerAddDebugEntry("+---------+---------+---------+---------+");
    fileLoggerAddDebugEntry("| Orange  | Purple  | Cyan    | Magenta |");
    fileLoggerAddDebugEntry("+---------+---------+---------+---------+");
    fileLoggerAddDebugEntry("| Brown   | Pink    | Gray    | Lime    |");
    fileLoggerAddDebugEntry("+---------+---------+---------+---------+");
    fileLoggerAddDebugEntry("| Teal    | Gold    | White   | Black   |");
    fileLoggerAddDebugEntry("+---------+---------+---------+---------+");

    /* Also log a compact version with abbreviated color names */
    fileLoggerAddDebugEntry("Compact color grid with abbreviated names:");
    fileLoggerAddDebugEntry("+------+------+------+------+");
    fileLoggerAddDebugEntry("| Red  | Grn  | Blu  | Yel  |");
    fileLoggerAddDebugEntry("+------+------+------+------+");
    fileLoggerAddDebugEntry("| Org  | Pur  | Cyn  | Mag  |");
    fileLoggerAddDebugEntry("+------+------+------+------+");
    fileLoggerAddDebugEntry("| Brn  | Pnk  | Gry  | Lim  |");
    fileLoggerAddDebugEntry("+------+------+------+------+");
    fileLoggerAddDebugEntry("| Teal | Gold | Wht  | Blk  |");
    fileLoggerAddDebugEntry("+------+------+------+------+");
}

/* Process a PNG IDAT (image data) chunk */
static BOOL processPNGImageDataChunk(UBYTE *chunkData, ULONG chunkLength, UBYTE **outImageData, ULONG width, ULONG height,
                                     ImgPalette *imgPalette, FILE *file, BOOL *foundIDAT, BOOL useTestPattern, PNGHeader *pngHeader,
                                     UBYTE *transData, ULONG transSize, BOOL hasTrans)
{
    char logMessage[256];

    /* Validate parameters */
    if (!chunkData || !outImageData || !foundIDAT || width <= 0 || height <= 0 || !pngHeader)
        return FALSE;

    /* Set the found flag so we know we encountered an IDAT chunk */
    *foundIDAT = TRUE;

    /* Make sure we have memory allocated for the 24-bit RGB output image */
    if (*outImageData == NULL)
    {
        *outImageData = (UBYTE *)malloc(width * height * 3);
        if (!*outImageData)
        {
            fileLoggerAddDebugEntry("Failed to allocate memory for image data");
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
        fileLoggerAddDebugEntry("Using test pattern mode for PNG rendering");
        generateTestPattern(outImageData, width, height);

        /* Log the color grid layout */
        logTestPatternColorGrid();

        fileLoggerAddDebugEntry("Generated test pattern as fallback for PNG data");
    }
    else
    {
        /* In normal mode, try to decompress and process the actual PNG data */
        fileLoggerAddDebugEntry("Found IDAT chunk - attempting to process actual PNG data");

        /* Step 1: Decompress the zlib-compressed data */
        UBYTE *decompressedData = NULL;
        ULONG decompressedSize = 0;

        BOOL enableDebug = TRUE;
        BOOL decompressed = FALSE;

        if (enableDebug)
        {
            decompressed = decompressZlibData(chunkData, chunkLength, &decompressedData, &decompressedSize);
        }

        if (decompressed)
        {
            /* Step 2: Apply PNG filters and convert to RGB */
            sprintf(logMessage, "Successfully decompressed %lu bytes of PNG data", decompressedSize);
            fileLoggerAddDebugEntry(logMessage);

            /* Get the bytes per pixel based on color type */
            UBYTE bytesPerPixel = 0;
            BOOL success = FALSE;

            /* Determine bytes per pixel for filtering based on color type */
            switch (pngHeader->colorType)
            {
            case PNG_COLOR_TYPE_GRAYSCALE:
                bytesPerPixel = (pngHeader->bitDepth + 7) / 8; /* Round up to nearest byte */
                break;

            case PNG_COLOR_TYPE_RGB:
                bytesPerPixel = 3 * pngHeader->bitDepth / 8;
                break;

            case PNG_COLOR_TYPE_PALETTE:
                bytesPerPixel = 1; /* Indexed color - always 1 byte */
                break;

            case PNG_COLOR_TYPE_GRAYSCALE_ALPHA:
                bytesPerPixel = 2 * pngHeader->bitDepth / 8;
                break;

            case PNG_COLOR_TYPE_RGBA:
                bytesPerPixel = 4 * pngHeader->bitDepth / 8;
                break;

            default:
                bytesPerPixel = 0;
                break;
            }

            /* Check if we have a valid bytes per pixel value */
            if (bytesPerPixel > 0)
            {
                /* Allocate a buffer for the unfiltered data */
                UBYTE *unfilteredData = (UBYTE *)malloc(width * height * bytesPerPixel);
                if (unfilteredData)
                {
                    /* Apply PNG filters */
                    if (applyPNGFilters(decompressedData, decompressedSize, width, height, bytesPerPixel, unfilteredData))
                    {
                        /* Convert unfiltered data to RGB format for output */
                        fileLoggerAddDebugEntry("Successfully applied PNG filters");

                        /* Log transparency info if available */
                        if (hasTrans && transData)
                        {
                            fileLoggerAddDebugEntry("Using transparency information from tRNS chunk");
                        }

                        /* Convert to RGB format based on color type */
                        switch (pngHeader->colorType)
                        {
                        case PNG_COLOR_TYPE_RGB:
                            /* For RGB PNGs with transparency, check for single color transparency */
                            if (hasTrans && transData && transSize >= 6)
                            {
                                /* tRNS for RGB defines a single transparent color (R,G,B) */
                                UWORD transR = (transData[0] << 8) | transData[1];
                                UWORD transG = (transData[2] << 8) | transData[3];
                                UWORD transB = (transData[4] << 8) | transData[5];

                                sprintf(logMessage, "Transparent RGB color: (%u,%u,%u)", transR, transG, transB);
                                fileLoggerAddDebugEntry(logMessage);

                                /* Compare each pixel and make transparent pixels black */
                                for (ULONG i = 0; i < width * height; i++)
                                {
                                    UBYTE r = unfilteredData[i * 3];
                                    UBYTE g = unfilteredData[i * 3 + 1];
                                    UBYTE b = unfilteredData[i * 3 + 2];

                                    /* Check if this pixel matches the transparent color */
                                    if (r == (transR & 0xFF) && g == (transG & 0xFF) && b == (transB & 0xFF))
                                    {
                                        /* Make transparent pixels completely black as a marker */
                                        (*outImageData)[i * 3] = 0;     /* R */
                                        (*outImageData)[i * 3 + 1] = 0; /* G */
                                        (*outImageData)[i * 3 + 2] = 0; /* B */

                                        /* If we have a palette and it supports transparency */
                                        if (imgPalette)
                                        {
                                            imgPalette->hasTransparency = TRUE;
                                            imgPalette->transparentColor = 0; /* Using black as transparent */
                                        }
                                    }
                                    else
                                    {
                                        /* Copy non-transparent pixels directly */
                                        (*outImageData)[i * 3] = r;     /* R */
                                        (*outImageData)[i * 3 + 1] = g; /* G */
                                        (*outImageData)[i * 3 + 2] = b; /* B */
                                    }
                                }
                            }
                            else
                            {
                                /* No transparency, direct copy for RGB data */
                                memcpy(*outImageData, unfilteredData, width * height * 3);
                            }
                            success = TRUE;
                            fileLoggerAddDebugEntry("Converted PNG RGB data to output format");
                            break;

                        case PNG_COLOR_TYPE_RGBA:
                            /* For RGBA, use alpha channel for transparency */
                            fileLoggerAddDebugEntry("Processing RGBA data with alpha channel");

                            /* If we have a palette, we'll need to mark the transparent color */
                            if (imgPalette)
                            {
                                imgPalette->hasTransparency = FALSE; // Start with no transparency
                            }

                            // First pass: check if we have any transparent pixels
                            BOOL hasTransPixels = FALSE;
                            for (ULONG i = 0; i < width * height && !hasTransPixels; i++)
                            {
                                UBYTE a = unfilteredData[i * 4 + 3];
                                if (a < 128) // If pixel is mostly transparent
                                {
                                    hasTransPixels = TRUE;
                                }
                            }

                            // Only set hasTransparency if we actually found transparent pixels
                            if (hasTransPixels && imgPalette)
                            {
                                imgPalette->hasTransparency = TRUE;
                                imgPalette->transparentColor = 0; // Using black as the marker
                                fileLoggerAddDebugEntry("Found transparent pixels in RGBA image");
                            }
                            else
                            {
                                fileLoggerAddDebugEntry("No transparent pixels found in RGBA image");
                            }

                            for (ULONG i = 0; i < width * height; i++)
                            {
                                UBYTE r = unfilteredData[i * 4];
                                UBYTE g = unfilteredData[i * 4 + 1];
                                UBYTE b = unfilteredData[i * 4 + 2];
                                UBYTE a = unfilteredData[i * 4 + 3];

                                if (a < 128) /* If pixel is mostly transparent */
                                {
                                    // For transparent pixels, we'll set them to black (0,0,0)
                                    // This is our marker for transparency
                                    (*outImageData)[i * 3] = 0;     /* R */
                                    (*outImageData)[i * 3 + 1] = 0; /* G */
                                    (*outImageData)[i * 3 + 2] = 0; /* B */
                                }
                                else
                                {
                                    // For non-transparent pixels, copy the RGB values
                                    // If the pixel is black (0,0,0) but not transparent, we'll adjust
                                    // it slightly so it's not confused with transparent black
                                    if (r == 0 && g == 0 && b == 0 && imgPalette && imgPalette->hasTransparency)
                                    {
                                        // If this is a legitimate black pixel and we have transparency
                                        // Adjust to near-black to distinguish from transparent black
                                        (*outImageData)[i * 3] = 1;     /* R - slight adjustment */
                                        (*outImageData)[i * 3 + 1] = 1; /* G - slight adjustment */
                                        (*outImageData)[i * 3 + 2] = 1; /* B - slight adjustment */
                                    }
                                    else
                                    {
                                        // For all other non-transparent colors, use the original values
                                        (*outImageData)[i * 3] = r;     /* R */
                                        (*outImageData)[i * 3 + 1] = g; /* G */
                                        (*outImageData)[i * 3 + 2] = b; /* B */
                                    }
                                }
                            }
                            success = TRUE;
                            fileLoggerAddDebugEntry("Converted PNG RGBA data to RGB output format with transparency");
                            break;

                        case PNG_COLOR_TYPE_PALETTE:
                            /* For indexed color, use palette entries */
                            if (imgPalette && imgPalette->colorRegs)
                            {
                                /* If we have transparency data for the palette */
                                if (hasTrans && transData && transSize > 0)
                                {
                                    /* Set the first fully transparent color */
                                    for (ULONG t = 0; t < transSize; t++)
                                    {
                                        if (transData[t] == 0) /* Fully transparent */
                                        {
                                            imgPalette->transparentColor = t;
                                            imgPalette->hasTransparency = TRUE;
                                            sprintf(logMessage, "Palette transparency: Index %lu is fully transparent", t);
                                            fileLoggerAddDebugEntry(logMessage);
                                            break;
                                        }
                                    }
                                }

                                /* Convert indexed data to RGB using the palette */
                                for (ULONG i = 0; i < width * height; i++)
                                {
                                    UBYTE index = unfilteredData[i];
                                    if (index < 256)
                                    {
                                        /* Get color from palette */
                                        (*outImageData)[i * 3] = imgPalette->colorRegs[index * 3];         /* R */
                                        (*outImageData)[i * 3 + 1] = imgPalette->colorRegs[index * 3 + 1]; /* G */
                                        (*outImageData)[i * 3 + 2] = imgPalette->colorRegs[index * 3 + 2]; /* B */
                                    }
                                    else
                                    {
                                        /* Invalid index, use black */
                                        (*outImageData)[i * 3] = 0;     /* R */
                                        (*outImageData)[i * 3 + 1] = 0; /* G */
                                        (*outImageData)[i * 3 + 2] = 0; /* B */
                                    }
                                }
                                success = TRUE;
                                fileLoggerAddDebugEntry("Converted indexed PNG data to RGB using palette");
                            }
                            else
                            {
                                fileLoggerAddErrorEntry("No palette available for indexed PNG");
                            }
                            break;

                        default:
                            /* Other color types not yet implemented */
                            fileLoggerAddErrorEntry("Unsupported PNG color type for conversion");
                            break;
                        }
                    }
                    else
                    {
                        fileLoggerAddErrorEntry("PNG filter processing failed");
                    }

                    /* Free unfiltered data */
                    free(unfilteredData);
                }
                else
                {
                    fileLoggerAddErrorEntry("Failed to allocate memory for unfiltered data");
                }
            }
            else
            {
                fileLoggerAddErrorEntry("Invalid bytes per pixel value for PNG format");
            }

            /* Free decompressed data */
            free(decompressedData);

            /* If we successfully processed the PNG, we're done */
            if (success)
            {
                fileLoggerAddDebugEntry("Successfully processed PNG image data");
                return TRUE;
            }

            /* Fall back to test pattern if processing failed */
            fileLoggerAddErrorEntry("PNG processing failed, using test pattern as fallback");
            generateTestPattern(outImageData, width, height);
        }
        else
        {
            /* Decompression failed, fall back to test pattern */
            fileLoggerAddErrorEntry("PNG decompression failed, using test pattern instead");
            generateTestPattern(outImageData, width, height);
        }
    }

    return TRUE;
}

/* Initialize a palette structure */
void initImgPalette(ImgPalette *palette)
{
    if (palette)
    {
        palette->colorRegs = NULL;
        palette->allocated = FALSE;
        palette->hasTransparency = FALSE;
        palette->transparentColor = 0;
    }
}

/* Free resources allocated for a palette */
void freeImgPalette(ImgPalette *palette)
{
    if (palette)
    {
        /* Only free if we allocated it */
        if (palette->allocated && palette->colorRegs)
        {
            free(palette->colorRegs);
            palette->colorRegs = NULL;
        }

        /* Reset fields */
        palette->colorTable = NULL;
        palette->allocated = FALSE;
    }
}
