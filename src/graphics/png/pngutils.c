
#include "pngutils.h"

/* Load PNG image with palette information */
BOOL loadPNGToBitmapObject(CONST_STRPTR filename, PNGImage **outImage)
{
    if (!filename || !outImage)
        return FALSE;

    unsigned char *image = NULL;
    unsigned int width = 0, height = 0;
    ULONG error = 0;

    /* Decode PNG to RGBA */
    error = lodepng_decode32_file(&image, &width, &height, filename);
    if (error || !image)
    {
        fileLoggerAddErrorEntry("Failed to load PNG image");
        *outImage = NULL;
        return FALSE;
    }

    /* Allocate output image data */
    *outImage = (PNGImage *)malloc(sizeof(PNGImage));
    PNGImage *_outImage = *outImage;
    if (!_outImage)
    {
        fileLoggerAddErrorEntry("Failed to allocate memory for PNGImage");
        free(image);
        return FALSE;
    }

    _outImage->data = (UBYTE *)image;
    _outImage->width = width;
    _outImage->height = height;

    _outImage->hasTransparency = FALSE;

    for (ULONG i = 0; i < width * height; i++)
    {
        if (image[i * 4 + 3] < 255)
        {
            _outImage->hasTransparency = TRUE;
            fileLoggerAddDebugEntry("PNG image has transparency");
            break;
        }
    }

    return TRUE;
}
