/**
 * @file pteimagepanel.c
 * @brief Implements the PTEImagePanel custom MUI class for displaying images (PNG, 24-bit) with optional border and transparency.
 *
 * This file defines the PTEImagePanel class, which provides a custom MUI panel for rendering image data,
 * including PNG support, border drawing, and transparency handling. It includes functions for class creation,
 * initialization, drawing, and pixel rendering optimized for AmigaOS environments.
 *
 * Functions:
 * - struct MUI_CustomClass *createPTEImagePanelClass(void)
 *      Creates and initializes the custom MUI class for the image panel.
 *
 * - void initializePTEImagePanel(void)
 *      Initializes the global pointer to the custom class.
 *
 * - static IPTR SAVEDS mNew(struct IClass *cl, Object *obj, struct opSet *msg)
 *      Handles object instantiation and attribute parsing for the image panel.
 *
 * - static IPTR SAVEDS mDraw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg)
 *      Handles drawing the panel, including border and image rendering.
 *
 * - static void mDrawBorder(Object *obj, PTEImagePanelData *data)
 *      Draws a border around the image panel using the specified color and margin.
 *
 * - static void mDrawToScreen(Object *obj, PTEImagePanelData *data)
 *      Renders PNG image data to the screen, handling 24-bit color and transparency.
 *
 * - static BOOL mWritePixels(PTEImagePanelData *data, struct RastPort *rp, struct ViewPort *vp, WORD left, WORD top, WORD right, WORD bottom)
 *      Writes pixel data directly to the screen, supporting 24-bit RGB and alpha transparency.
 *
 * - static LONG xget(Object *obj, ULONG attribute)
 *      Helper function to retrieve an attribute value from an object.
 *
 * - DISPATCHER(PTEImagePanelDispatcher)
 *      Main dispatcher for handling MUI method calls (OM_NEW, MUIM_Draw, etc.).
 *
 * Global Variables:
 * - struct MUI_CustomClass *pteImagePanelClass
 *      Pointer to the custom class instance.
 *
 * Logging and error handling are integrated throughout for debugging and diagnostics.
 */

#include "pteimagepanel.h"

/***********************************************************************/

/********************** Prototypes *************************/
extern struct Library *MUIMasterBase;

DISPATCHER(PTEImagePanelDispatcher);
static IPTR SAVEDS mNew(struct IClass *cl, Object *obj, struct opSet *msg);
static IPTR SAVEDS mDraw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg);
static void mDrawBorder(Object *obj, PTEImagePanelData *data);
static void mDrawToScreen(Object *obj, PTEImagePanelData *data);
static LONG xget(Object *obj, ULONG attribute);
static BOOL mWritePixels(PTEImagePanelData *data, struct RastPort *rp, struct ViewPort *vp, WORD left, WORD top, WORD right, WORD bottom);

/***********************************************************************/

struct MUI_CustomClass *pteImagePanelClass;

static ULONG STACKARGS DoSuperNew(struct IClass *const cl, Object *const obj, const ULONG tags, ...)
{
    return (DoSuperMethod(cl, obj, OM_NEW, &tags, NULL));
}

struct MUI_CustomClass *createPTEImagePanelClass(void)
{
    char logMessage[256];
    loggerFormatMessage(logMessage, "Dispatcher address: 0x%08lx", PTEImagePanelDispatcher);
    fileLoggerAddDebugEntry(logMessage);
    struct MUI_CustomClass *obj = MUI_CreateCustomClass(NULL, MUIC_Rectangle, NULL, sizeof(PTEImagePanelData), (APTR)PTEImagePanelDispatcher);
    if (!obj)
    {
        fileLoggerAddErrorEntry("PTEImagePanel: Failed to create custom class");
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

static IPTR SAVEDS mNew(struct IClass *cl, Object *obj, struct opSet *msg)
{

    obj = (Object *)DoSuperNew(cl, obj, TAG_MORE, msg->ops_AttrList);

    if (!obj)
    {
        fileLoggerAddErrorEntry("PTEImagePanel: Failed to call Super");
        return 0;
    }

    // Default values
    BYTE borderColor = 1; // Black
    WORD borderMargin = 0;
    BOOL drawBorder = FALSE;
    UBYTE *imageData = NULL;
    WORD imageHeight = 0;
    WORD imageWidth = 0;
    BOOL isPNG = FALSE;
    BOOL hasTransparency = FALSE;

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

        case PTEA_HasTransparency:
            hasTransparency = (BOOL)walk->ti_Data;
            break;

        case PTEA_IsPNG:
            isPNG = (BOOL)walk->ti_Data;
            break;

        default:
            break;
        }
    }

    // Store them in your instance data (assuming you have a struct like this)
    PTEImagePanelData *data = INST_DATA(cl, obj);
    data->borderColor = borderColor;
    data->borderMargin = borderMargin;
    data->drawBorder = drawBorder;
    data->imageData = imageData;
    data->imageHeight = imageHeight;
    data->imageWidth = imageWidth;
    data->hasTransparency = hasTransparency;
    data->isPNG = isPNG;

    return (ULONG)obj;
}

/***********************************************************************/

/***********************************************************************/
// Function to draw border for the PTEImagePanel
static void mDrawBorder(Object *obj, PTEImagePanelData *data)
{
    struct RastPort *rp;
    WORD left, top, right, bottom;
    char logMessage[256];

    // Get RastPort
    rp = _rp(obj);
    if (!rp)
    {
        fileLoggerAddErrorEntry("PTEImagePanel: rp failed...");
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
static IPTR SAVEDS mDraw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg)
{

    struct RastPort *rp;
    WORD left, top, right, bottom;
    char logMessage[256];

    // Let superclass draw base rectangle
    DoSuperMethodA(cl, obj, (Msg)msg);

    // Get the class data
    PTEImagePanelData *data = INST_DATA(cl, obj);

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
        fileLoggerAddErrorEntry("PTEImagePanel: PTEImagePanelDispatcher called with NULL MethodID");
        loggerFormatMessage(logMessage, "PTEImagePanel: PTEImagePanelDispatcher called with MethodID: 0x%08lx", msg->MethodID);
        fileLoggerAddErrorEntry(logMessage);
        loggerFormatMessage(logMessage, "msg pointer Address: 0x%08lx", (ULONG)msg);
        fileLoggerAddErrorEntry(logMessage);
        loggerFormatMessage(logMessage, "PTEImagePanel: Obj Address: 0x%08lx", (ULONG)obj);
        fileLoggerAddErrorEntry(logMessage);
        loggerFormatMessage(logMessage, "PTEImagePanel: Cl Address: 0x%08lx", (ULONG)cl);
        fileLoggerAddErrorEntry(logMessage);
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

static LONG xget(Object *obj, ULONG attribute)
{
    LONG x;
    get(obj, attribute, &x);
    return (x);
}

/**********************************************************************/

/* Draw To Screen specific function optimized for 24-bit desktop environment */
static void mDrawToScreen(Object *obj, PTEImagePanelData *data)
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

    // Get RastPort
    rp = _rp(obj);
    if (!rp)
    {
        fileLoggerAddErrorEntry("PTEImagePanel: RastPort unavailable, cannot draw PNG");
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
        fileLoggerAddErrorEntry("PTEImagePanel: Could not get window object, cannot draw");
        return;
    }

    if (!getScreenViewport(obj, win, &vp))
    {
        fileLoggerAddErrorEntry("PTEColorPalettePanel: Could not get screen viewport");
        return;
    }

    // For best performance, we'll use the ViewPort with SetRGB32
    if (vp)
    {
        fileLoggerAddDebugEntry("Using 24-bit direct RGB32 rendering with ViewPort");
        mWritePixels(data, rp, vp, left, top, right, bottom);
        fileLoggerAddDebugEntry("Completed drawing with SetRGB32 for direct RGB rendering");
    }
    else
    {
        fileLoggerAddErrorEntry("No ViewPort available - fallback to direct drawing");
        // Fallback handled below
    }
}

static BOOL mWritePixels(PTEImagePanelData *data, struct RastPort *rp, struct ViewPort *vp, WORD left, WORD top, WORD right, WORD bottom)
{
    char loggerMessage[256];

    // Direct GBR drawing using the actual image data
    for (WORD y = 0; y < data->imageHeight; y++)
    {
        for (WORD x = 0; x < data->imageWidth; x++)
        {
            LONG px = left + x;
            LONG py = top + y;

            // Clip to drawable area
            if (px <= right && py <= bottom)
            {
                // Calculate offset into RGB chunky data (4 bytes per pixel)
                ULONG offset = (y * data->imageWidth + x) * 4;
                ULONG pixelIndex = y * data->imageWidth + x;

                // Get RGB components
                UBYTE r8 = data->imageData[offset];     // R
                UBYTE g8 = data->imageData[offset + 1]; // G
                UBYTE b8 = data->imageData[offset + 2]; // B
                UBYTE a8 = data->imageData[offset + 3]; // A

                // Check if we have a transparency mask and if this pixel is transparent
                BOOL isTransparent = FALSE;

                if (data->hasTransparency)
                {
                    // Alpha is 10 or less.
                    if (a8 <= 10)
                    {
                        continue;
                    }
                }

                // Convert 8-bit RGB to 32-bit RGB required by SetRGB32
                ULONG r32 = ((ULONG)r8 << 24) | ((ULONG)r8 << 16) | ((ULONG)r8 << 8) | r8;
                ULONG g32 = ((ULONG)g8 << 24) | ((ULONG)g8 << 16) | ((ULONG)g8 << 8) | g8;
                ULONG b32 = ((ULONG)b8 << 24) | ((ULONG)b8 << 16) | ((ULONG)b8 << 8) | b8;

                // Set the color for a temporary pen in the viewport
                SetRGB32(vp, 255, r32, g32, b32);

                // Draw with the temporary pen
                SetAPen(rp, 255);
                WritePixel(rp, px, py);
            }
        }
    }
    return TRUE;
}
