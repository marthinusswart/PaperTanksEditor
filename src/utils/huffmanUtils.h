/*
 * Huffman coding utilities for AmigaOS 3.1
 * Used for decompressing DEFLATE compressed data in PNG files
 */

#ifndef HUFFMAN_UTILS_H
#define HUFFMAN_UTILS_H

#include <exec/types.h>
#include "zlibutils.h" /* For BitBuffer definition */

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

/* Process a dynamic Huffman (type 2) DEFLATE block */
BOOL processDynamicHuffmanBlock(BitBuffer *bitBuf, UBYTE *compressedData, ULONG compressedSize,
                                UBYTE *outputBuffer, ULONG outputBufferSize, ULONG *outPos);

/* Get the code length code order for dynamic Huffman decoding */
const UBYTE *getCodeLengthCodeOrder(void);

#endif /* HUFFMAN_UTILS_H */
