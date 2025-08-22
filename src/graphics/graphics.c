
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
void initImg8BitPalette(Img8BitPalette **palette)
{
    *palette = (Img8BitPalette *)malloc(sizeof(Img8BitPalette));
    Img8BitPalette *_palette = (Img8BitPalette *)*palette;

    if (_palette)
    {
        _palette->colorsAllocated = 0;
        _palette->hasTransparency = FALSE;
        _palette->transparentR8 = 0;
        _palette->transparentG8 = 0;
        _palette->transparentB8 = 0;

        // Initialize palette to black
        for (int i = 0; i < 256; i++)
        {
            _palette->red8[i] = 0;
            _palette->green8[i] = 0;
            _palette->blue8[i] = 0;
        }
    }
}

/* Free resources allocated for a palette */
void freeImg8BitPalette(Img8BitPalette *palette)
{
    if (palette)
    {
    }
}
