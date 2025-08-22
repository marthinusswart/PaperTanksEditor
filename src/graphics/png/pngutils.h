/* PNG utilities for AmigaOS 3.1 */
#ifndef PNGUTILS_H
#define PNGUTILS_H

#include <exec/types.h>
#include "../../graphics/graphics.h"

/* Load PNG image with palette information */
BOOL loadPNGToBitmapObject2(CONST_STRPTR filename, UBYTE **outImageData, ImgPalette **outPalette);

#endif /* PNGUTILS_H */
