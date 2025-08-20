/*
 * Basic zlib utilities for AmigaOS 3.1
 * Used for decompressing zlib data in PNG files
 */

#ifndef ZLIBUTILS_H
#define ZLIBUTILS_H

#include <exec/types.h>

/* Function to decompress zlib-compressed data */
BOOL decompressZlibData(UBYTE *compressedData, ULONG compressedSize, UBYTE **decompressedData, ULONG *decompressedSize);

#endif /* ZLIBUTILS_H */
