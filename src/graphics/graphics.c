/*
 * Implementation of common graphics functions for AmigaOS 3.1
 */

#include <stdlib.h>
#include <string.h>
#include "graphics.h"

/* Initialize a palette structure */
void initImgPalette(ImgPalette *palette)
{
    if (palette)
    {
        palette->numColors = 0;
        palette->colorRegs = NULL;
        palette->colorTable = NULL;
        palette->allocated = FALSE;
        palette->hasTransparency = FALSE;
        palette->transparentColor = 0;

        /* Initialize pen mapping to identity (each pen maps to itself) */
        for (ULONG i = 0; i < 256; i++)
        {
            palette->penMap[i] = i;
        }
    }
}

/* Free resources allocated for a palette */
void freeImgPalette(ImgPalette *palette)
{
    if (palette)
    {
        /* Only free if we allocated it */
        if (palette->allocated && palette->colorRegs)
        {
            free(palette->colorRegs);
            palette->colorRegs = NULL;
        }

        /* Reset fields */
        palette->numColors = 0;
        palette->colorTable = NULL;
        palette->allocated = FALSE;
    }
}
