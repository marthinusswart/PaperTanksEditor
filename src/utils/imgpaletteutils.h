/*
 * Image Palette Utilities for AmigaOS 3.1
 * Handles color palette operations and pen mapping
 */

#ifndef IMGPALETTEUTILS_H
#define IMGPALETTEUTILS_H

#include <exec/types.h>
#include <intuition/intuition.h>
#include <datatypes/pictureclass.h>
#include <graphics/view.h>
#include <proto/graphics.h>
#include "filelogger.h"

// Maximum number of unique colors we can track
#define MAX_UNIQUE_COLORS 32

// Structure to track unique colors and their pen assignments
typedef struct {
    UBYTE r, g, b;     // RGB color values
    UBYTE systemPen;   // Assigned system pen
    BOOL used;         // Whether this entry is used
} UniqueColorEntry;

// Create a pen mapping table from source image colors to system pens
// Returns the number of unique colors found
UBYTE createPenMapping(
    UBYTE *penMap,         // Output: Array of 256 pen mappings
    UBYTE *colorRegs,      // Input: Color registers (RGB triplets)
    ULONG numColors,       // Input: Number of colors in the palette
    BOOL optimizeDuplicates, // Input: Whether to optimize duplicate colors
    struct ColorMap *colorMap // Input: System colormap for accurate color matching
);

// Log the color palette information
void logPaletteInfo(
    UBYTE *colorRegs,      // Input: Color registers (RGB triplets) 
    ULONG numColors,       // Input: Number of colors in the palette
    ULONG *colorTable      // Input: Alternative color table (ARGB values)
);

// Log the pen mapping information
void logPenMappingInfo(
    UBYTE *penMap,         // Input: Array of 256 pen mappings
    UBYTE *colorRegs,      // Input: Color registers (RGB triplets)
    ULONG numColors        // Input: Number of colors in the palette
);

#endif /* IMGPALETTEUTILS_H */
