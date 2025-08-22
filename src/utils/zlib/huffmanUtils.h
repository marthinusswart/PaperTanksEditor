/*
 * Huffman coding utilities for AmigaOS 3.1
 * Used for decompressing DEFLATE compressed data in PNG files
 */

#ifndef HUFFMAN_UTILS_H
#define HUFFMAN_UTILS_H

#include <exec/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "zlibutils.h" /* For BitBuffer definition */
#include "../filelogger.h"

/* Constants for Huffman coding */
#define MAX_BITS 15
#define MAX_LITERAL_CODES 286
#define MAX_DISTANCE_CODES 30
#define MAX_CODE_LENGTHS 19
#define END_OF_BLOCK 256

/* Huffman tree node structure */
typedef struct HuffmanNode
{
    UWORD value; /* Value to output (literal, length, or distance) */
    UBYTE bits;  /* Number of bits in the code */
    ULONG code;  /* The Huffman code */
} HuffmanNode;

/* Huffman code table structure */
typedef struct HuffmanTable
{
    UWORD maxCodes;     /* Maximum number of codes in the table */
    UBYTE maxBits;      /* Maximum bit length for codes */
    HuffmanNode *nodes; /* Array of Huffman nodes */
} HuffmanTable;

/* Process a dynamic Huffman (type 2) DEFLATE block */
BOOL processDynamicHuffmanBlock(BitBuffer *bitBuf, UBYTE *compressedData, ULONG compressedSize,
                                UBYTE *outputBuffer, ULONG outputBufferSize, ULONG *outPos);

/* Process a fixed Huffman (type 1) DEFLATE block */
BOOL processFixedHuffmanBlock(BitBuffer *bitBuf, UBYTE *compressedData, ULONG compressedSize,
                              UBYTE *outputBuffer, ULONG outputBufferSize, ULONG *outPos);

/* Get the code length code order for dynamic Huffman decoding */
const UBYTE *getCodeLengthCodeOrder(void);

/* Build a Huffman tree from code lengths */
BOOL buildHuffmanTreeFromCodeLengths(UBYTE *codeLengths, ULONG numCodes, HuffmanTable *table);

/* Decode a single value using a Huffman table */
BOOL decodeHuffmanValue(BitBuffer *bitBuf, HuffmanTable *table, UWORD *value);

/* Free resources allocated for a Huffman table */
void freeHuffmanTable(HuffmanTable *table);

/* Decode LZ77 compressed data using Huffman tables */
BOOL decodeLZ77Data(BitBuffer *bitBuf, HuffmanTable *literalTable, HuffmanTable *distanceTable,
                    UBYTE *outputBuffer, ULONG outputBufferSize, ULONG *outPos);

/* Free resources allocated for a Huffman table */
void freeHuffmanTable(HuffmanTable *table);

#endif /* HUFFMAN_UTILS_H */
