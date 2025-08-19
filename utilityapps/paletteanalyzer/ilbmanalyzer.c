/*
 * ILBM Analyzer for AmigaOS 3.1
 * Extracts and logs details about ILBM image files,
 * with particular focus on palette information
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/datatypes.h>
#include <datatypes/pictureclass.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include "../../src/utils/filelogger.h"
#include "ilbmanalyzer.h"

// Analyze ILBM image palette and log detailed information
BOOL analyzeILBMPalette(CONST_STRPTR filename)
{
    Object *dto = NULL;
    struct BitMapHeader *bmHeader = NULL;
    ULONG *colorTable = NULL;
    UBYTE *colorRegs = NULL;
    ULONG numColors = 0;
    char logMessage[512];
    BOOL success = FALSE;

    snprintf(logMessage, sizeof(logMessage), "\n===== ILBM PALETTE ANALYSIS =====\n");
    fileLoggerAddEntry(logMessage);
    snprintf(logMessage, sizeof(logMessage), "Analyzing file: %s\n", filename);
    fileLoggerAddEntry(logMessage);

    // Try to load the image using datatypes.library
    dto = NewDTObject((APTR)filename,
                      DTA_SourceType, DTST_FILE,
                      DTA_GroupID, GID_PICTURE,
                      PDTA_Remap, FALSE,
                      TAG_DONE);
    
    if (!dto)
    {
        snprintf(logMessage, sizeof(logMessage), "ERROR: Failed to load image with datatypes\n");
        fileLoggerAddEntry(logMessage);
        return FALSE;
    }

    // Get basic image information
    GetDTAttrs(dto, 
              PDTA_BitMapHeader, &bmHeader,
              TAG_DONE);
    
    if (!bmHeader)
    {
        snprintf(logMessage, sizeof(logMessage), "ERROR: Failed to get bitmap header\n");
        fileLoggerAddEntry(logMessage);
        DisposeDTObject(dto);
        return FALSE;
    }

    snprintf(logMessage, sizeof(logMessage), "Image dimensions: %ld x %ld pixels\n", 
             bmHeader->bmh_Width, bmHeader->bmh_Height);
    fileLoggerAddEntry(logMessage);
    
    snprintf(logMessage, sizeof(logMessage), "Bit depth: %ld bits\n", bmHeader->bmh_Depth);
    fileLoggerAddEntry(logMessage);
    
    // Get colormap information - try both PDTA_ColorRegisters and PDTA_ColorTable
    GetDTAttrs(dto,
              PDTA_ColorRegisters, &colorRegs,
              PDTA_NumColors, &numColors,
              PDTA_ColorTable, &colorTable,
              TAG_DONE);
              
    snprintf(logMessage, sizeof(logMessage), "Number of colors in palette: %ld\n", numColors);
    fileLoggerAddEntry(logMessage);
    
    if (numColors > 0)
    {
        // Log palette information
        snprintf(logMessage, sizeof(logMessage), "\nColor palette details:\n");
        fileLoggerAddEntry(logMessage);
        
        snprintf(logMessage, sizeof(logMessage), "Index | RGB (Decimal) | RGB (Hex) | Color Description\n");
        fileLoggerAddEntry(logMessage);
        snprintf(logMessage, sizeof(logMessage), "------------------------------------------------------\n");
        fileLoggerAddEntry(logMessage);
        
        ULONG maxColorsToLog = (numColors > 64) ? 64 : numColors; // Limit output for large palettes
        
        if (colorRegs)
        {
            // Log from RGB triples (PDTA_ColorRegisters)
            for (ULONG i = 0; i < maxColorsToLog; i++)
            {
                UBYTE r = colorRegs[i*3];
                UBYTE g = colorRegs[i*3 + 1];
                UBYTE b = colorRegs[i*3 + 2];
                
                // Determine a simple color description
                const char *colorDesc = "Other";
                
                // Find if it's a pure or close-to-pure color
                if (r > 240 && g > 240 && b > 240) colorDesc = "White";
                else if (r < 15 && g < 15 && b < 15) colorDesc = "Black";
                else if (r > 200 && g < 50 && b < 50) colorDesc = "Red";
                else if (r < 50 && g > 200 && b < 50) colorDesc = "Green";
                else if (r < 50 && g < 50 && b > 200) colorDesc = "Blue";
                else if (r > 200 && g > 200 && b < 50) colorDesc = "Yellow";
                else if (r < 50 && g > 200 && b > 200) colorDesc = "Cyan";
                else if (r > 200 && g < 50 && b > 200) colorDesc = "Magenta";
                else if (abs(r-g) < 20 && abs(g-b) < 20 && abs(r-b) < 20) colorDesc = "Gray";
                
                snprintf(logMessage, sizeof(logMessage), "%4lu | (%3d,%3d,%3d) | #%02X%02X%02X | %s\n", 
                         i, r, g, b, r, g, b, colorDesc);
                fileLoggerAddEntry(logMessage);
            }
            
            success = TRUE;
        }
        else if (colorTable)
        {
            // Log from ARGB table (PDTA_ColorTable)
            for (ULONG i = 0; i < maxColorsToLog; i++)
            {
                ULONG argb = colorTable[i];
                UBYTE a = (argb >> 24) & 0xFF;
                UBYTE r = (argb >> 16) & 0xFF;
                UBYTE g = (argb >> 8) & 0xFF;
                UBYTE b = argb & 0xFF;
                
                // Determine a simple color description
                const char *colorDesc = "Other";
                
                // Find if it's a pure or close-to-pure color
                if (r > 240 && g > 240 && b > 240) colorDesc = "White";
                else if (r < 15 && g < 15 && b < 15) colorDesc = "Black";
                else if (r > 200 && g < 50 && b < 50) colorDesc = "Red";
                else if (r < 50 && g > 200 && b < 50) colorDesc = "Green";
                else if (r < 50 && g < 50 && b > 200) colorDesc = "Blue";
                else if (r > 200 && g > 200 && b < 50) colorDesc = "Yellow";
                else if (r < 50 && g > 200 && b > 200) colorDesc = "Cyan";
                else if (r > 200 && g < 50 && b > 200) colorDesc = "Magenta";
                else if (abs(r-g) < 20 && abs(g-b) < 20 && abs(r-b) < 20) colorDesc = "Gray";
                
                snprintf(logMessage, sizeof(logMessage), "%4lu | (%3d,%3d,%3d) | #%02X%02X%02X | %s\n", 
                         i, r, g, b, r, g, b, colorDesc);
                fileLoggerAddEntry(logMessage);
            }
            
            success = TRUE;
        }
        
        if (maxColorsToLog < numColors)
        {
            snprintf(logMessage, sizeof(logMessage), "... %ld more colors not shown\n", 
                     numColors - maxColorsToLog);
            fileLoggerAddEntry(logMessage);
        }
    }
    else
    {
        snprintf(logMessage, sizeof(logMessage), "WARNING: No color palette found or image is true color\n");
        fileLoggerAddEntry(logMessage);
    }
    
    // Clean up
    DisposeDTObject(dto);

    snprintf(logMessage, sizeof(logMessage), "===== ANALYSIS COMPLETE =====\n");
    fileLoggerAddEntry(logMessage);
    
    return success;
}
