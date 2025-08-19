/*
 * ILBM Analyzer for AmigaOS 3.1
 * Header file for analyzing ILBM image files
 */

#ifndef ILBMANALYZER_H
#define ILBMANALYZER_H

#include <exec/types.h>

// Analyze ILBM image palette and log detailed information
BOOL analyzeILBMPalette(CONST_STRPTR filename);

#endif /* ILBMANALYZER_H */
