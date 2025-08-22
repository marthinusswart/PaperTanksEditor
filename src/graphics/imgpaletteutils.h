/*
 * Image Palette Utilities for AmigaOS 3.1
 * Handles color palette operations and pen mapping
 */

#ifndef IMGPALETTEUTILS_H
#define IMGPALETTEUTILS_H

#include <exec/types.h>
#include <intuition/intuition.h>
#include <datatypes/pictureclass.h>
#include <exec/memory.h>
#include <graphics/view.h>
#include <proto/graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/filelogger.h"
#include "graphics.h"

BOOL loadPaletteFromPNGImage32(PNGImage *image, Img8BitPalette **img8BitPalette);

#endif /* IMGPALETTEUTILS_H */
