
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
void initImg8BitPalette(Img8BitPalette *palette)
{
    if (palette)
    {
        palette->colorsAllocated = 0;
        palette->hasTransparency = FALSE;
        palette->transparentR8 = 0;
        palette->transparentG8 = 0;
        palette->transparentB8 = 0;
    }
}

/* Free resources allocated for a palette */
void freeImg8BitPalette(Img8BitPalette *palette)
{
    if (palette)
    {
    }
}
