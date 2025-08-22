/* PNG utilities for AmigaOS 3.1 */
#ifndef PNGUTILS_H
#define PNGUTILS_H

#include <exec/types.h>
#include <stdlib.h>
#include <string.h>
#include "../../graphics/graphics.h"
#include "../../../external/lodepng/lodepng.h"
#include "../../utils/filelogger.h"

/* Load PNG image with palette information */
BOOL loadPNGToBitmapObject(CONST_STRPTR filename, UBYTE **outImageData, ImgPalette **outPalette);

#endif /* PNGUTILS_H */
