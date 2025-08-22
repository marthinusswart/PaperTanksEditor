/**
 * @file ptecolorpalettepanel.c
 * @brief Implements the PTEColorPalettePanel custom MUI class for displaying color palettes with optional border.
 *
 * This file defines the PTEColorPalettePanel class, a custom MUI panel for rendering color palette data,
 * including border drawing and integration with AmigaOS graphics. It provides functions for class creation,
 * initialization, drawing, and palette rendering.
 *
 * Functions:
 * - struct MUI_CustomClass *createPTEColorPalettePanelClass(void)
 *      Creates and initializes the custom MUI class for the color palette panel.
 *
 * - void initializePTEColorPalettePanel(void)
 *      Initializes the global pointer to the custom class.
 *
 * - static IPTR SAVEDS mNew(struct IClass *cl, Object *obj, struct opSet *msg)
 *      Handles object instantiation and attribute parsing for the color palette panel.
 *
 * - static IPTR SAVEDS mDraw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg)
 *      Handles drawing the panel, including border and palette rendering.
 *
 * - static void mDrawBorder(Object *obj, PTEColorPalettePanelData *data)
 *      Draws a border around the color palette panel using the specified color and margin.
 *
 * - static void mDrawToScreen(Object *obj, PTEColorPalettePanelData *data)
 *      Renders the color palette data to the screen, handling 24-bit color.
 *
 * - static LONG xget(Object *obj, ULONG attribute)
 *      Helper function to retrieve an attribute value from an object.
 *
 * - DISPATCHER(PTEColorPalettePanelDispatcher)
 *      Main dispatcher for handling MUI method calls (OM_NEW, MUIM_Draw, etc.).
 *
 * Global Variables:
 * - struct MUI_CustomClass *pteColorPalettePanelClass
 *      Pointer to the custom class instance.
 *
 * Logging and error handling are integrated throughout for debugging and diagnostics.
 */

#include "ptecolorpalettepanel.h"

#define PALETTE_SIZE 256
#define GRID_SIZE 16

/********************** Prototypes *************************/
extern struct Library *MUIMasterBase;

DISPATCHER(PTEColorPalettePanelDispatcher);
static IPTR SAVEDS mNew(struct IClass *cl, Object *obj, struct opSet *msg);
static IPTR SAVEDS mDraw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg);
static void mDrawBorder(Object *obj, PTEColorPalettePanelData *data);
static LONG xget(Object *obj, ULONG attribute);
static void mDrawToScreen(Object *obj, PTEColorPalettePanelData *data);
// BOOL getScreenViewport(Object *obj, Object *win, struct ViewPort **outVp);

/***********************************************************************/

struct MUI_CustomClass *pteColorPalettePanelClass;

typedef struct
{
    int x;
    int y;
    int width;
    int height;
} Square;

static ULONG STACKARGS DoSuperNew(struct IClass *const cl, Object *const obj, const ULONG tags, ...)
{
    return (DoSuperMethod(cl, obj, OM_NEW, &tags, NULL));
}

struct MUI_CustomClass *createPTEColorPalettePanelClass(void)
{
    char logMessage[256];
    loggerFormatMessage(logMessage, "Dispatcher address: 0x%08lx", PTEColorPalettePanelDispatcher);
    fileLoggerAddDebugEntry(logMessage);
    struct MUI_CustomClass *obj = MUI_CreateCustomClass(NULL, MUIC_Rectangle, NULL, sizeof(PTEColorPalettePanelData), (APTR)PTEColorPalettePanelDispatcher);
    if (!obj)
    {
        fileLoggerAddErrorEntry("PTEColorPalettePanel: Failed to create custom class");
        return NULL;
    }
    else
    {
        fileLoggerAddDebugEntry("PTEColorPalettePanel: Custom class created successfully");
        loggerFormatMessage(logMessage, "Custom class obj address: 0x%08lx", (ULONG)obj);
        fileLoggerAddDebugEntry(logMessage);
        return obj;
    }
}

/***********************************************************************/

static IPTR SAVEDS mNew(struct IClass *cl, Object *obj, struct opSet *msg)
{

    obj = (Object *)DoSuperNew(cl, obj, TAG_MORE, msg->ops_AttrList);

    if (!obj)
    {
        fileLoggerAddErrorEntry("PTEColorPalettePanel: Failed to call Super");
        return 0;
    }

    // Default values
    BYTE borderColor = 1; // Black
    WORD borderMargin = 0;
    BOOL drawBorder = FALSE;
    Img8BitPalette *colorPalette = NULL;

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

        case PTEA_ColorPalette:
            colorPalette = (Img8BitPalette *)walk->ti_Data;
            break;

        default:
            break;
        }
    }

    // Store them in your instance data (assuming you have a struct like this)
    PTEColorPalettePanelData *data = INST_DATA(cl, obj);
    data->borderColor = borderColor;
    data->borderMargin = borderMargin;
    data->drawBorder = drawBorder;
    data->colorPalette = colorPalette;

    return (ULONG)obj;
}

/***********************************************************************/

/***********************************************************************/
// Function to draw border for the PTEColorPalettePanel
static void mDrawBorder(Object *obj, PTEColorPalettePanelData *data)
{
    struct RastPort *rp;
    WORD left, top, right, bottom;
    char logMessage[256];

    // Get RastPort
    rp = _rp(obj);
    if (!rp)
    {
        fileLoggerAddErrorEntry("PTEColorPalettePanel: rp failed...");
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
    loggerFormatMessage(logMessage, "PTEColorPalettePanel: Drawing rectangle at L=%d T=%d R=%d B=%d", left, top, right, bottom);
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
    PTEColorPalettePanelData *data = INST_DATA(cl, obj);

    if (data->drawBorder)
    {
        mDrawBorder(obj, data);
    }

    if (data->colorPalette)
    {
        // Draw the color palette
        mDrawToScreen(obj, data);
    }

    return 0;
}

/***********************************************************************/

/***********************************************************************/

void initializePTEColorPalettePanel(void)
{
    pteColorPalettePanelClass = createPTEColorPalettePanelClass();
}

/***********************************************************************/

// ULONG __saveds __asm PTEColorPalettePanelDispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
DISPATCHER(PTEColorPalettePanelDispatcher)
{
    char logMessage[256];

    if (!msg->MethodID)
    {
        fileLoggerAddErrorEntry("PTEColorPalettePanel: PTEColorPalettePanelDispatcher called with NULL MethodID");
        loggerFormatMessage(logMessage, "PTEColorPalettePanel: PTEColorPalettePanelDispatcher called with MethodID: 0x%08lx", msg->MethodID);
        fileLoggerAddErrorEntry(logMessage);
        loggerFormatMessage(logMessage, "msg pointer Address: 0x%08lx", (ULONG)msg);
        fileLoggerAddErrorEntry(logMessage);
        loggerFormatMessage(logMessage, "PTEColorPalettePanel: Obj Address: 0x%08lx", (ULONG)obj);
        fileLoggerAddErrorEntry(logMessage);
        loggerFormatMessage(logMessage, "PTEColorPalettePanel: Cl Address: 0x%08lx", (ULONG)cl);
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

static void mDrawToScreen(Object *obj, PTEColorPalettePanelData *data)
{
    struct RastPort *rp;
    WORD left, top, right, bottom;
    char logMessage[256];
    struct ViewPort *vp = NULL;
    struct Screen *scr = NULL;

    fileLoggerAddDebugEntry("PTEColorPalettePanel: Drawing Palette image data");

    // Get RastPort
    rp = _rp(obj);
    if (!rp)
    {
        fileLoggerAddErrorEntry("PTEColorPalettePanel: RastPort unavailable, cannot draw Palette");
        return;
    }

    // Calculate inset bounds
    left = _mleft(obj) + data->borderMargin;
    top = _mtop(obj) + data->borderMargin;
    right = _mright(obj) - data->borderMargin;
    bottom = _mbottom(obj) - data->borderMargin;

    // Check bounds against drawable area
    if (right > _mright(obj) - data->borderMargin)
        right = _mright(obj) - data->borderMargin;
    if (bottom > _mbottom(obj) - data->borderMargin)
        bottom = _mbottom(obj) - data->borderMargin;

    // Get the screen from the window - this is required for direct RGB drawing
    Object *win = getWindowObject(obj);
    if (!win)
    {
        fileLoggerAddErrorEntry("PTEColorPalettePanel: Could not get window object, cannot draw");
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
        // mWritePixels(data, rp, vp, left, top, right, bottom);
        fileLoggerAddDebugEntry("Completed drawing with SetRGB32 for direct RGB rendering");
    }
    else
    {
        fileLoggerAddErrorEntry("No ViewPort available - fallback to direct drawing");
        // Fallback handled below
    }
}

BOOL createPaletteSquares(int left, int top, int squareSize, Square *squares, int numSquares)
{
    if (!squares || numSquares < PALETTE_SIZE || squareSize <= 0)
    {
        fileLoggerAddErrorEntry("createPaletteSquares: Invalid arguments (null pointer, insufficient array size, or non-positive square size)");
        return FALSE;
    }

    for (int i = 0; i < PALETTE_SIZE; ++i)
    {
        int row = i / GRID_SIZE;
        int col = i % GRID_SIZE;
        squares[i].x = left + col * squareSize;
        squares[i].y = top + row * squareSize;
        squares[i].width = squareSize;
        squares[i].height = squareSize;
    }
    return TRUE;
}

// BOOL getScreenViewport(Object *obj, Object *win, struct ViewPort **outVp)
// {
//     struct Screen *scr = NULL;
//     struct ViewPort *vp = NULL;

//     if (!obj || !win || !outVp)
//     {
//         fileLoggerAddErrorEntry("getScreenViewport: Invalid arguments (null pointer)");
//         return FALSE;
//     }

//     // Try to get screen using MUI macro
//     scr = _screen(obj);
//     if (scr)
//     {
//         fileLoggerAddDebugEntry("getScreenViewport: Successfully got screen using _screen() macro");
//         vp = &scr->ViewPort;
//     }
//     else
//     {
//         // Try getting window structure
//         struct Window *window = NULL;
//         get(win, MUIA_Window_Window, &window);

//         if (window && window->WScreen)
//         {
//             scr = window->WScreen;
//             vp = &scr->ViewPort;
//             fileLoggerAddDebugEntry("getScreenViewport: Successfully got screen from Window structure");
//         }
//         else
//         {
//             // Try getting screen directly
//             get(win, MUIA_Window_Screen, &scr);
//             if (scr)
//             {
//                 vp = &scr->ViewPort;
//                 fileLoggerAddDebugEntry("getScreenViewport: Successfully got screen from MUIA_Window_Screen");
//             }
//             else
//             {
//                 fileLoggerAddErrorEntry("getScreenViewport: WARNING - Could not get screen structure, using direct 24-bit drawing without screen info");
//                 *outVp = NULL;
//                 return FALSE;
//             }
//         }
//     }

//     *outVp = vp;
//     return (vp != NULL);
// }
