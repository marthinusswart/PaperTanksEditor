/*
 * Basic zlib utilities for AmigaOS 3.1
 * Used for decompressing zlib data in PNG files
 */

#ifndef ZLIBUTILS_H
#define ZLIBUTILS_H

#include <exec/types.h>

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

/* Process an uncompressed (type 0) DEFLATE block */
BOOL processUncompressedBlock(BitBuffer *bitBuf, UBYTE *compressedData, ULONG compressedSize,
                              UBYTE *outputBuffer, ULONG outputBufferSize, ULONG *outPos);

/* Process a dynamic Huffman (type 2) DEFLATE block */
BOOL processSkipUnsupportedBlock(BitBuffer *bitBuf, BOOL isFinalBlock, const char *blockTypeName);

/* Process a dynamic Huffman (type 2) DEFLATE block - actual implementation */
BOOL processDynamicHuffmanBlock(BitBuffer *bitBuf, UBYTE *compressedData, ULONG compressedSize,
                                UBYTE *outputBuffer, ULONG outputBufferSize, ULONG *outPos);

/* Initialize a bit buffer for reading compressed data */
void initBitBuffer(BitBuffer *buffer, UBYTE *data, ULONG size, ULONG startPos);

/* Read bits from the bit buffer (LSB first) */
ULONG readBits(BitBuffer *buffer, UBYTE numBits);

#endif /* ZLIBUTILS_H */