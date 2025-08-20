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

## Transparency Implementation

### Overview

Transparency in PNG files can be implemented in several ways:

1. Alpha channel in RGBA images (color type 6)
2. Alpha channel in grayscale+alpha images (color type 4)
3. tRNS chunk for palette-based images (color type 3)
4. tRNS chunk with a specific transparent color for RGB images (color type 2)

Our implementation supports transparency with the following approach:

### Key Components

1. **Transparency Detection**:

   - Added support for the tRNS chunk (chunk type 0x74524E53)
   - Added transparency fields to the ImgPalette structure:
     - `hasTransparency` flag to indicate if the image has transparent pixels
     - `transparentColor` to store the index of the transparent color in the palette

2. **RGBA Handling**:
   - Processes alpha channel in RGBA images (color type 6)
   - Pixels with alpha < 128 are treated as fully transparent
   - Only sets the hasTransparency flag if transparent pixels are actually found
3. **Rendering Approach**:
   - Transparent pixels are encoded as black (0,0,0) during PNG processing
   - When an image has transparency, black pixels (0,0,0) are skipped during rendering
   - To prevent legitimate black pixels from being treated as transparent, they are adjusted to near-black (1,1,1)

### Implementation Details

1. **PNG Processing Phase**:

   ```c
   // For RGBA images with transparency
   if (a < 128) /* If pixel is mostly transparent */
   {
       // Mark as transparent by setting to black (0,0,0)
       (*outImageData)[i * 3] = 0;     /* R */
       (*outImageData)[i * 3 + 1] = 0; /* G */
       (*outImageData)[i * 3 + 2] = 0; /* B */
   }
   else if (r == 0 && g == 0 && b == 0 && imgPalette && imgPalette->hasTransparency)
   {
       // If this is a legitimate black pixel in an image with transparency,
       // adjust it slightly to distinguish from transparent black
       (*outImageData)[i * 3] = 1;     /* R */
       (*outImageData)[i * 3 + 1] = 1; /* G */
       (*outImageData)[i * 3 + 2] = 1; /* B */
   }
   ```

2. **Rendering Phase**:
   ```c
   // Check if we have transparency information and if this pixel is marked as transparent
   if (data->imgPalette && data->imgPalette->hasTransparency && r == 0 && g == 0 && b == 0)
   {
       // Skip drawing this pixel, leaving the background visible
       continue;
   }
   ```

### Limitations and Future Improvements

1. **Black Pixel Handling**:
   - The current approach has a limitation in that it modifies legitimate black pixels in images with transparency
   - A more robust approach would be to use a separate transparency mask
2. **Partial Transparency**:
   - Current implementation treats transparency as binary (either fully transparent or fully opaque)
   - Alpha blending is not supported, which would require more complex rendering capabilities
3. **tRNS Chunk Support**:
   - While the tRNS chunk is defined in the code, the processing could be enhanced to support different color types
4. **Performance Considerations**:
   - The current implementation prioritizes correctness over performance
   - Specialized routines for different transparency types could improve performance

### Testing and Validation

When testing PNG transparency:

1. Verify that transparent areas in the PNG show the background through them
2. Confirm that legitimate black pixels in the image are still visible
3. Test with different types of PNGs: RGBA, indexed with tRNS, etc.
4. Verify the edges of transparent regions to ensure proper rendering
