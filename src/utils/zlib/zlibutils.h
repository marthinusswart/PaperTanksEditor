/*
 * Basic zlib utilities for AmigaOS 3.1
 * Used for decompressing zlib data in PNG files
 */

#ifndef ZLIBUTILS_H
#define ZLIBUTILS_H

#include <exec/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "../filelogger.h"

/* Bit buffer structure for reading bits from a byte stream */
typedef struct BitBuffer
{
    UBYTE *data;    /* Pointer to the data buffer */
    ULONG size;     /* Size of the data buffer in bytes */
    ULONG pos;      /* Current byte position in the buffer */
    UBYTE bitPos;   /* Current bit position in the current byte (0-7) */
    ULONG bitCount; /* Number of bits read so far */
} BitBuffer;

/* Function to decompress zlib-compressed data */
BOOL decompressZlibData(UBYTE *compressedData, ULONG compressedSize, UBYTE **decompressedData, ULONG *decompressedSize);

/* Function to process zlib header */
BOOL processZlibHeader(UBYTE *compressedData, ULONG compressedSize, UBYTE *compressionMethod, UBYTE *compressionInfo,
                       UBYTE *fCheck, BOOL *hasDictionary, UBYTE *compressionLevel);

/* Function to inflate/decompress DEFLATE compressed data */
BOOL inflateData(UBYTE *compressedData, ULONG compressedSize, ULONG startPos, UBYTE *outputBuffer,
                 ULONG outputBufferSize, ULONG *bytesWritten);

/* Function to calculate Adler-32 checksum */
ULONG calculateAdler32(UBYTE *data, ULONG length);

/* Function to verify Adler-32 checksum in ZLIB data */
BOOL verifyAdler32Checksum(UBYTE *compressedData, ULONG compressedSize, UBYTE *decompressedData, ULONG decompressedSize);

/* Process an uncompressed (type 0) DEFLATE block */
BOOL processUncompressedBlock(BitBuffer *bitBuf, UBYTE *compressedData, ULONG compressedSize,
                              UBYTE *outputBuffer, ULONG outputBufferSize, ULONG *outPos);

/* Skip an unsupported DEFLATE block type */
BOOL processSkipUnsupportedBlock(BitBuffer *bitBuf, BOOL isFinalBlock, const char *blockTypeName);

/* Initialize a bit buffer for reading compressed data */
void initBitBuffer(BitBuffer *buffer, UBYTE *data, ULONG size, ULONG startPos);

/* Read bits from the bit buffer (LSB first) */
BOOL readBits(BitBuffer *buffer, UBYTE numBits, UBYTE *value);

#endif /* ZLIBUTILS_H */