/*
 * ILBM Palette Analyzer Demo
 * Example of how to use the ILBM analyzer functions
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "../../src/utils/filelogger.h"
#include "ilbmanalyzer.h"

int main(int argc, char **argv)
{
    char *filename = "assets/disk-space.ilbm";  // Default file to analyze
    
    // Initialize logger
    fileLoggerInit("paletteanalysis.log");
    
    // If a filename was provided, use it instead
    if (argc > 1) 
    {
        filename = argv[1];
    }
    
    // Analyze the ILBM palette
    printf("Analyzing ILBM file: %s\n", filename);
    printf("See paletteanalysis.log for detailed results\n");
    
    if (!analyzeILBMPalette(filename))
    {
        printf("Failed to analyze ILBM palette\n");
    }
    
    return 0;
}
