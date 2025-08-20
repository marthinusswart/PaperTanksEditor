/*
 * Basic zlib utilities for AmigaOS 3.1
 * Used for decompressing zlib data in PNG files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "zlibutils.h"
#include "huffmanUtils.h"
#include "filelogger.h"

/* Process an unsupported block type (fixed or dynamic Huffman) by skipping it */
BOOL processSkipUnsupportedBlock(BitBuffer *bitBuf, BOOL isFinalBlock, const char *blockTypeName)
{
    char logMessage[256];

    sprintf(logMessage, "%s Huffman codes not implemented in this version", blockTypeName);
    fileLoggerAddDebugEntry(logMessage);

    /* Skip this block and move to the next one */
    if (isFinalBlock)
    {
        /* If this is the final block, we should stop */
        fileLoggerAddDebugEntry("Final block has unsupported compression type");
        return FALSE;
    }

    /* Skip a few bytes to hopefully find the next block */
    bitBuf->pos++;
    if (bitBuf->bitPos > 0)
    {
        bitBuf->bitPos = 0;
        bitBuf->pos++;
    }

    return TRUE;
}

/* Process an uncompressed (type 0) DEFLATE block */
BOOL processUncompressedBlock(BitBuffer *bitBuf, UBYTE *compressedData, ULONG compressedSize,
                              UBYTE *outputBuffer, ULONG outputBufferSize, ULONG *outPos)
{
    ULONG len, nlen;
    char logMessage[256];

    /* Skip to byte boundary */
    if (bitBuf->bitPos > 0)
    {
        bitBuf->bitPos = 0;
        bitBuf->pos++;
    }

    /* Make sure we have enough data for the block header */
    if (bitBuf->pos + 4 > compressedSize)
    {
        fileLoggerAddDebugEntry("Not enough data for uncompressed block header");
        return FALSE;
    }

    /* Read length and complement (little-endian) */
    len = compressedData[bitBuf->pos] | (compressedData[bitBuf->pos + 1] << 8);
    nlen = compressedData[bitBuf->pos + 2] | (compressedData[bitBuf->pos + 3] << 8);
    bitBuf->pos += 4;

    /* Verify length with complement */
    if ((len ^ 0xFFFF) != nlen)
    {
        sprintf(logMessage, "Invalid length in uncompressed block: len=%lu, nlen=%lu", len, nlen);
        fileLoggerAddDebugEntry(logMessage);
        return FALSE;
    }

    sprintf(logMessage, "Uncompressed block: length=%lu", len);
    fileLoggerAddDebugEntry(logMessage);

    /* Copy uncompressed data */
    if (bitBuf->pos + len > compressedSize)
    {
        fileLoggerAddDebugEntry("Not enough input data for uncompressed block");
        return FALSE;
    }

    if (*outPos + len > outputBufferSize)
    {
        fileLoggerAddDebugEntry("Output buffer too small for uncompressed data");
        return FALSE;
    }

    /* Copy the data */
    memcpy(outputBuffer + *outPos, compressedData + bitBuf->pos, len);
    bitBuf->pos += len;
    *outPos += len;

    return TRUE;
}

/* Initialize a bit buffer for reading compressed data */
void initBitBuffer(BitBuffer *buffer, UBYTE *data, ULONG size, ULONG startPos)
{
    buffer->data = data;
    buffer->size = size;
    buffer->pos = startPos;
    buffer->bitPos = 0;
    buffer->bitCount = 0;
}

/* Read bits from the bit buffer (LSB first) */
BOOL readBits(BitBuffer *buffer, UBYTE numBits, UBYTE *value)
{
    ULONG result = 0;
    UBYTE bitsRead = 0;

    /* Make sure we don't read past the end of the buffer */
    if (buffer->pos >= buffer->size || !value)
        return FALSE;

    /* Read bits one at a time */
    while (bitsRead < numBits)
    {
        /* Read the current bit */
        UBYTE bit = (buffer->data[buffer->pos] >> buffer->bitPos) & 1;

        /* Add the bit to our result */
        result |= (bit << bitsRead);

        /* Move to the next bit */
        buffer->bitPos++;
        buffer->bitCount++;
        bitsRead++;

        /* If we've read all bits in the current byte, move to the next byte */
        if (buffer->bitPos == 8)
        {
            buffer->bitPos = 0;
            buffer->pos++;

            /* Check if we've reached the end of the buffer */
            if (buffer->pos >= buffer->size && bitsRead < numBits)
                return FALSE; /* Not enough bits available */
        }
    }

    *value = (UBYTE)result;
    return TRUE;
}

/* Process the zlib header (first 2 bytes)
 * RFC 1950 - ZLIB Compressed Data Format Specification
 *
 * CMF (byte 0):
 *   - bits 0-3: Compression Method (CM) - usually 8 for DEFLATE
 *   - bits 4-7: Compression Info (CINFO) - window size
 *
 * FLG (byte 1):
 *   - bits 0-4: FCHECK (check bits for CMF and FLG)
 *   - bit 5: FDICT (preset dictionary)
 *   - bits 6-7: FLEVEL (compression level)
 */
BOOL processZlibHeader(UBYTE *compressedData, ULONG compressedSize, UBYTE *compressionMethod, UBYTE *compressionInfo,
                       UBYTE *fCheck, BOOL *hasDictionary, UBYTE *compressionLevel)
{
    char logMessage[256];

    /* Validate input parameters */
    if (!compressedData || compressedSize < 2 || !compressionMethod || !compressionInfo ||
        !fCheck || !hasDictionary || !compressionLevel)
    {
        fileLoggerAddDebugEntry("Invalid parameters for processZlibHeader");
        return FALSE;
    }

    /* Extract CMF (first byte) */
    UBYTE cmf = compressedData[0];
    *compressionMethod = cmf & 0x0F;      /* Bits 0-3 */
    *compressionInfo = (cmf >> 4) & 0x0F; /* Bits 4-7 */

    /* Extract FLG (second byte) */
    UBYTE flg = compressedData[1];
    *fCheck = flg & 0x1F;                  /* Bits 0-4 */
    *hasDictionary = (flg & 0x20) != 0;    /* Bit 5 */
    *compressionLevel = (flg >> 6) & 0x03; /* Bits 6-7 */

    /* Validate header according to RFC 1950 */
    /* Check if the compression method is 8 (DEFLATE) */
    if (*compressionMethod != 8)
    {
        sprintf(logMessage, "Unsupported compression method: %d (expected 8 for DEFLATE)",
                *compressionMethod);
        fileLoggerAddDebugEntry(logMessage);
        return FALSE;
    }

    /* Validate window size - CINFO must be between 0-7 for DEFLATE */
    if (*compressionInfo > 7)
    {
        sprintf(logMessage, "Invalid compression info (window size): %d", *compressionInfo);
        fileLoggerAddDebugEntry(logMessage);
        return FALSE;
    }

    /* Validate checksum - CMF*256 + FLG must be a multiple of 31 */
    ULONG checksum = cmf * 256 + flg;
    if (checksum % 31 != 0)
    {
        sprintf(logMessage, "Invalid zlib header checksum: %lu is not divisible by 31",
                checksum);
        fileLoggerAddDebugEntry(logMessage);
        return FALSE;
    }

    /* Log header information */
    sprintf(logMessage, "Zlib header: CM=%d, CINFO=%d, FCHECK=%d, FDICT=%d, FLEVEL=%d",
            *compressionMethod, *compressionInfo, *fCheck, *hasDictionary, *compressionLevel);
    fileLoggerAddDebugEntry(logMessage);

    /* If a preset dictionary is present, we need to handle that */
    if (*hasDictionary)
    {
        fileLoggerAddDebugEntry("Warning: Preset dictionary is not supported in this implementation");
        /* In a complete implementation, we would check for a 4-byte ADLER32 checksum
           of the dictionary that follows the header */
    }

    return TRUE;
}

/* SIMPLIFIED IMPLEMENTATION - In a real system we would:
 * 1. Process zlib header (CMF, FLG) - DONE
 * 2. Process deflate blocks (using Huffman decoding)
 * 3. Verify the Adler-32 checksum at the end
 *
 * For this proof of concept, we'll just do minimal processing
 * and return an empty buffer with a success status */

/* Constants for DEFLATE decompression */
#define BLOCK_TYPE_NONE 0    /* No compression */
#define BLOCK_TYPE_FIXED 1   /* Fixed Huffman codes */
#define BLOCK_TYPE_DYNAMIC 2 /* Dynamic Huffman codes */
#define BLOCK_TYPE_ERROR 3   /* Reserved (error) */

/* Basic implementation of DEFLATE decompression algorithm
 * This is a simplified version that handles the most common case:
 * uncompressed blocks (type 0) and non-compressed data
 */
BOOL inflateData(UBYTE *compressedData, ULONG compressedSize, ULONG startPos, UBYTE *outputBuffer,
                 ULONG outputBufferSize, ULONG *bytesWritten)
{
    char logMessage[256];
    ULONG outPos = 0; /* Current position in output */
    BOOL isFinalBlock = FALSE;
    UBYTE blockType, bitValue;
    ULONG len, nlen;
    BitBuffer bitBuf;

    /* Initialize bit buffer */
    initBitBuffer(&bitBuf, compressedData, compressedSize, startPos);

    *bytesWritten = 0;

    fileLoggerAddDebugEntry("Starting DEFLATE decompression");

    /* Process blocks until we find the final block */
    while (!isFinalBlock && bitBuf.pos < compressedSize && outPos < outputBufferSize)
    {
        /* Read block header (3 bits) */
        if (!readBits(&bitBuf, 1, &bitValue))
            return FALSE;
        isFinalBlock = bitValue != 0;

        if (!readBits(&bitBuf, 2, &blockType))
            return FALSE;

        sprintf(logMessage, "Block header: Final=%d, Type=%d", isFinalBlock, blockType);
        fileLoggerAddDebugEntry(logMessage);

        /* Process based on block type */
        switch (blockType)
        {
        case BLOCK_TYPE_NONE: /* Uncompressed block */
            if (!processUncompressedBlock(&bitBuf, compressedData, compressedSize, outputBuffer, outputBufferSize, &outPos))
            {
                return FALSE;
            }
            break;

        case BLOCK_TYPE_FIXED: /* Fixed Huffman codes */
            if (!processFixedHuffmanBlock(&bitBuf, compressedData, compressedSize, outputBuffer, outputBufferSize, &outPos))
            {
                return FALSE;
            }
            break;

        case BLOCK_TYPE_DYNAMIC: /* Dynamic Huffman codes */
            if (!processDynamicHuffmanBlock(&bitBuf, compressedData, compressedSize, outputBuffer, outputBufferSize, &outPos))
            {
                return FALSE;
            }
            break;

        default: /* Error or unknown block type */
            fileLoggerAddDebugEntry("Invalid block type");
            return FALSE;
        }
    }

    *bytesWritten = outPos;

    sprintf(logMessage, "DEFLATE decompression completed: %lu bytes decompressed", outPos);
    fileLoggerAddDebugEntry(logMessage);

    return TRUE;
}

/* Basic zlib data decompression function
 * This is a very simplified implementation that handles the bare minimum
 * needed to decompress zlib-compressed data in PNG files
 */
BOOL decompressZlibData(UBYTE *compressedData, ULONG compressedSize, UBYTE **decompressedData, ULONG *decompressedSize)
{
    char logMessage[256];
    UBYTE compressionMethod = 0;
    UBYTE compressionInfo = 0;
    UBYTE fCheck = 0;
    BOOL hasDictionary = FALSE;
    UBYTE compressionLevel = 0;

    /* Validate parameters */
    if (!compressedData || compressedSize == 0 || !decompressedData || !decompressedSize)
    {
        fileLoggerAddDebugEntry("Invalid parameters for zlib decompression");
        return FALSE;
    }

    /* Process the zlib header (first 2 bytes) */
    if (!processZlibHeader(compressedData, compressedSize, &compressionMethod, &compressionInfo,
                           &fCheck, &hasDictionary, &compressionLevel))
    {
        fileLoggerAddDebugEntry("Failed to process zlib header");
        return FALSE;
    }

    /* For now, allocate a buffer for decompressed data
     * In a real implementation, we would dynamically resize this
     * as needed during decompression */
    ULONG estimatedSize = compressedSize * 10; /* Increase buffer size for larger expansion ratios */
    *decompressedData = (UBYTE *)malloc(estimatedSize);
    if (!*decompressedData)
    {
        fileLoggerAddDebugEntry("Failed to allocate memory for decompressed data");
        return FALSE;
    }

    fileLoggerAddDebugEntry("Starting simple zlib decompression");

    /* Skip zlib header (2 bytes) and initialize some data */
    ULONG srcPos = 2;
    ULONG dstPos = 0;

    /* Handle dictionary if present (4 more bytes to skip) */
    if (hasDictionary && compressedSize >= 6)
    {
        /* Dictionary ID is 4 bytes */
        srcPos += 4;
        fileLoggerAddDebugEntry("Skipping 4-byte dictionary ID");
    }

    /* Call the inflate function to decompress the data */
    if (!inflateData(compressedData, compressedSize, srcPos, *decompressedData,
                     estimatedSize, decompressedSize))
    {
        fileLoggerAddDebugEntry("DEFLATE decompression failed");
        free(*decompressedData);
        *decompressedData = NULL;
        return FALSE;
    }

    sprintf(logMessage, "Successful decompression: %lu bytes compressed, %lu bytes decompressed",
            compressedSize, *decompressedSize);
    fileLoggerAddDebugEntry(logMessage);

    /* Verify Adler-32 checksum */
    if (!verifyAdler32Checksum(compressedData, compressedSize, *decompressedData, *decompressedSize))
    {
        fileLoggerAddDebugEntry("Adler-32 checksum verification failed");
        free(*decompressedData);
        *decompressedData = NULL;
        return FALSE;
    }

    fileLoggerAddDebugEntry("Adler-32 checksum verification successful");
    return TRUE;
}

/* Calculate Adler-32 checksum
 * Implementation based on RFC 1950 specification
 * Adler-32 is a checksum algorithm which is a modified version of Fletcher's checksum
 */
#define ADLER_MOD 65521 /* Largest prime smaller than 65536 */

ULONG calculateAdler32(UBYTE *data, ULONG length)
{
    ULONG a = 1, b = 0;
    ULONG i;

    /* Process each byte */
    for (i = 0; i < length; i++)
    {
        a = (a + data[i]) % ADLER_MOD;
        b = (b + a) % ADLER_MOD;
    }

    return (b << 16) | a;
}

/* Verify Adler-32 checksum in ZLIB data
 * According to RFC 1950, the Adler-32 checksum is stored as 4 bytes
 * at the end of the ZLIB stream
 */
BOOL verifyAdler32Checksum(UBYTE *compressedData, ULONG compressedSize, UBYTE *decompressedData, ULONG decompressedSize)
{
    char logMessage[256];
    ULONG storedChecksum, calculatedChecksum;

    fileLoggerAddDebugEntry("Starting Adler-32 checksum verification");

    /* Make sure we have at least 4 bytes for the checksum at the end */
    if (compressedSize < 6) /* 2 header bytes + at least 4 checksum bytes */
    {
        sprintf(logMessage, "Compressed data too small to contain checksum: %lu bytes", compressedSize);
        fileLoggerAddDebugEntry(logMessage);
        return FALSE;
    }

    /* Log the size of the compressed and decompressed data */
    sprintf(logMessage, "Compressed size: %lu bytes, Decompressed size: %lu bytes",
            compressedSize, decompressedSize);
    fileLoggerAddDebugEntry(logMessage);

    /* Extract stored checksum (big-endian) from the end of the compressed data */
    storedChecksum = ((ULONG)compressedData[compressedSize - 4] << 24) |
                     ((ULONG)compressedData[compressedSize - 3] << 16) |
                     ((ULONG)compressedData[compressedSize - 2] << 8) |
                     (ULONG)compressedData[compressedSize - 1];

    /* Calculate checksum of decompressed data */
    calculatedChecksum = calculateAdler32(decompressedData, decompressedSize);

    sprintf(logMessage, "Adler-32 checksum: stored=0x%08lX, calculated=0x%08lX",
            storedChecksum, calculatedChecksum);
    fileLoggerAddDebugEntry(logMessage);

    /* Compare stored and calculated checksums */
    if (storedChecksum == calculatedChecksum)
    {
        fileLoggerAddDebugEntry("Adler-32 checksum verification passed");
        return TRUE;
    }
    else
    {
        fileLoggerAddDebugEntry("Adler-32 checksum verification failed - checksums don't match");
        return FALSE;
    }
}
