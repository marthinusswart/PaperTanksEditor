/*
 * Huffman coding utilities for AmigaOS 3.1
 * Used for decompressing DEFLATE compressed data in PNG files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "huffmanUtils.h"
#include "zlibutils.h"
#include "filelogger.h"

/* Code length code order according to RFC 1951 */
static const UBYTE codelenCodeOrder[MAX_CODE_LENGTHS] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

/* Length and distance lookup tables for LZ77 decompression
 * According to RFC 1951 (DEFLATE) section 3.2.5
 */
static const UWORD lengthBase[29] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};

static const UBYTE lengthExtraBits[29] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};

static const UWORD distanceBase[30] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
    8193, 12289, 16385, 24577};

static const UBYTE distanceExtraBits[30] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

/* Get the code length code order for dynamic Huffman decoding */
const UBYTE *getCodeLengthCodeOrder(void)
{
    return codelenCodeOrder;
}

/* Process a dynamic Huffman (type 2) DEFLATE block - actual implementation */
BOOL processDynamicHuffmanBlock(BitBuffer *bitBuf, UBYTE *compressedData, ULONG compressedSize,
                                UBYTE *outputBuffer, ULONG outputBufferSize, ULONG *outPos)
{
    char logMessage[256];
    ULONG hlit, hdist, hclen;
    UBYTE codeLengths[MAX_CODE_LENGTHS];
    UBYTE literalLengths[MAX_LITERAL_CODES];
    UBYTE distanceLengths[MAX_DISTANCE_CODES];
    ULONG i, j, repeat;
    UWORD decodedValue;
    UBYTE bitValue;
    const UBYTE *codelenCodeOrder = getCodeLengthCodeOrder();

    /* Initialize all code lengths to 0 */
    for (i = 0; i < MAX_CODE_LENGTHS; i++)
        codeLengths[i] = 0;

    for (i = 0; i < MAX_LITERAL_CODES; i++)
        literalLengths[i] = 0;

    for (i = 0; i < MAX_DISTANCE_CODES; i++)
        distanceLengths[i] = 0;

    /* Read the header of the dynamic Huffman block */
    if (!readBits(bitBuf, 5, &bitValue))
        return FALSE;
    hlit = bitValue + 257; /* Number of literal/length codes (257-286) */

    if (!readBits(bitBuf, 5, &bitValue))
        return FALSE;
    hdist = bitValue + 1; /* Number of distance codes (1-32) */

    if (!readBits(bitBuf, 4, &bitValue))
        return FALSE;
    hclen = bitValue + 4; /* Number of code length codes (4-19) */

    sprintf(logMessage, "Dynamic Huffman block: HLIT=%lu, HDIST=%lu, HCLEN=%lu", hlit, hdist, hclen);
    fileLoggerAddDebugEntry(logMessage);

    /* Validate the ranges */
    if (hlit > MAX_LITERAL_CODES || hdist > MAX_DISTANCE_CODES)
    {
        fileLoggerAddDebugEntry("Invalid dynamic Huffman code counts");
        return FALSE;
    }

    /* Read code lengths for the code length alphabet */
    for (i = 0; i < hclen; i++)
    {
        if (!readBits(bitBuf, 3, &bitValue))
            return FALSE;
        codeLengths[codelenCodeOrder[i]] = bitValue;
    }

    /* Build the Huffman tree for the code length alphabet */
    HuffmanTable codeLengthTable;
    if (!buildHuffmanTreeFromCodeLengths(codeLengths, MAX_CODE_LENGTHS, &codeLengthTable))
    {
        fileLoggerAddErrorEntry("Failed to build Huffman tree for code lengths");
        return FALSE;
    }

    // fileLoggerAddDebugEntry("Built Huffman tree for code lengths");

    /* Use the code length table to decode the literal/length and distance code lengths */
    /* First decode literal/length code lengths */
    i = 0;
    while (i < hlit)
    {
        if (!decodeHuffmanValue(bitBuf, &codeLengthTable, &decodedValue))
        {
            fileLoggerAddErrorEntry("Error decoding literal/length code length");
            freeHuffmanTable(&codeLengthTable);
            return FALSE;
        }

        if (decodedValue < 16)
        {
            /* Direct code length 0-15 */
            literalLengths[i++] = decodedValue;
        }
        else if (decodedValue == 16)
        {
            /* Repeat previous code length 3-6 times */
            UBYTE prevLen = (i > 0) ? literalLengths[i - 1] : 0;
            UBYTE repeatCount;

            if (!readBits(bitBuf, 2, &repeatCount))
            {
                freeHuffmanTable(&codeLengthTable);
                return FALSE;
            }
            repeatCount += 3;

            for (j = 0; j < repeatCount && i < hlit; j++)
                literalLengths[i++] = prevLen;
        }
        else if (decodedValue == 17)
        {
            /* Repeat code length 0 for 3-10 times */
            UBYTE repeatCount;

            if (!readBits(bitBuf, 3, &repeatCount))
            {
                freeHuffmanTable(&codeLengthTable);
                return FALSE;
            }
            repeatCount += 3;

            for (j = 0; j < repeatCount && i < hlit; j++)
                literalLengths[i++] = 0;
        }
        else if (decodedValue == 18)
        {
            /* Repeat code length 0 for 11-138 times */
            UBYTE repeatCount;

            if (!readBits(bitBuf, 7, &repeatCount))
            {
                freeHuffmanTable(&codeLengthTable);
                return FALSE;
            }
            repeatCount += 11;

            for (j = 0; j < repeatCount && i < hlit; j++)
                literalLengths[i++] = 0;
        }
    }

    /* Now decode distance code lengths */
    i = 0;
    while (i < hdist)
    {
        if (!decodeHuffmanValue(bitBuf, &codeLengthTable, &decodedValue))
        {
            fileLoggerAddErrorEntry("Error decoding distance code length");
            freeHuffmanTable(&codeLengthTable);
            return FALSE;
        }

        if (decodedValue < 16)
        {
            /* Direct code length 0-15 */
            distanceLengths[i++] = decodedValue;
        }
        else if (decodedValue == 16)
        {
            /* Repeat previous code length 3-6 times */
            UBYTE prevLen = (i > 0) ? distanceLengths[i - 1] : 0;
            UBYTE repeatCount;

            if (!readBits(bitBuf, 2, &repeatCount))
            {
                freeHuffmanTable(&codeLengthTable);
                return FALSE;
            }
            repeatCount += 3;

            for (j = 0; j < repeatCount && i < hdist; j++)
                distanceLengths[i++] = prevLen;
        }
        else if (decodedValue == 17)
        {
            /* Repeat code length 0 for 3-10 times */
            UBYTE repeatCount;

            if (!readBits(bitBuf, 3, &repeatCount))
            {
                freeHuffmanTable(&codeLengthTable);
                return FALSE;
            }
            repeatCount += 3;

            for (j = 0; j < repeatCount && i < hdist; j++)
                distanceLengths[i++] = 0;
        }
        else if (decodedValue == 18)
        {
            /* Repeat code length 0 for 11-138 times */
            UBYTE repeatCount;

            if (!readBits(bitBuf, 7, &repeatCount))
            {
                freeHuffmanTable(&codeLengthTable);
                return FALSE;
            }
            repeatCount += 11;

            for (j = 0; j < repeatCount && i < hdist; j++)
                distanceLengths[i++] = 0;
        }
    }

    /* Build Huffman trees for literals/lengths and distances */
    HuffmanTable literalTable, distanceTable;

    if (!buildHuffmanTreeFromCodeLengths(literalLengths, hlit, &literalTable))
    {
        fileLoggerAddErrorEntry("Failed to build Huffman tree for literals/lengths");
        freeHuffmanTable(&codeLengthTable);
        return FALSE;
    }

    if (!buildHuffmanTreeFromCodeLengths(distanceLengths, hdist, &distanceTable))
    {
        fileLoggerAddErrorEntry("Failed to build Huffman tree for distances");
        freeHuffmanTable(&codeLengthTable);
        freeHuffmanTable(&literalTable);
        return FALSE;
    }

    // fileLoggerAddDebugEntry("Built Huffman trees for literals/lengths and distances");

    /* Use these trees to decode the actual compressed data (literals and length/distance pairs) */
    if (!decodeLZ77Data(bitBuf, &literalTable, &distanceTable, outputBuffer, outputBufferSize, outPos))
    {
        fileLoggerAddErrorEntry("Failed to decode LZ77 compressed data");
        freeHuffmanTable(&codeLengthTable);
        freeHuffmanTable(&literalTable);
        freeHuffmanTable(&distanceTable);
        return FALSE;
    }

    /* Clean up */
    freeHuffmanTable(&codeLengthTable);
    freeHuffmanTable(&literalTable);
    freeHuffmanTable(&distanceTable);

    return TRUE;
}

/* Build a Huffman tree from code lengths according to RFC 1951
 * This algorithm constructs canonical Huffman codes from code lengths
 */
BOOL buildHuffmanTreeFromCodeLengths(UBYTE *codeLengths, ULONG numCodes, HuffmanTable *table)
{
    ULONG i;
    ULONG code = 0;
    UBYTE maxBits = 0;
    ULONG *blCount = NULL;
    ULONG *nextCode = NULL;
    char logMessage[256];

    if (!codeLengths || !table)
    {
        fileLoggerAddDebugEntry("Invalid parameters for buildHuffmanTreeFromCodeLengths");
        return FALSE;
    }

    /* Allocate the nodes array */
    table->maxCodes = numCodes;
    table->nodes = (HuffmanNode *)malloc(numCodes * sizeof(HuffmanNode));
    if (!table->nodes)
    {
        fileLoggerAddDebugEntry("Failed to allocate memory for Huffman nodes");
        return FALSE;
    }

    /* Initialize all nodes */
    for (i = 0; i < numCodes; i++)
    {
        table->nodes[i].value = i;
        table->nodes[i].bits = codeLengths[i];
        table->nodes[i].code = 0;

        /* Find the maximum bit length */
        if (codeLengths[i] > maxBits)
            maxBits = codeLengths[i];
    }

    table->maxBits = maxBits;

    /* Allocate temporary arrays for counting and generating codes */
    blCount = (ULONG *)malloc((maxBits + 1) * sizeof(ULONG));
    nextCode = (ULONG *)malloc((maxBits + 1) * sizeof(ULONG));

    if (!blCount || !nextCode)
    {
        fileLoggerAddDebugEntry("Failed to allocate memory for Huffman code generation");
        free(blCount);
        free(nextCode);
        free(table->nodes);
        table->nodes = NULL;
        return FALSE;
    }

    /* Initialize arrays */
    for (i = 0; i <= maxBits; i++)
    {
        blCount[i] = 0;
        nextCode[i] = 0;
    }

    /* Count the number of codes for each bit length */
    for (i = 0; i < numCodes; i++)
    {
        if (codeLengths[i] > 0)
            blCount[codeLengths[i]]++;
    }

    /* Find the numerical value of the smallest code for each bit length */
    code = 0;
    for (i = 1; i <= maxBits; i++)
    {
        code = (code + blCount[i - 1]) << 1;
        nextCode[i] = code;
    }

    /* Assign codes to each symbol according to bit lengths */
    for (i = 0; i < numCodes; i++)
    {
        UBYTE len = codeLengths[i];
        if (len > 0)
        {
            table->nodes[i].code = nextCode[len];
            nextCode[len]++;
        }
    }

    /* Clean up temporary arrays */
    free(blCount);
    free(nextCode);

    // sprintf(logMessage, "Built Huffman tree with %lu codes, max bits: %u", numCodes, maxBits);
    // fileLoggerAddDebugEntry(logMessage);

    return TRUE;
}

/* Decode a single value using a Huffman table */
BOOL decodeHuffmanValue(BitBuffer *bitBuf, HuffmanTable *table, UWORD *value)
{
    ULONG code = 0;
    UBYTE len = 0;
    ULONG i;
    UBYTE bit;
    char logMessage[256];

    /* Read bits one at a time and try to find a matching code */
    while (len <= table->maxBits)
    {
        /* Get next bit */
        if (!readBits(bitBuf, 1, &bit))
        {
            fileLoggerAddErrorEntry("Failed to read bit from buffer");
            return FALSE;
        }

        /* Append bit to the code, MSB first */
        code = (code << 1) | bit;
        len++;

        /* Search for matching code in the table */
        for (i = 0; i < table->maxCodes; i++)
        {
            if (table->nodes[i].bits == len && table->nodes[i].code == code)
            {
                /* Found a match */
                *value = table->nodes[i].value;

                return TRUE;
            }
        }
    }

    /* Exceeded max bits without finding a valid code */
    sprintf(logMessage, "Failed to decode Huffman value after %u bits", len);
    fileLoggerAddDebugEntry(logMessage);

    return FALSE;
}

/* Free resources allocated for a Huffman table */
void freeHuffmanTable(HuffmanTable *table)
{
    if (table && table->nodes)
    {
        free(table->nodes);
        table->nodes = NULL;
        table->maxBits = 0;
        table->maxCodes = 0;
    }
}

/* Decode LZ77 compressed data using Huffman tables
 * This function decodes the actual compressed data using the provided Huffman tables
 * It implements LZ77 decompression with variable-length backreferences
 * according to RFC 1951 (DEFLATE) specification
 */
BOOL decodeLZ77Data(BitBuffer *bitBuf, HuffmanTable *literalTable, HuffmanTable *distanceTable,
                    UBYTE *outputBuffer, ULONG outputBufferSize, ULONG *outPos)
{
    UWORD code;
    ULONG length, distance;
    ULONG i, j;
    UBYTE extraBits;
    char logMessage[256];

    /* Keep decoding until we find the end-of-block marker (256) */
    while (1)
    {
        /* Decode a literal/length value using the literal/length Huffman table */
        if (!decodeHuffmanValue(bitBuf, literalTable, &code))
        {
            fileLoggerAddDebugEntry("Failed to decode literal/length value");
            return FALSE;
        }

        if (code < 256)
        {
            /* Literal byte */
            if (*outPos >= outputBufferSize)
            {
                fileLoggerAddDebugEntry("Output buffer overflow when writing literal");
                return FALSE;
            }

            /* Write the literal byte to the output */
            outputBuffer[(*outPos)++] = (UBYTE)code;
        }
        else if (code == 256)
        {
            /* End of block */
            fileLoggerAddDebugEntry("End of block marker found");
            return TRUE;
        }
        else if (code <= 285)
        {
            /* Length/distance pair (backreference) */
            ULONG lengthCode = code - 257;

            /* Get base length for this code */
            if (lengthCode >= 29)
            {
                sprintf(logMessage, "Invalid length code: %lu", lengthCode);
                fileLoggerAddDebugEntry(logMessage);
                return FALSE;
            }

            /* Start with the base length */
            length = lengthBase[lengthCode];

            /* Read extra bits if needed */
            extraBits = lengthExtraBits[lengthCode];
            if (extraBits > 0)
            {
                UBYTE extraBitsValue;

                for (i = 0; i < extraBits; i++)
                {
                    if (!readBits(bitBuf, 1, &extraBitsValue))
                    {
                        fileLoggerAddDebugEntry("Failed to read length extra bits");
                        return FALSE;
                    }
                    length += extraBitsValue << i;
                }
            }

            /* Decode the distance code */
            if (!decodeHuffmanValue(bitBuf, distanceTable, &code))
            {
                fileLoggerAddErrorEntry("Failed to decode distance value");
                return FALSE;
            }

            /* Distance code must be in range 0-29 */
            if (code >= 30)
            {
                sprintf(logMessage, "Invalid distance code: %u", code);
                fileLoggerAddErrorEntry(logMessage);
                return FALSE;
            }

            /* Start with the base distance */
            distance = distanceBase[code];

            /* Read extra bits if needed */
            extraBits = distanceExtraBits[code];
            if (extraBits > 0)
            {
                UBYTE extraBitsValue;

                for (i = 0; i < extraBits; i++)
                {
                    if (!readBits(bitBuf, 1, &extraBitsValue))
                    {
                        fileLoggerAddErrorEntry("Failed to read distance extra bits");
                        return FALSE;
                    }
                    distance += extraBitsValue << i;
                }
            }

            /* Validate the backreference */
            if (distance > *outPos)
            {
                fileLoggerAddErrorEntry("Invalid backreference: distance larger than output position");
                return FALSE;
            }

            if (*outPos + length > outputBufferSize)
            {
                char sizeMessage[256];
                sprintf(sizeMessage, "Output buffer overflow when copying backreference: outPos=%lu, length=%lu, buffer size=%lu",
                        *outPos, length, outputBufferSize);
                fileLoggerAddErrorEntry(sizeMessage);
                return FALSE;
            }

            /* Copy the referenced bytes */
            for (i = 0; i < length; i++)
            {
                outputBuffer[*outPos] = outputBuffer[*outPos - distance];
                (*outPos)++;
            }
        }
        else
        {
            /* Invalid code (codes 286-287 are reserved) */
            sprintf(logMessage, "Invalid literal/length code: %u", code);
            fileLoggerAddErrorEntry(logMessage);
            return FALSE;
        }
    }

    /* We should never reach here because we return when we find the end-of-block marker */
    return FALSE;
}

/* Process a fixed Huffman (type 1) DEFLATE block
 * According to RFC 1951, the fixed Huffman codes are defined as follows:
 * Literal/length alphabet:
 *   - Literals 0-143: 8 bits, codes 00110000 through 10111111
 *   - Literals 144-255: 9 bits, codes 110010000 through 111111111
 *   - Literals 256-279: 7 bits, codes 0000000 through 0010111
 *   - Literals 280-287: 8 bits, codes 11000000 through 11000111
 * Distance alphabet:
 *   - All 30 codes are 5 bits, codes 00000 through 11101
 */
BOOL processFixedHuffmanBlock(BitBuffer *bitBuf, UBYTE *compressedData, ULONG compressedSize,
                              UBYTE *outputBuffer, ULONG outputBufferSize, ULONG *outPos)
{
    char logMessage[256];
    HuffmanTable literalTable, distanceTable;
    UBYTE literalLengths[MAX_LITERAL_CODES];
    UBYTE distanceLengths[MAX_DISTANCE_CODES];
    ULONG i;

    fileLoggerAddDebugEntry("Processing fixed Huffman block");

    /* Initialize code lengths for fixed Huffman codes */
    /* Literals 0-143: 8 bits */
    for (i = 0; i <= 143; i++)
        literalLengths[i] = 8;

    /* Literals 144-255: 9 bits */
    for (i = 144; i <= 255; i++)
        literalLengths[i] = 9;

    /* Literals 256-279: 7 bits */
    for (i = 256; i <= 279; i++)
        literalLengths[i] = 7;

    /* Literals 280-287: 8 bits */
    for (i = 280; i < MAX_LITERAL_CODES; i++)
        literalLengths[i] = 8;

    /* All 30 distance codes are 5 bits */
    for (i = 0; i < MAX_DISTANCE_CODES; i++)
        distanceLengths[i] = 5;

    /* Build Huffman trees for literals/lengths and distances */
    if (!buildHuffmanTreeFromCodeLengths(literalLengths, MAX_LITERAL_CODES, &literalTable))
    {
        fileLoggerAddDebugEntry("Failed to build fixed Huffman tree for literals/lengths");
        return FALSE;
    }

    if (!buildHuffmanTreeFromCodeLengths(distanceLengths, MAX_DISTANCE_CODES, &distanceTable))
    {
        fileLoggerAddDebugEntry("Failed to build fixed Huffman tree for distances");
        freeHuffmanTable(&literalTable);
        return FALSE;
    }

    fileLoggerAddDebugEntry("Built fixed Huffman trees for literals/lengths and distances");

    /* Use these trees to decode the actual compressed data */
    if (!decodeLZ77Data(bitBuf, &literalTable, &distanceTable, outputBuffer, outputBufferSize, outPos))
    {
        fileLoggerAddDebugEntry("Failed to decode fixed Huffman LZ77 compressed data");
        freeHuffmanTable(&literalTable);
        freeHuffmanTable(&distanceTable);
        return FALSE;
    }

    fileLoggerAddDebugEntry("Successfully decoded fixed Huffman LZ77 compressed data");

    /* Clean up */
    freeHuffmanTable(&literalTable);
    freeHuffmanTable(&distanceTable);

    return TRUE;
}
