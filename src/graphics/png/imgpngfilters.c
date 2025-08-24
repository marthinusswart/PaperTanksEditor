/*
 * PNG filter processing utilities for AmigaOS 3.1
 * Used to process filtered scanlines in PNG files
 */

#include "imgpngfilters.h"

/* Paeth predictor function as defined in the PNG specification */
UBYTE paethPredictor(UBYTE a, UBYTE b, UBYTE c)
{
    /* a = left, b = above, c = upper left */
    LONG p = (LONG)a + (LONG)b - (LONG)c;
    LONG pa = abs(p - (LONG)a);
    LONG pb = abs(p - (LONG)b);
    LONG pc = abs(p - (LONG)c);

    if (pa <= pb && pa <= pc)
        return a;
    else if (pb <= pc)
        return b;
    else
        return c;
}

/* Process a scanline with no filtering (type 0) */
void applyNoneFilter(UBYTE *filtered, UBYTE *scanline, UBYTE *prevScanline,
                     ULONG lineBytes, UBYTE bytesPerPixel)
{
    /* Skip the filter type byte and just copy the data */
    memcpy(scanline, filtered + 1, lineBytes);
}

/* Process a scanline with Sub filter (type 1) */
void applySubFilter(UBYTE *filtered, UBYTE *scanline, UBYTE *prevScanline,
                    ULONG lineBytes, UBYTE bytesPerPixel)
{
    ULONG i;

    /* First bytesPerPixel bytes are copied as is */
    for (i = 0; i < bytesPerPixel; i++)
    {
        scanline[i] = filtered[i + 1];
    }

    /* Remaining bytes: Raw(x) = Sub(x) + Raw(x - bpp) */
    for (i = bytesPerPixel; i < lineBytes; i++)
    {
        scanline[i] = filtered[i + 1] + scanline[i - bytesPerPixel];
    }
}

/* Process a scanline with Up filter (type 2) */
void applyUpFilter(UBYTE *filtered, UBYTE *scanline, UBYTE *prevScanline,
                   ULONG lineBytes, UBYTE bytesPerPixel)
{
    ULONG i;

    /* If no previous scanline, treat as zeros */
    if (!prevScanline)
    {
        /* Just copy filtered data (minus filter byte) */
        memcpy(scanline, filtered + 1, lineBytes);
    }
    else
    {
        /* Raw(x) = Up(x) + Prior(x) */
        for (i = 0; i < lineBytes; i++)
        {
            scanline[i] = filtered[i + 1] + prevScanline[i];
        }
    }
}

/* Process a scanline with Average filter (type 3) */
void applyAverageFilter(UBYTE *filtered, UBYTE *scanline, UBYTE *prevScanline,
                        ULONG lineBytes, UBYTE bytesPerPixel)
{
    ULONG i;
    UBYTE left, up;

    /* For first line, prev is all zeros */
    /* For first bytesPerPixel bytes, left is zero */

    for (i = 0; i < lineBytes; i++)
    {
        /* Get the pixel to the left, or 0 if we're at the start */
        left = (i < bytesPerPixel) ? 0 : scanline[i - bytesPerPixel];

        /* Get the pixel above, or 0 if we're at the top */
        up = prevScanline ? prevScanline[i] : 0;

        /* Raw(x) = Average(x) + floor((Raw(x-bpp) + Prior(x))/2) */
        scanline[i] = filtered[i + 1] + ((left + up) / 2);
    }
}

/* Process a scanline with Paeth filter (type 4) */
void applyPaethFilter(UBYTE *filtered, UBYTE *scanline, UBYTE *prevScanline,
                      ULONG lineBytes, UBYTE bytesPerPixel)
{
    ULONG i;
    UBYTE left, up, upleft;

    /* For first line, prev & upleft are all zeros */
    /* For first bytesPerPixel bytes, left & upleft are zero */

    for (i = 0; i < lineBytes; i++)
    {
        /* Get the pixel to the left, or 0 if we're at the start */
        left = (i < bytesPerPixel) ? 0 : scanline[i - bytesPerPixel];

        /* If no previous scanline, treat prev values as zeros */
        if (!prevScanline)
        {
            scanline[i] = filtered[i + 1] + left; /* up and upleft are 0 */
        }
        else
        {
            /* Get the pixel above */
            up = prevScanline[i];

            /* Get the pixel to the upper left, or 0 at the start */
            upleft = (i < bytesPerPixel) ? 0 : prevScanline[i - bytesPerPixel];

            /* Raw(x) = Paeth(x) + PaethPredictor(Raw(x-bpp), Prior(x), Prior(x-bpp)) */
            scanline[i] = filtered[i + 1] + paethPredictor(left, up, upleft);
        }
    }
}

/* Main filter processing function */
BOOL applyPNGFilters(UBYTE *decompressedData, ULONG decompressedSize,
                     ULONG width, ULONG height, UBYTE bytesPerPixel,
                     UBYTE *outputData)
{
    ULONG lineBytes = width * bytesPerPixel;
    ULONG expectedSize = (lineBytes + 1) * height; /* +1 for filter type byte per line */
    ULONG row;
    UBYTE *prevScanline = NULL;
    UBYTE *filtered;
    UBYTE *scanline;
    UBYTE filterType;
    char logMessage[256];

    /* Validate input */
    if (!decompressedData || !outputData || !lineBytes)
    {
        fileLoggerAddDebugEntry("Invalid parameters for applyPNGFilters");
        return FALSE;
    }

    /* Check if the decompressed size matches what we expect */
    if (decompressedSize < expectedSize)
    {
        sprintf(logMessage, "PNG filter processing: Expected %lu bytes, got %lu bytes",
                expectedSize, decompressedSize);
        fileLoggerAddDebugEntry(logMessage);
        return FALSE;
    }

    fileLoggerAddDebugEntry("Starting PNG filter processing");

    /* Process each scanline */
    for (row = 0; row < height; row++)
    {
        filtered = decompressedData + row * (lineBytes + 1);
        scanline = outputData + row * lineBytes;

        /* Get the filter type from the first byte */
        filterType = filtered[0];

        /* Apply the appropriate filter */
        switch (filterType)
        {
        case PNG_FILTER_NONE:
            applyNoneFilter(filtered, scanline, prevScanline, lineBytes, bytesPerPixel);
            break;

        case PNG_FILTER_SUB:
            applySubFilter(filtered, scanline, prevScanline, lineBytes, bytesPerPixel);
            break;

        case PNG_FILTER_UP:
            applyUpFilter(filtered, scanline, prevScanline, lineBytes, bytesPerPixel);
            break;

        case PNG_FILTER_AVERAGE:
            applyAverageFilter(filtered, scanline, prevScanline, lineBytes, bytesPerPixel);
            break;

        case PNG_FILTER_PAETH:
            applyPaethFilter(filtered, scanline, prevScanline, lineBytes, bytesPerPixel);
            break;

        default:
            sprintf(logMessage, "Unknown PNG filter type: %d in row %lu", (int)filterType, (unsigned long)row);
            fileLoggerAddDebugEntry(logMessage);
            return FALSE;
        }

        /* Current scanline becomes previous for next iteration */
        prevScanline = scanline;
    }

    fileLoggerAddDebugEntry("PNG filter processing completed successfully");
    return TRUE;
}
