# PNG Implementation Status and Next Steps

## Current Implementation State

The `imgpngutils.c` file currently implements a basic PNG loader for AmigaOS 3.1 with the following capabilities:

### Completed Components

1. **PNG Header Processing**

   - Correctly identifies and validates PNG signatures
   - Parses the IHDR chunk to extract width, height, bit depth, and color type information
   - Supports basic validation of PNG header fields

2. **Chunk Parsing**

   - Implements a standard chunk parsing mechanism according to the PNG specification
   - Recognizes and processes key chunk types: IHDR, PLTE, IDAT, and IEND
   - Correctly handles chunk CRC validation

3. **ZLIB/DEFLATE Decompression**

   - Successfully processes ZLIB headers according to RFC 1950
   - Implements all three DEFLATE block types (uncompressed, fixed Huffman, dynamic Huffman)
   - Correctly handles Huffman tree construction and LZ77 decompression
   - Verifies Adler-32 checksums to ensure data integrity

4. **Basic Color Handling**

   - Contains foundation for handling different color types (RGB, RGBA, Grayscale, etc.)
   - Includes palette (PLTE chunk) processing for indexed color images
   - Accounts for the BGRA color format used by UAE/Picasso96 environments

5. **Fallback Mode**
   - Implements a test pattern generation function as a fallback
   - Currently uses the test pattern when filter processing fails

### Current Limitations

1. **PNG Filter Processing**: The key missing component is PNG filter processing. After ZLIB decompression, the PNG data still needs to be processed to reverse the filtering applied before compression.

2. **Interlacing**: No support for Adam7 interlacing, which breaks down images into 7 passes for progressive rendering.

3. **Advanced Chunks**: Does not process advanced chunks like tRNS (transparency), gAMA (gamma), etc.

4. **Memory Management**: Uses fixed buffer allocation rather than dynamic resizing, potentially limiting the size of images that can be loaded.

## Next Steps

### 1. Implement PNG Filter Processing

The highest priority is to implement the five standard PNG filter types:

- **Type 0: None** - No filtering
- **Type 1: Sub** - Subtracts the value of the pixel to the left
- **Type 2: Up** - Subtracts the value of the pixel above
- **Type 3: Average** - Uses the average of the left and above pixels
- **Type 4: Paeth** - Uses a specific predictor algorithm

Each scanline in the PNG is preceded by a filter type byte, and the appropriate filter must be applied to recover the original pixel data.

#### Implementation Approach

1. Create a new function `applyPNGFilters` that:

   - Takes the decompressed data from ZLIB decompression
   - Processes it scanline by scanline, applying the appropriate filter
   - Outputs the reconstructed pixel data

2. Add helper functions for each filter type:

   - `applyNoneFilter` (trivial - just copies data)
   - `applySubFilter`
   - `applyUpFilter`
   - `applyAverageFilter`
   - `applyPaethFilter`

3. Create a Paeth predictor function as defined in the PNG specification

### 2. Color Conversion and Handling

After filter processing, implement proper color handling:

1. **Color Type Processing**:

   - Process grayscale images (color types 0 and 4)
   - Process true color images (color types 2 and 6)
   - Process indexed color images (color type 3)

2. **Bit Depth Handling**:

   - Support various bit depths (1, 2, 4, 8, 16)
   - Implement proper scaling for conversion to 8-bit RGB

3. **Alpha Channel Handling**:
   - Properly handle transparency in RGBA images
   - Process the tRNS chunk for images with transparency

### 3. Memory Management Improvements

1. **Dynamic Buffer Allocation**:

   - Implement more robust buffer allocation that can resize as needed
   - Add safeguards against extremely large images

2. **Stream Processing**:
   - Consider implementing a streaming approach for large images

### 4. Advanced Features

1. **Interlacing Support**:

   - Implement Adam7 interlacing for progressive rendering

2. **Additional Chunk Support**:

   - Add support for more optional chunks like gAMA, cHRM, etc.

3. **Error Handling**:
   - Improve error reporting and recovery mechanisms

## Resources

- [PNG Specification](http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html)
- [RFC 1950 - ZLIB Compressed Data Format](https://tools.ietf.org/html/rfc1950)
- [RFC 1951 - DEFLATE Compressed Data Format](https://tools.ietf.org/html/rfc1951)

## Implementation Notes

The PNG filter algorithms are relatively simple but require careful implementation, especially for edge cases like the first pixel in a row or the first row in an image. The Paeth predictor is the most complex but also often the most effective filter.

For AmigaOS 3.1 implementation, special attention must be paid to:

1. Memory constraints
2. Performance optimization for 68k processors
3. Compatibility with UAE/Picasso96 color formats (BGRA)
