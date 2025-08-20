/*
 * PNG filter processing utilities for AmigaOS 3.1
 * Used to process filtered scanlines in PNG files
 */

#ifndef IMGPNGFILTERS_H
#define IMGPNGFILTERS_H

#include <exec/types.h>

/* PNG filter type constants */
#define PNG_FILTER_NONE 0
#define PNG_FILTER_SUB 1
#define PNG_FILTER_UP 2
#define PNG_FILTER_AVERAGE 3
#define PNG_FILTER_PAETH 4

/*
 * Process PNG filtered data
 * Inputs:
 *   - decompressedData: Raw decompressed data from ZLIB (with filter bytes)
 *   - decompressedSize: Size of the decompressed data
 *   - width: Image width in pixels
 *   - height: Image height in pixels
 *   - bytesPerPixel: Number of bytes per pixel (depends on color type and bit depth)
 *   - outputData: Destination buffer for unfiltered data (must be pre-allocated)
 * Returns:
 *   - TRUE if successful, FALSE otherwise
 */
BOOL applyPNGFilters(UBYTE *decompressedData, ULONG decompressedSize,
                     ULONG width, ULONG height, UBYTE bytesPerPixel,
                     UBYTE *outputData);

/*
 * Individual filter processing functions
 * Each takes:
 *   - filtered: Current filtered scanline (with filter byte)
 *   - scanline: Destination buffer for this scanline
 *   - prevScanline: Previous unfiltered scanline (NULL for first row)
 *   - lineBytes: Bytes per scanline (width * bytesPerPixel)
 *   - bytesPerPixel: Number of bytes per pixel
 */
void applyNoneFilter(UBYTE *filtered, UBYTE *scanline, UBYTE *prevScanline,
                     ULONG lineBytes, UBYTE bytesPerPixel);

void applySubFilter(UBYTE *filtered, UBYTE *scanline, UBYTE *prevScanline,
                    ULONG lineBytes, UBYTE bytesPerPixel);

void applyUpFilter(UBYTE *filtered, UBYTE *scanline, UBYTE *prevScanline,
                   ULONG lineBytes, UBYTE bytesPerPixel);

void applyAverageFilter(UBYTE *filtered, UBYTE *scanline, UBYTE *prevScanline,
                        ULONG lineBytes, UBYTE bytesPerPixel);

void applyPaethFilter(UBYTE *filtered, UBYTE *scanline, UBYTE *prevScanline,
                      ULONG lineBytes, UBYTE bytesPerPixel);

/* Paeth predictor function as defined in the PNG specification */
UBYTE paethPredictor(UBYTE a, UBYTE b, UBYTE c);

#endif /* IMGPNGFILTERS_H */
