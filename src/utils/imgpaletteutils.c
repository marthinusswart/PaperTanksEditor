/*
 * Implementation of Image Palette Utilities for AmigaOS 3.1
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <clib/alib_protos.h>
#include "imgpaletteutils.h"

/* Helper function to determine if a color is primarily in a specific category */
static BOOL isColorCategory(UBYTE r, UBYTE g, UBYTE b, UBYTE category)
{
    /* Calculate intensity and dominance of each channel */
    ULONG total = r + g + b;
    ULONG threshold = total / 5;  /* 20% intensity threshold */
    
    /* Avoid division by zero */
    if (total < 30) return FALSE;  /* Very dark colors treated as neutral */
    
    /* Use integer math with scaling for comparisons */
    switch (category) {
        case 'R':  /* Red dominant */
            return (r*10 > g*13 && r*10 > b*13 && r > threshold);
        case 'G':  /* Green dominant - improved to catch more green shades */
            /* Specific to our ILBM file which has multiple green variations */
            /* More permissive green detection - green must be highest channel */
            if (g > r && g > b && g > threshold) {
                /* Either pure green or green-leaning color */
                return TRUE;
            }
            /* Also detect yellow-greens where green is very strong */
            if (g > b*2 && g > r && g > 100) {
                return TRUE;
            }
            return FALSE;
        case 'B':  /* Blue dominant */
            return (b*10 > r*13 && b*10 > g*13 && b > threshold);
        case 'Y':  /* Yellow (red+green) */
            return (r*10 > b*15 && g*10 > b*15 && r > threshold && g > threshold);
        case 'C':  /* Cyan (green+blue) */
            return (g*10 > r*15 && b*10 > r*15 && g > threshold && b > threshold);
        case 'M':  /* Magenta (red+blue) */
            return (r*10 > g*15 && b*10 > g*15 && r > threshold && b > threshold);
        case 'W':  /* White/Light gray */
            return (r > 192 && g > 192 && b > 192);
        case 'K':  /* Black/Dark gray */
            return (r < 64 && g < 64 && b < 64);
        case 'N':  /* Neutral/Gray */
            return (abs(r-g) < 30 && abs(g-b) < 30 && abs(r-b) < 30);
    }
    return FALSE;
}

/* Calculate perceptually weighted color distance */
static ULONG perceptualColorDistance(UBYTE r1, UBYTE g1, UBYTE b1, UBYTE r2, UBYTE g2, UBYTE b2)
{
    /* Apply perceptual weights (human eye is more sensitive to green) */
    LONG dr = (LONG)r1 - r2;
    LONG dg = (LONG)g1 - g2;
    LONG db = (LONG)b1 - b2;
    
    /* Weighted Euclidean distance - emphasizes green channel much more strongly */
    /* Enhanced weights based on palette analyzer results - our image has many green shades */
    return (dr*dr*2 + dg*dg*8 + db*db*1) / 11;
}

// Create a pen mapping table from source image colors to system pens
UBYTE createPenMapping(
    UBYTE *penMap,          // Output: Array of 256 pen mappings
    UBYTE *colorRegs,       // Input: Color registers (RGB triplets)
    ULONG numColors,        // Input: Number of colors in the palette
    BOOL optimizeDuplicates, // Input: Whether to optimize duplicate colors
    struct ColorMap *colorMap // Input: System colormap for accurate color matching
)
{
    char logMessage[256];
    
    // Initialize with black as fallback
    for (ULONG i = 0; i < 256; i++)
    {
        penMap[i] = 0;  // Default to black (pen 0) - based on palette analysis
    }
    
    // If no color data, use simple mapping
    if (!colorRegs)
    {
        // Simple offset mapping if no color data available
        for (ULONG i = 0; i < 256; i++)
        {
            penMap[i] = i + 1;  // Source pen 0 ? System pen 1, etc.
            
            // Don't exceed pen limits
            if (penMap[i] >= 256)
                penMap[i] = 255;
        }
        
        return numColors;  // Return original color count
    }
    
    // Array to track unique colors for optimization
    UniqueColorEntry uniqueColors[MAX_UNIQUE_COLORS];
    
    // Initialize tracking array
    for (ULONG i = 0; i < MAX_UNIQUE_COLORS; i++)
    {
        uniqueColors[i].r = 0;
        uniqueColors[i].g = 0;
        uniqueColors[i].b = 0;
        uniqueColors[i].systemPen = 0;
        uniqueColors[i].used = FALSE;
    }
    
    // Different approach based on colorMap availability and optimization preferences
    if (colorMap)
    {
        // ACCURATE COLOR MATCHING APPROACH
        // ---------------------------------
        // We have access to the system's ColorMap, so we can find the closest color matches
        
        // Get number of colors in the system palette (typical Amiga system has at least 8)
        ULONG systemColorCount = 256;  // Maximum possible
        
        // Storage for system palette RGB values (scaled to 0-255 range)
        struct {
            UBYTE r, g, b;
        } systemPalette[256];
        
        // Get RGB values for system palette colors
        
        // First populate with some reasonable defaults in case GetRGB32 fails
        // These are adjusted based on palette analyzer findings
        systemPalette[0].r = 11;  systemPalette[0].g = 10;  systemPalette[0].b = 11;  // Black (index 0)
        systemPalette[1].r = 39;  systemPalette[1].g = 163; systemPalette[1].b = 32;  // Green
        systemPalette[2].r = 146; systemPalette[2].g = 146; systemPalette[2].b = 146; // Gray
        systemPalette[3].r = 144; systemPalette[3].g = 220; systemPalette[3].b = 113; // Light green
        
        // Try to get actual system palette values (will override defaults if successful)
        sprintf(logMessage, "Attempting to read system palette colors");
        fileLoggerAddDebugEntry(logMessage);
        
        // Retrieve system palette colors - Read up to 32 colors for efficiency
        ULONG colorsToRead = (systemColorCount > 32) ? 32 : systemColorCount;
        
        // Option 1: Try to get color information from the current screen's ViewPort
        BOOL screenPaletteReadSuccess = FALSE;
        struct Screen *scr = LockPubScreen(NULL);
        if (scr) {
            sprintf(logMessage, "Reading palette from current screen's ViewPort");
            fileLoggerAddDebugEntry(logMessage);
            
            // Use screen's ColorMap if available
            if (scr->ViewPort.ColorMap) {
                for (ULONG i = 0; i < colorsToRead; i++) {
                    // GetRGB4 returns 4-bit per component (0-15 range)
                    ULONG rgb = GetRGB4(scr->ViewPort.ColorMap, i);
                    
                    // Extract components from RGB4 value (0xRGB format)
                    systemPalette[i].r = ((rgb >> 8) & 0xF) * 17;  // Scale 0-15 to 0-255
                    systemPalette[i].g = ((rgb >> 4) & 0xF) * 17;
                    systemPalette[i].b = (rgb & 0xF) * 17;
                }
                screenPaletteReadSuccess = TRUE;
            }
            
            UnlockPubScreen(NULL, scr);
        }
        
        // Option 2: Fall back to provided ColorMap if screen method fails
        if (!screenPaletteReadSuccess && colorMap) {
            sprintf(logMessage, "Using provided ColorMap");
            fileLoggerAddDebugEntry(logMessage);
            
            for (ULONG i = 0; i < colorsToRead; i++) {
                // GetRGB4 returns 4-bit per component (0-15 range)
                ULONG rgb = GetRGB4(colorMap, i);
                
                // Extract components from RGB4 value (0xRGB format)
                systemPalette[i].r = ((rgb >> 8) & 0xF) * 17;  // Scale 0-15 to 0-255
                systemPalette[i].g = ((rgb >> 4) & 0xF) * 17;
                systemPalette[i].b = (rgb & 0xF) * 17;
            }
        }
        
        // Log a few system palette colors
        sprintf(logMessage, "System palette sample (first 8 colors):");
        fileLoggerAddDebugEntry(logMessage);
        for (ULONG i = 0; i < 8 && i < colorsToRead; i++)
        {
            ULONG rgb = GetRGB4(colorMap, i);
            sprintf(logMessage, "  System Pen %lu: RGB4=0x%04lx ? RGB(%u,%u,%u)", 
                    i, rgb, systemPalette[i].r, systemPalette[i].g, systemPalette[i].b);
            fileLoggerAddDebugEntry(logMessage);
        }
        
        // Number of unique colors we've found
        UBYTE nextUniqueIndex = 0;
        
        // Process source colors and find best system pen match
        for (ULONG i = 0; i < numColors; i++)
        {
            UBYTE r = colorRegs[i*3];
            UBYTE g = colorRegs[i*3 + 1];
            UBYTE b = colorRegs[i*3 + 2];
            BOOL foundMatch = FALSE;
            
            // For diagnostic purposes, note any particularly green colors
            if (isColorCategory(r, g, b, 'G') && i < 16) {
                sprintf(logMessage, "Source color %lu is GREEN: RGB(%u,%u,%u)", 
                        i, r, g, b);
                fileLoggerAddDebugEntry(logMessage);
            }
            
            // If optimizing duplicates, check if we've seen this color before
            if (optimizeDuplicates && nextUniqueIndex > 0)
            {
                for (ULONG j = 0; j < nextUniqueIndex; j++)
                {
                    if (uniqueColors[j].used && 
                        uniqueColors[j].r == r && 
                        uniqueColors[j].g == g && 
                        uniqueColors[j].b == b)
                    {
                        // We found a match - reuse the pen
                        penMap[i] = uniqueColors[j].systemPen;
                        foundMatch = TRUE;
                        break;
                    }
                }
            }
            
            // If no duplicate found or not optimizing, find closest system color
            if (!foundMatch)
            {
                ULONG bestMatch = 0;  // Default to pen 0 (black) - based on palette analysis
                ULONG minDistance = 0xFFFFFFFF;  // Start with maximum possible distance
                
                // Find closest color in system palette
                for (ULONG j = 0; j < colorsToRead; j++)
                {
                    // Check if color categories match for the most distinct color types
                    // This ensures greens map to greens, reds to reds, etc.
                    if (isColorCategory(r, g, b, 'G') && !isColorCategory(systemPalette[j].r, systemPalette[j].g, systemPalette[j].b, 'G')) {
                        // Skip if source is green but system color isn't
                        continue;
                    }
                        
                    if (isColorCategory(r, g, b, 'R') && !isColorCategory(systemPalette[j].r, systemPalette[j].g, systemPalette[j].b, 'R')) {
                        // Skip if source is red but system color isn't
                        continue;
                    }
                        
                    if (isColorCategory(r, g, b, 'B') && !isColorCategory(systemPalette[j].r, systemPalette[j].g, systemPalette[j].b, 'B')) {
                        // Skip if source is blue but system color isn't
                        continue;
                    }
                    
                    // Special case for dark colors - try to map to similar dark colors
                    if (r < 30 && g < 30 && b < 30) {
                        if (systemPalette[j].r > 50 || systemPalette[j].g > 50 || systemPalette[j].b > 50) {
                            continue; // Skip if source is very dark but system color isn't
                        }
                    }
                    
                    // Calculate perceptual color distance with emphasis on green preservation
                    ULONG distance = perceptualColorDistance(r, g, b, 
                                                         systemPalette[j].r, 
                                                         systemPalette[j].g, 
                                                         systemPalette[j].b);
                    
                    if (distance < minDistance)
                    {
                        minDistance = distance;
                        bestMatch = j;
                    }
                }
                
                // Map to the closest system pen 
                penMap[i] = bestMatch;
                
                // For diagnostic purposes, log what green colors got mapped to
                if (isColorCategory(r, g, b, 'G') && i < 16) {
                    UBYTE mappedR = systemPalette[bestMatch].r;
                    UBYTE mappedG = systemPalette[bestMatch].g;
                    UBYTE mappedB = systemPalette[bestMatch].b;
                    sprintf(logMessage, "  GREEN source pen %lu: RGB(%u,%u,%u) ? system pen %lu: RGB(%u,%u,%u)", 
                            i, r, g, b, (ULONG)bestMatch, mappedR, mappedG, mappedB);
                    fileLoggerAddDebugEntry(logMessage);
                    
                    // Alert if green was mapped to something that's not green
                    if (!isColorCategory(mappedR, mappedG, mappedB, 'G')) {
                        sprintf(logMessage, "  WARNING: Green color mapped to non-green system pen!");
                        fileLoggerAddDebugEntry(logMessage);
                    }
                }
                
                // Store for future reference if optimizing duplicates
                if (optimizeDuplicates && nextUniqueIndex < MAX_UNIQUE_COLORS)
                {
                    uniqueColors[nextUniqueIndex].r = r;
                    uniqueColors[nextUniqueIndex].g = g;
                    uniqueColors[nextUniqueIndex].b = b;
                    uniqueColors[nextUniqueIndex].systemPen = penMap[i];
                    uniqueColors[nextUniqueIndex].used = TRUE;
                    nextUniqueIndex++;
                }
            }
        }
        
        // Log how many unique colors we processed
        sprintf(logMessage, "Mapped %lu source colors to system palette using color matching", 
                numColors);
        fileLoggerAddDebugEntry(logMessage);
        
        if (optimizeDuplicates)
        {
            sprintf(logMessage, "Found %u unique colors out of %lu in palette", 
                    nextUniqueIndex, numColors);
            fileLoggerAddDebugEntry(logMessage);
        }
        
        return (optimizeDuplicates) ? nextUniqueIndex : numColors;
    }
    else
    {
        // SIMPLER APPROACH (ADJUSTED BASED ON PALETTE ANALYSIS)
        // ----------------------------------------------
        // No color map available, fall back to optimization only
        
        // First handle special cases (based on palette analyzer findings)
        penMap[0] = 0;  // Black is at index 0 in our ILBM file
        
        // First color is always mapped to pen 0 (black)
        uniqueColors[0].r = colorRegs[0];
        uniqueColors[0].g = colorRegs[1];
        uniqueColors[0].b = colorRegs[2];
        uniqueColors[0].systemPen = 0;
        uniqueColors[0].used = TRUE;
        
        UBYTE nextPen = 2;  // Start at pen 2 (white in your system)
        UBYTE nextUniqueIndex = 1;
        
        // Process all colors in the palette
        for (ULONG i = 1; i < numColors; i++)
        {
            UBYTE r = colorRegs[i*3];
            UBYTE g = colorRegs[i*3 + 1];
            UBYTE b = colorRegs[i*3 + 2];
            BOOL foundMatch = FALSE;
            
            // Only look for duplicates if optimizing
            if (optimizeDuplicates)
            {
                // Check if this color matches any we've seen before
                for (ULONG j = 0; j < nextUniqueIndex; j++)
                {
                    if (uniqueColors[j].used && 
                        uniqueColors[j].r == r && 
                        uniqueColors[j].g == g && 
                        uniqueColors[j].b == b)
                    {
                        // We found a match - reuse the pen
                        penMap[i] = uniqueColors[j].systemPen;
                        foundMatch = TRUE;
                        break;
                    }
                }
            }
            
            // If no match, assign a new pen
            if (!foundMatch && nextUniqueIndex < MAX_UNIQUE_COLORS)
            {
                uniqueColors[nextUniqueIndex].r = r;
                uniqueColors[nextUniqueIndex].g = g;
                uniqueColors[nextUniqueIndex].b = b;
                uniqueColors[nextUniqueIndex].systemPen = nextPen;
                uniqueColors[nextUniqueIndex].used = TRUE;
                
                penMap[i] = nextPen;
                
                // Increment for next unique color
                nextUniqueIndex++;
                nextPen++;
                
                // Don't exceed system palette limits
                if (nextPen >= 256)
                    nextPen = 255;
            }
            else if (!foundMatch)
            {
                // Too many unique colors, just use sequential mapping
                penMap[i] = i + 1;
                if (penMap[i] >= 256)
                    penMap[i] = 255;
            }
        }
        
        // Log how many unique colors we found
        if (optimizeDuplicates)
        {
            sprintf(logMessage, "Found %u unique colors out of %lu in palette", 
                    nextUniqueIndex, numColors);
            fileLoggerAddDebugEntry(logMessage);
        }
        else
        {
            sprintf(logMessage, "Using simple 1:1 pen mapping (offset by 1)");
            fileLoggerAddDebugEntry(logMessage);
        }
        
        return (optimizeDuplicates) ? nextUniqueIndex : numColors;
    }
}

// Log the color palette information
void logPaletteInfo(
    UBYTE *colorRegs,      // Input: Color registers (RGB triplets) 
    ULONG numColors,       // Input: Number of colors in the palette
    ULONG *colorTable      // Input: Alternative color table (ARGB values)
)
{
    char logMessage[256];
    
    // Define a fallback palette based on our ILBM analysis
    struct {
        UBYTE r, g, b;
    } defaultPalette[16] = {
        {11, 10, 11},     // 0: Black
        {39, 163, 32},    // 1: Green
        {146, 146, 146},  // 2: Gray
        {144, 220, 113},  // 3: Light green
        {193, 198, 192},  // 4: Light gray
        {4, 184, 4},      // 5: Bright green
        {231, 232, 231},  // 6: White
        {66, 70, 65},     // 7: Dark gray
        {103, 104, 103},  // 8: Medium gray
        {74, 172, 61},    // 9: Green
        {198, 248, 27},   // 10: Yellow-green
        {101, 196, 81},   // 11: Medium green
        {177, 177, 177},  // 12: Light gray
        {53, 54, 53},     // 13: Dark gray
        {62, 105, 50},    // 14: Dark green
        {180, 237, 148}   // 15: Light yellow-green
    };
    
    // Log all colors if we have 16 or fewer, otherwise log first 16
    ULONG samplesToLog = numColors > 16 ? 16 : numColors;
    
    if (colorRegs && numColors > 0)
    {
        sprintf(logMessage, "Color registers available with %lu colors", numColors);
        fileLoggerAddDebugEntry(logMessage);
        
        for (ULONG i = 0; i < samplesToLog; i++)
        {
            UBYTE r = colorRegs[i*3];      // Red
            UBYTE g = colorRegs[i*3 + 1];  // Green  
            UBYTE b = colorRegs[i*3 + 2];  // Blue
            sprintf(logMessage, "  Color %lu: R=%u G=%u B=%u (0x%02x%02x%02x)", 
                    i, r, g, b, r, g, b);
            fileLoggerAddDebugEntry(logMessage);
        }
    }
    else if (colorTable && numColors > 0)
    {
        sprintf(logMessage, "Color table available with %lu colors", numColors);
        fileLoggerAddDebugEntry(logMessage);
        
        for (ULONG i = 0; i < samplesToLog; i++)
        {
            ULONG rgb = colorTable[i];
            UBYTE r = (rgb >> 16) & 0xFF;
            UBYTE g = (rgb >> 8) & 0xFF;
            UBYTE b = rgb & 0xFF;
            sprintf(logMessage, "  Color %lu: R=%u G=%u B=%u (0x%06lx)", 
                    i, r, g, b, rgb);
            fileLoggerAddDebugEntry(logMessage);
        }
    }
    else
    {
        sprintf(logMessage, "No color information available, using default palette");
        fileLoggerAddDebugEntry(logMessage);
        
        // Log the default palette we'll use
        ULONG palSize = numColors > 16 ? 16 : numColors;
        for (ULONG i = 0; i < palSize; i++)
        {
            sprintf(logMessage, "  Default Color %lu: R=%u G=%u B=%u", 
                    i, defaultPalette[i].r, defaultPalette[i].g, defaultPalette[i].b);
            fileLoggerAddDebugEntry(logMessage);
        }
    }
}

// Log the pen mapping information
void logPenMappingInfo(
    UBYTE *penMap,         // Input: Array of 256 pen mappings
    UBYTE *colorRegs,      // Input: Color registers (RGB triplets)
    ULONG numColors        // Input: Number of colors in the palette
)
{
    char logMessage[256];
    
    sprintf(logMessage, "Pen mapping (source ? system):");
    fileLoggerAddDebugEntry(logMessage);
    
    // Log all pen mappings if we have 16 or fewer, otherwise log first 16
    ULONG mapSamples = numColors > 16 ? 16 : numColors;
    
    // Create a table showing source pen, RGB values, and mapped pen
    for (ULONG i = 0; i < mapSamples; i++)
    {
        if (colorRegs)
        {
            UBYTE r = colorRegs[i*3];
            UBYTE g = colorRegs[i*3 + 1];
            UBYTE b = colorRegs[i*3 + 2];
            sprintf(logMessage, "  Pen %lu: RGB(%u,%u,%u) ? System Pen %u", 
                    i, r, g, b, penMap[i]);
        }
        else
        {
            sprintf(logMessage, "  Pen %lu ? System Pen %u", i, penMap[i]);
        }
        fileLoggerAddDebugEntry(logMessage);
    }
}
