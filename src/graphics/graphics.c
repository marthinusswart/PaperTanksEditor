
/*
 * Implementation of common graphics functions for AmigaOS 3.1
 */

#include <stdlib.h>
#include <string.h>
#include "graphics.h"

/*
 * Attempts to find the MUI window object for a given MUI object by walking up the hierarchy.
 */
Object *getWindowObject(Object *obj)
{
    Object *win = NULL;
    Object *parent = obj;
    int depth = 0;

    while (parent)
    {
        depth++;
        LONG isWindow = 0;
        BOOL gotWindowAttr = get(parent, MUIA_WindowObject, &isWindow);
        if (isWindow)
        {
            win = parent;
            break;
        }
        struct Window *window = NULL;
        BOOL gotWin = get(parent, MUIA_Window_Window, &window);
        if (gotWin && window)
        {
            win = parent;
            break;
        }
        struct Screen *scr = NULL;
        BOOL gotScr = get(parent, MUIA_Window_Screen, &scr);
        if (gotScr && scr)
        {
            win = parent;
            break;
        }
        Object *oldParent = parent;
        get(parent, MUIA_Parent, &parent);
        if (parent == oldParent || depth > 10)
        {
            break;
        }
    }
    return win;
}

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
