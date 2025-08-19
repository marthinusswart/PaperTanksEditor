// Include your header
#include "pteimagepanel.h"

/***********************************************************************/

/********************** Prototypes *************************/
extern struct Library *MUIMasterBase;
/***********************************************************************/

DISPATCHER(PTEImagePanelDispatcher);
IPTR SAVEDS mNew(struct IClass *cl, Object *obj, struct opSet *msg);
IPTR SAVEDS mDraw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg);
void mDrawBorder(Object *obj, struct PTEImagePanelData *data);
void mDrawRaw(Object *obj, struct PTEImagePanelData *data);
void mDrawRGB(Object *obj, struct PTEImagePanelData *data);
LONG xget(Object *obj, ULONG attribute);
Object *getWindowObject(Object *obj);

struct MUI_CustomClass *pteImagePanelClass;

static ULONG STACKARGS DoSuperNew(struct IClass *const cl, Object *const obj, const ULONG tags, ...)
{
    return (DoSuperMethod(cl, obj, OM_NEW, &tags, NULL));
}

struct MUI_CustomClass *createPTEImagePanelClass(void)
{
    char logMessage[256];
    fileLoggerAddEntry("PTEImagePanel: Creating MUI_CreateCustomClass");
    loggerFormatMessage(logMessage, "Dispatcher address: 0x%08lx", PTEImagePanelDispatcher);
    fileLoggerAddEntry(logMessage);
    struct MUI_CustomClass *obj = MUI_CreateCustomClass(NULL, MUIC_Rectangle, NULL, sizeof(struct PTEImagePanelData), PTEImagePanelDispatcher);
    if (!obj)
    {
        fileLoggerAddEntry("PTEImagePanel: Failed to create custom class");
        return NULL;
    }
    else
    {
        fileLoggerAddEntry("PTEImagePanel: Custom class created successfully");
        loggerFormatMessage(logMessage, "Custom class obj address: 0x%08lx", (ULONG)obj);
        fileLoggerAddEntry(logMessage);
        return obj;
    }
}

/***********************************************************************/

/***********************************************************************/

IPTR SAVEDS mNew(struct IClass *cl, Object *obj, struct opSet *msg)
{

    fileLoggerAddEntry("PTEImagePanel: mNew called");

    obj = (Object *)DoSuperNew(cl, obj, TAG_MORE, msg->ops_AttrList);

    if (!obj)
    {
        fileLoggerAddEntry("PTEImagePanel: Failed to call Super");
        return 0;
    }

    // Default values
    BYTE borderColor = 1; // Black
    WORD borderMargin = 10;
    BOOL drawBorder = TRUE;
    UBYTE *imageData = NULL;
    WORD imageHeight = 0;
    WORD imageWidth = 0;
    BOOL enableRGB = FALSE;
    ILBMPalette *ilbmPalette = NULL;

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

        case PTEA_ImageWidth:
            imageWidth = (WORD)walk->ti_Data;
            break;

        case PTEA_EnableRGB:
            enableRGB = (BOOL)walk->ti_Data;
            break;

        case PTEA_ILBMPalette:
            ilbmPalette = (ILBMPalette *)walk->ti_Data;
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
    data->enableRGB = enableRGB;
    data->ilbmPalette = ilbmPalette;

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
        fileLoggerAddEntry("PTEImagePanel: rp failed...");
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
    fileLoggerAddEntry(logMessage);

    // Draw rectangle border
    Move(rp, left, top);
    Draw(rp, right, top);
    Draw(rp, right, bottom);
    Draw(rp, left, bottom);
    Draw(rp, left, top); // Close the loop
}

/***********************************************************************/
// Function to draw raw image data for the PTEImagePanel
void mDrawRaw(Object *obj, struct PTEImagePanelData *data)
{
    struct RastPort *rp;
    WORD left, top, right, bottom;
    char logMessage[256];

    loggerFormatMessage(logMessage, "PTEImagePanel: we have image data at: 0x%08lx", (ULONG)data->imageData);
    fileLoggerAddEntry(logMessage);

    // Get RastPort
    rp = _rp(obj);
    if (!rp)
    {
        fileLoggerAddEntry("PTEImagePanel: rp failed...");
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
    right = left + right - 1;
    bottom = top + bottom - 1;

    // Draw image inside drawable area
    for (WORD y = 0; y < data->imageHeight; y++)
    {
        for (WORD x = 0; x < data->imageWidth; x++)
        {
            LONG px = left + x;
            LONG py = top + y;

            // Clip to drawable area
            if (px <= right && py <= bottom)
            {
                UBYTE pixelValue = data->imageData[y * data->imageWidth + x];
                SetAPen(rp, pixelValue);
                WritePixel(rp, px, py);
            }
        }
    }
}

/***********************************************************************/
// Function to draw RGB image data for the PTEImagePanel
void mDrawRGB(Object *obj, struct PTEImagePanelData *data)
{
    struct RastPort *rp;
    WORD left, top, right, bottom;
    char logMessage[256];

// Amiga hardware has limited color palette
#define MAX_COLORS 16 // Use 16 pens (excluding pen 0 which is often the background)
    UBYTE redValues[MAX_COLORS];
    UBYTE greenValues[MAX_COLORS];
    UBYTE blueValues[MAX_COLORS];
    BOOL colorUsed[MAX_COLORS];
    UBYTE colorCount = 0;

    // Two-pass approach: first identify all unique colors, then draw
    UBYTE colorMap[16][16][16]; // Maps 4-bit RGB to pen numbers

    // Get the ViewPort once outside the pixel loop
    struct ViewPort *vp = NULL;
    struct Screen *scr = NULL;

    loggerFormatMessage(logMessage, "PTEImagePanel: we have RGB image data at: 0x%08lx", (ULONG)data->imageData);
    fileLoggerAddEntry(logMessage);

    // Get RastPort
    rp = _rp(obj);
    if (!rp)
    {
        fileLoggerAddEntry("PTEImagePanel: rp failed...");
        return;
    }

    // Get the screen properly from the window that contains this object
    Object *win = getWindowObject(obj);
    if (win)
    {
        // Get the screen from the window
        struct Window *window = NULL;
        get(win, MUIA_Window_Window, &window);
        if (window)
        {
            scr = window->WScreen;
            if (scr)
            {
                vp = &scr->ViewPort;
                fileLoggerAddDebugEntry("Successfully retrieved ViewPort from window");
            }
        }
    }

    // Fallback to ViewPortAddress if we couldn't get it through MUI
    if (!vp)
    {
        vp = ViewPortAddress(NULL);
        fileLoggerAddDebugEntry("Falling back to ViewPortAddress(NULL)");
    }

    if (!vp)
    {
        fileLoggerAddEntry("PTEImagePanel: No viewport available, cannot draw RGB image");
        return;
    }

    // Initialize color tables
    for (UBYTE i = 0; i < MAX_COLORS; i++)
    {
        colorUsed[i] = FALSE;
    }

    // Initialize the color map to invalid values
    for (UBYTE r = 0; r < 16; r++)
    {
        for (UBYTE g = 0; g < 16; g++)
        {
            for (UBYTE b = 0; b < 16; b++)
            {
                colorMap[r][g][b] = 255; // Invalid pen number
            }
        }
    }

    // First pass: Find all unique colors in the image and build color map
    fileLoggerAddEntry("First pass: Identifying unique colors in the image");
    for (WORD y = 0; y < data->imageHeight; y++)
    {
        for (WORD x = 0; x < data->imageWidth; x++)
        {
            // Calculate offset into RGB chunky data (3 bytes per pixel)
            ULONG offset = (y * data->imageWidth + x) * 3;

            // Get RGB components (bytes are packed R,G,B consecutively)
            UBYTE r = data->imageData[offset];
            UBYTE g = data->imageData[offset + 1];
            UBYTE b = data->imageData[offset + 2];

            // Convert 8-bit RGB to the 4-bit per component Amiga format (0-15)
            UBYTE r4 = (r >> 4) & 0xF;
            UBYTE g4 = (g >> 4) & 0xF;
            UBYTE b4 = (b >> 4) & 0xF;

            // If this color isn't already in our map, add it
            if (colorMap[r4][g4][b4] == 255 && colorCount < MAX_COLORS)
            {
                // Assign a pen to this color
                colorMap[r4][g4][b4] = colorCount;

                // Store color values
                redValues[colorCount] = r4;
                greenValues[colorCount] = g4;
                blueValues[colorCount] = b4;
                colorUsed[colorCount] = TRUE;

                colorCount++;

                // Log the detected color
                char colorMsg[64];
                sprintf(colorMsg, "Color %d: R=%d, G=%d, B=%d", colorCount, r4, g4, b4);
                fileLoggerAddDebugEntry(colorMsg);

                // Stop if we've reached the maximum number of colors
                if (colorCount >= MAX_COLORS)
                {
                    fileLoggerAddEntry("Maximum number of colors reached (16)");
                    break;
                }
            }
        }

        if (colorCount >= MAX_COLORS)
            break;
    }

    // If we didn't find 16 colors, we'll still have valid entries up to colorCount
    sprintf(logMessage, "Found %d unique colors in the image", colorCount);
    fileLoggerAddEntry(logMessage);

    // Set the palette with our identified colors
    for (UBYTE i = 0; i < colorCount; i++)
    {
        SetRGB4(vp, i + 1, redValues[i], greenValues[i], blueValues[i]);
    }

    // Calculate inset bounds
    left = _mleft(obj) + data->borderMargin;
    top = _mtop(obj) + data->borderMargin;
    right = _mright(obj) - data->borderMargin;
    bottom = _mbottom(obj) - data->borderMargin;

    // Convert width/height to right/bottom
    left += 5;
    top += 5;
    right = left + right - 1;
    bottom = top + bottom - 1;

    // Second pass: Draw the image using our 1:1 color mapping
    fileLoggerAddEntry("Second pass: Drawing image with exact color mapping");
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

                // Get RGB components (bytes are packed R,G,B consecutively)
                UBYTE r = data->imageData[offset];
                UBYTE g = data->imageData[offset + 1];
                UBYTE b = data->imageData[offset + 2];

                // Convert 8-bit RGB to the 4-bit per component Amiga format (0-15)
                UBYTE r4 = (r >> 4) & 0xF;
                UBYTE g4 = (g >> 4) & 0xF;
                UBYTE b4 = (b >> 4) & 0xF;

                // Look up the pen from our color map
                UBYTE pen = colorMap[r4][g4][b4];

                // If this color wasn't in our map (which shouldn't happen if we have ?16 colors),
                // use pen 1 as a fallback
                if (pen == 255)
                {
                    pen = 1;
                }
                else
                {
                    // Add 1 to the pen because pens start at 1 (0 is often the background)
                    pen = pen + 1;
                }

                // Set the pen and draw the pixel
                SetAPen(rp, pen);
                WritePixel(rp, px, py);
            }
        }
    }

    fileLoggerAddEntry("RGB image drawing completed with 1:1 color mapping");
}

/***********************************************************************/
IPTR SAVEDS mDraw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg)
{
    fileLoggerAddEntry("PTEImagePanel: mDraw called");

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
        if (!data->enableRGB)
        {
            mDrawRaw(obj, data);
        }
        else
        {
            mDrawRGB(obj, data);
        }
    }
    return 0;
}
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
    fileLoggerAddDebugEntry("PTEImagePanel: PTEImagePanelDispatcher called");

    if (!msg->MethodID)
    {
        fileLoggerAddEntry("PTEImagePanel: PTEImagePanelDispatcher called with NULL MethodID");
        loggerFormatMessage(logMessage, "PTEImagePanel: PTEImagePanelDispatcher called with MethodID: 0x%08lx", msg->MethodID);
        fileLoggerAddEntry(logMessage);
        loggerFormatMessage(logMessage, "msg pointer Address: 0x%08lx", (ULONG)msg);
        fileLoggerAddEntry(logMessage);
        loggerFormatMessage(logMessage, "PTEImagePanel: Obj Address: 0x%08lx", (ULONG)obj);
        fileLoggerAddEntry(logMessage);
        loggerFormatMessage(logMessage, "PTEImagePanel: Cl Address: 0x%08lx", (ULONG)cl);
        fileLoggerAddEntry(logMessage);
        return 0;
    }

    // loggerFormatMessage(logMessage, "PTEImagePanel: PTEImagePanelDispatcher called with MethodID: 0x%08lx", msg->MethodID);
    // fileLoggerAddEntry(logMessage);
    // loggerFormatMessage(logMessage, "msg pointer: 0x%08lx", (ULONG)msg);
    // fileLoggerAddEntry(logMessage);

    switch (msg->MethodID)
    {
    case OM_NEW:
        return mNew(cl, obj, (APTR)msg);
        // case OM_DISPOSE:                    return mDispose(cl, obj, (APTR)msg);
        // case OM_GET:                        return mGet(cl, obj, (APTR)msg);
        // case OM_SET:                        return mSets(cl, obj, (APTR)msg);
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

Object *getWindowObject(Object *obj)
{
    Object *win = NULL;

    // Attempt to find window by walking up the object hierarchy
    Object *parent = obj;
    while (parent)
    {
        // Check if this is a window
        LONG isWindow = 0;
        get(parent, MUIA_WindowObject, &isWindow);
        if (isWindow)
        {
            win = parent;
            break;
        }

        // Move up to parent
        get(parent, MUIA_Parent, &parent);
    }

    fileLoggerAddDebugEntry(win ? "Found window object" : "Window object not found");
    return win;
}
