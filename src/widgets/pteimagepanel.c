// Include your header
#include "pteimagepanel.h"

/***********************************************************************/

/********************** Prototypes *************************/
extern struct Library *MUIMasterBase;

DISPATCHER(PTEImagePanelDispatcher);
IPTR SAVEDS mNew(struct IClass *cl, Object *obj, struct opSet *msg);
IPTR SAVEDS mDraw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg);
void mDrawBorder(Object *obj, struct PTEImagePanelData *data);
// void mDrawRGB2(Object *obj, struct PTEImagePanelData *data);
// void mDrawRGB3(Object *obj, struct PTEImagePanelData *data);
void mDrawToScreen(Object *obj, struct PTEImagePanelData *data);
LONG xget(Object *obj, ULONG attribute);
Object *getWindowObject(Object *obj);

/***********************************************************************/

struct MUI_CustomClass *pteImagePanelClass;

static ULONG STACKARGS DoSuperNew(struct IClass *const cl, Object *const obj, const ULONG tags, ...)
{
    return (DoSuperMethod(cl, obj, OM_NEW, &tags, NULL));
}

struct MUI_CustomClass *createPTEImagePanelClass(void)
{
    char logMessage[256];
    fileLoggerAddDebugEntry("PTEImagePanel: Creating MUI_CreateCustomClass");
    loggerFormatMessage(logMessage, "Dispatcher address: 0x%08lx", PTEImagePanelDispatcher);
    fileLoggerAddDebugEntry(logMessage);
    struct MUI_CustomClass *obj = MUI_CreateCustomClass(NULL, MUIC_Rectangle, NULL, sizeof(struct PTEImagePanelData), (APTR)PTEImagePanelDispatcher);
    if (!obj)
    {
        fileLoggerAddDebugEntry("PTEImagePanel: Failed to create custom class");
        return NULL;
    }
    else
    {
        fileLoggerAddDebugEntry("PTEImagePanel: Custom class created successfully");
        loggerFormatMessage(logMessage, "Custom class obj address: 0x%08lx", (ULONG)obj);
        fileLoggerAddDebugEntry(logMessage);
        return obj;
    }
}

/***********************************************************************/

/***********************************************************************/

IPTR SAVEDS mNew(struct IClass *cl, Object *obj, struct opSet *msg)
{
    fileLoggerAddDebugEntry("PTEImagePanel: mNew called");

    obj = (Object *)DoSuperNew(cl, obj, TAG_MORE, msg->ops_AttrList);

    if (!obj)
    {
        fileLoggerAddDebugEntry("PTEImagePanel: Failed to call Super");
        return 0;
    }

    // Default values
    BYTE borderColor = 1; // Black
    WORD borderMargin = 0;
    BOOL drawBorder = FALSE;
    UBYTE *imageData = NULL;
    WORD imageHeight = 0;
    WORD imageWidth = 0;
    ImgPalette *imgPalette = NULL;
    BOOL isPNG = FALSE;

    // Parse tag list for custom attributes
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *walk = tags;

    /* Using this because the Parent already walked the list, so the pointer is at the end */
    int i = 0;
    while (walk && walk->ti_Tag != TAG_END)
    {
        char buf[64];
        sprintf(buf, "Tag[%d]: Tag = 0x%lx, Data = 0x%lx", i++, walk->ti_Tag, walk->ti_Data);
        fileLoggerAddDebugEntry(buf);
        walk++;

        switch (walk->ti_Tag)
        {
        case PTEA_BorderColor:
            borderColor = (BYTE)walk->ti_Data;
            break;

        case PTEA_BorderMargin:
            borderMargin = (WORD)walk->ti_Data;
            break;

        case PTEA_DrawBorder:
            drawBorder = (BOOL)walk->ti_Data;
            break;

        case PTEA_ImageData:
            imageData = (UBYTE *)walk->ti_Data;
            break;

        case PTEA_ImageHeight:
            imageHeight = (WORD)walk->ti_Data;
            break;

        case PTEA_ImageWidth:
            imageWidth = (WORD)walk->ti_Data;
            break;

        case PTEA_ImgPalette:
            imgPalette = (ImgPalette *)walk->ti_Data;
            break;

        case PTEA_IsPNG:
            isPNG = (BOOL)walk->ti_Data;
            break;

        default:
            break;
        }
    }

    // Store them in your instance data (assuming you have a struct like this)
    struct PTEImagePanelData *data = INST_DATA(cl, obj);
    data->borderColor = borderColor;
    data->borderMargin = borderMargin;
    data->drawBorder = drawBorder;
    data->imageData = imageData;
    data->imageHeight = imageHeight;
    data->imageWidth = imageWidth;
    data->imgPalette = imgPalette;
    data->isPNG = isPNG;

    return (ULONG)obj;
}

/***********************************************************************/

/***********************************************************************/
// Function to draw border for the PTEImagePanel
void mDrawBorder(Object *obj, struct PTEImagePanelData *data)
{
    struct RastPort *rp;
    WORD left, top, right, bottom;
    char logMessage[256];

    // Get RastPort
    rp = _rp(obj);
    if (!rp)
    {
        fileLoggerAddDebugEntry("PTEImagePanel: rp failed...");
        return;
    }

    // Use DetailPen for visibility (usually black or dark)
    SetAPen(rp, data->borderColor);

    // Calculate inset bounds
    left = _mleft(obj) + data->borderMargin;
    top = _mtop(obj) + data->borderMargin;
    right = _mright(obj) - data->borderMargin;
    bottom = _mbottom(obj) - data->borderMargin;

    // Log coordinates for debugging
    loggerFormatMessage(logMessage, "PTEImagePanel: Drawing rectangle at L=%d T=%d R=%d B=%d", left, top, right, bottom);
    fileLoggerAddDebugEntry(logMessage);

    // Draw rectangle border
    Move(rp, left, top);
    Draw(rp, right, top);
    Draw(rp, right, bottom);
    Draw(rp, left, bottom);
    Draw(rp, left, top); // Close the loop
}

/***********************************************************************/
IPTR SAVEDS mDraw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg)
{
    fileLoggerAddDebugEntry("PTEImagePanel: mDraw called");

    struct RastPort *rp;
    WORD left, top, right, bottom;
    char logMessage[256];

    // Let superclass draw base rectangle
    DoSuperMethodA(cl, obj, (Msg)msg);

    // Get the class data
    struct PTEImagePanelData *data = INST_DATA(cl, obj);

    if (data->drawBorder)
    {
        mDrawBorder(obj, data);
    }

    if (data->imageData != NULL)
    {
        if (data->isPNG)
        {
            // Use specialized PNG drawing function
            mDrawToScreen(obj, data);
        }
    }
    return 0;
}

/***********************************************************************/

/***********************************************************************/

void initializePTEImagePanel(void)
{
    pteImagePanelClass = createPTEImagePanelClass();
}

/***********************************************************************/

// ULONG __saveds __asm PTEImagePanelDispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
DISPATCHER(PTEImagePanelDispatcher)
{
    char logMessage[256];

    if (!msg->MethodID)
    {
        fileLoggerAddDebugEntry("PTEImagePanel: PTEImagePanelDispatcher called with NULL MethodID");
        loggerFormatMessage(logMessage, "PTEImagePanel: PTEImagePanelDispatcher called with MethodID: 0x%08lx", msg->MethodID);
        fileLoggerAddDebugEntry(logMessage);
        loggerFormatMessage(logMessage, "msg pointer Address: 0x%08lx", (ULONG)msg);
        fileLoggerAddDebugEntry(logMessage);
        loggerFormatMessage(logMessage, "PTEImagePanel: Obj Address: 0x%08lx", (ULONG)obj);
        fileLoggerAddDebugEntry(logMessage);
        loggerFormatMessage(logMessage, "PTEImagePanel: Cl Address: 0x%08lx", (ULONG)cl);
        fileLoggerAddDebugEntry(logMessage);
        return 0;
    }

    // case OM_DISPOSE:                    return mDispose(cl, obj, (APTR)msg);
    // case OM_GET:                        return mGet(cl, obj, (APTR)msg);
    // case OM_SET:                        return mSets(cl, obj, (APTR)msg);
    switch (msg->MethodID)
    {
    case OM_NEW:
        return mNew(cl, obj, (APTR)msg);
    case MUIM_Draw:
        return mDraw(cl, obj, (APTR)msg);

    default:
        return DoSuperMethodA(cl, obj, msg);
    }
}

/**********************************************************************/

LONG xget(Object *obj, ULONG attribute)
{
    LONG x;
    get(obj, attribute, &x);
    return (x);
}

/**********************************************************************/

/* Draw To Screen specific function optimized for 24-bit desktop environment */
void mDrawToScreen(Object *obj, struct PTEImagePanelData *data)
{
    struct RastPort *rp;
    WORD left, top, right, bottom;
    char logMessage[256];
    struct ViewPort *vp = NULL;
    struct Screen *scr = NULL;

    fileLoggerAddDebugEntry("PTEImagePanel: Drawing PNG image data in 24-bit mode");
    loggerFormatMessage(logMessage, "PNG image data at: 0x%08lx, dimensions: %dx%d",
                        (ULONG)data->imageData, data->imageWidth, data->imageHeight);
    fileLoggerAddDebugEntry(logMessage);

    // Log transparency information
    if (data->imgPalette)
    {
        loggerFormatMessage(logMessage, "Image palette at: 0x%08lx, has transparency: %s, transparent color: %d",
                            (ULONG)data->imgPalette,
                            data->imgPalette->hasTransparency ? "YES" : "NO",
                            data->imgPalette->transparentColor);
        fileLoggerAddDebugEntry(logMessage);
    }
    else
    {
        fileLoggerAddDebugEntry("No palette information available for PNG");
    }

    // Get RastPort
    rp = _rp(obj);
    if (!rp)
    {
        fileLoggerAddDebugEntry("PTEImagePanel: RastPort unavailable, cannot draw PNG");
        return;
    }

    // Calculate inset bounds
    left = _mleft(obj) + data->borderMargin;
    top = _mtop(obj) + data->borderMargin;
    right = _mright(obj) - data->borderMargin;
    bottom = _mbottom(obj) - data->borderMargin;

    // Convert width/height to right/bottom
    left += 5;
    top += 5;
    right = left + data->imageWidth - 1;
    bottom = top + data->imageHeight - 1;

    // Check bounds against drawable area
    if (right > _mright(obj) - data->borderMargin)
        right = _mright(obj) - data->borderMargin;
    if (bottom > _mbottom(obj) - data->borderMargin)
        bottom = _mbottom(obj) - data->borderMargin;

    // Get the screen from the window - this is required for direct RGB drawing
    Object *win = getWindowObject(obj);
    if (!win)
    {
        fileLoggerAddDebugEntry("PTEImagePanel: Could not get window object, cannot draw");
        return;
    }

    // First try to get screen directly using MUI macros
    scr = _screen(obj);
    if (scr)
    {
        fileLoggerAddDebugEntry("Successfully got screen using _screen() macro");
        vp = &scr->ViewPort;
    }
    else
    {
        // Try getting window structure if macro didn't work
        struct Window *window = NULL;
        get(win, MUIA_Window_Window, &window);

        if (window && window->WScreen)
        {
            scr = window->WScreen;
            vp = &scr->ViewPort;
            fileLoggerAddDebugEntry("Successfully got screen from Window structure");
        }
        else
        {
            // Try getting screen directly
            get(win, MUIA_Window_Screen, &scr);
            if (scr)
            {
                vp = &scr->ViewPort;
                fileLoggerAddDebugEntry("Successfully got screen from MUIA_Window_Screen");
            }
            else
            {
                // In case we can't get the screen, we'll use a fallback approach for 24-bit drawing
                fileLoggerAddDebugEntry("WARNING: Could not get screen structure, using direct 24-bit drawing without screen info");
                // We'll continue without screen/viewport information
            }
        }
    }

    fileLoggerAddDebugEntry("24-bit PNG drawing using SetRGB32 with basic BGRA colors");

    // For best performance, we'll use the ViewPort with SetRGB32
    if (vp)
    {
        fileLoggerAddDebugEntry("Using 24-bit direct RGB rendering with ViewPort");

        // Direct RGB drawing using the actual image data
        for (WORD y = 0; y < data->imageHeight; y++)
        {
            for (WORD x = 0; x < data->imageWidth; x++)
            {
                LONG px = left + x;
                LONG py = top + y;

                // Clip to drawable area
                if (px <= right && py <= bottom)
                {
                    // Calculate offset into RGB chunky data (3 bytes per pixel)
                    ULONG offset = (y * data->imageWidth + x) * 3;
                    ULONG pixelIndex = y * data->imageWidth + x;

                    // Get RGB components (stored as RGB)
                    UBYTE r = data->imageData[offset];     // R
                    UBYTE g = data->imageData[offset + 1]; // G
                    UBYTE b = data->imageData[offset + 2]; // B

                    // Check if we have a transparency mask and if this pixel is transparent
                    BOOL isTransparent = FALSE;

                    if (data->imgPalette && data->imgPalette->hasTransparency)
                    {
                        // For transparent PNGs, we need to check if this is a transparent pixel
                        // In our current implementation, transparent pixels are marked as black (0,0,0)
                        // But we need to be careful not to skip legitimate black pixels
                        // A better approach would be to use a separate transparency mask

                        // This is a simplified approach - only pixels that are both black AND
                        // in images with transparency flags are considered transparent
                        if (r == 0 && g == 0 && b == 0)
                        {
                            // Log this potential transparent pixel for debugging
                            // Disabled in production to avoid log spam
                            // char transMsg[100];
                            // sprintf(transMsg, "Found black pixel at (%d,%d) - treating as transparent", x, y);
                            // fileLoggerAddDebugEntry(transMsg);

                            isTransparent = TRUE;
                        }
                    }

                    // If the pixel is transparent, skip drawing it
                    if (isTransparent)
                    {
                        continue; // Skip to the next pixel
                    }

                    // Convert 8-bit RGB to 32-bit RGB required by SetRGB32
                    ULONG r32 = ((ULONG)r << 24) | ((ULONG)r << 16) | ((ULONG)r << 8) | r;
                    ULONG g32 = ((ULONG)g << 24) | ((ULONG)g << 16) | ((ULONG)g << 8) | g;
                    ULONG b32 = ((ULONG)b << 24) | ((ULONG)b << 16) | ((ULONG)b << 8) | b;

                    // Set the color for a temporary pen in the viewport
                    SetRGB32(vp, 5, r32, g32, b32);

                    // Draw with the temporary pen
                    SetAPen(rp, 5);
                    WritePixel(rp, px, py);
                }
            }
        }

        fileLoggerAddDebugEntry("Completed drawing with SetRGB32 for direct RGB rendering");
    }
    else
    {
        fileLoggerAddDebugEntry("No ViewPort available - fallback to direct drawing");
        // Fallback handled below
    }

    // For 24-bit color direct drawing with ViewPort
    if (vp)
    {
        // Log success about ViewPort usage
        fileLoggerAddDebugEntry("Using ViewPort for 24-bit direct RGB drawing");
    }
    else
    {
        // If we're in this section, we need a fallback drawing method
        fileLoggerAddDebugEntry("No ViewPort available, using fallback drawing method");

        // Direct RGB drawing as a fallback method
        for (WORD y = 0; y < data->imageHeight; y++)
        {
            for (WORD x = 0; x < data->imageWidth; x++)
            {
                LONG px = left + x;
                LONG py = top + y;

                // Clip to drawable area
                if (px <= right && py <= bottom)
                {
                    // Calculate offset into RGB chunky data (3 bytes per pixel)
                    ULONG offset = (y * data->imageWidth + x) * 3;
                    ULONG pixelIndex = y * data->imageWidth + x;

                    // Get RGB components from our 24-bit RGB data
                    UBYTE r = data->imageData[offset];     // R
                    UBYTE g = data->imageData[offset + 1]; // G
                    UBYTE b = data->imageData[offset + 2]; // B

                    // Check if we have a transparency mask and if this pixel is transparent
                    BOOL isTransparent = FALSE;

                    if (data->imgPalette && data->imgPalette->hasTransparency)
                    {
                        // For transparent PNGs, we need to check if this is a transparent pixel
                        // In our current implementation, transparent pixels are marked as black (0,0,0)
                        if (r == 0 && g == 0 && b == 0)
                        {
                            isTransparent = TRUE;
                        }
                    }

                    // If the pixel is transparent, skip drawing it
                    if (isTransparent)
                    {
                        continue; // Skip to the next pixel
                    }

                    // Create a pen value suitable for SetAPen (depends on the platform)
                    // This is a fallback that may not work perfectly on all systems
                    ULONG penValue = ((ULONG)r << 16) | ((ULONG)g << 8) | b;

                    // Use the RGB value directly as a pen number
                    // This may not display correctly on all systems without SetRGB32
                    SetAPen(rp, penValue & 0xFF);
                    WritePixel(rp, px, py);
                }
            }
        }
    }

    fileLoggerAddDebugEntry("PNG drawing completed successfully");
}
