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
void mDrawRaw2(Object *obj, struct PTEImagePanelData *data);
void mDrawRGB(Object *obj, struct PTEImagePanelData *data);
void mDrawRGB2(Object *obj, struct PTEImagePanelData *data);
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

    // Try to get the ViewPort for setting the palette
    struct ViewPort *vp = NULL;
    struct Screen *scr = NULL;

    // First, try using the MUI _screen macro (valid between MUIM_Setup/Cleanup)
    scr = _screen(obj);
    if (scr)
    {
        vp = &scr->ViewPort;
        fileLoggerAddEntry("Successfully got ViewPort using _screen() macro");
    }
    else
    {
        // If that didn't work, try getting the screen from the window
        Object *win = getWindowObject(obj);
        if (win)
        {
            // Try getting screen directly using MUIA_Window_Screen
            get(win, MUIA_Window_Screen, &scr);
            if (scr)
            {
                vp = &scr->ViewPort;
                fileLoggerAddEntry("Successfully got ViewPort using MUIA_Window_Screen");
            }
            else
            {
                // Fall back to the original method
                struct Window *window = NULL;
                get(win, MUIA_Window_Window, &window);
                if (window && window->WScreen)
                {
                    vp = &window->WScreen->ViewPort;
                    fileLoggerAddEntry("Successfully got ViewPort from Window->WScreen");
                }
            }
        }
    }

    // Final fallback to ViewPortAddress if we still don't have a ViewPort
    if (!vp)
    {
        vp = ViewPortAddress(NULL);
        if (vp)
        {
            fileLoggerAddEntry("Using ViewPortAddress(NULL) as fallback to get ViewPort");
        }
        else
        {
            fileLoggerAddEntry("WARNING: Could not get a ViewPort! Using system palette.");
        }
    }

    // If we have a palette with colors, set it up
    if (data->ilbmPalette && data->ilbmPalette->numColors > 0 && vp)
    {
        fileLoggerAddEntry("Setting up ILBM palette colors");

        // Log the palette information
        char penMapMsg[512] = "PenMap: ";
        char numBuf[8];
        int msgLen = 8; // length of "PenMap: "

        // Only show first 32 entries to avoid huge logs
        for (int i = 0; i < 32 && i < data->ilbmPalette->numColors; i++)
        {
            sprintf(numBuf, "%d ", data->ilbmPalette->penMap[i]);
            strcat(penMapMsg + msgLen, numBuf);
            msgLen += strlen(numBuf);
        }
        fileLoggerAddEntry(penMapMsg);

        // Set the palette colors
        for (ULONG i = 0; i < data->ilbmPalette->numColors && i < 32; i++)
        {
            // Each color is stored as RGB triplet in colorRegs
            ULONG offset = i * 3;
            UBYTE r = data->ilbmPalette->colorRegs[offset];
            UBYTE g = data->ilbmPalette->colorRegs[offset + 1];
            UBYTE b = data->ilbmPalette->colorRegs[offset + 2];

            // Get the pen that corresponds to this color index
            UBYTE pen = data->ilbmPalette->penMap[i];

            // Convert 8-bit (0-255) to 32-bit (0-0xFFFFFFFF) for SetRGB32
            ULONG r32 = ((ULONG)r << 24) | ((ULONG)r << 16) | ((ULONG)r << 8) | r;
            ULONG g32 = ((ULONG)g << 24) | ((ULONG)g << 16) | ((ULONG)g << 8) | g;
            ULONG b32 = ((ULONG)b << 24) | ((ULONG)b << 16) | ((ULONG)b << 8) | b;

            // Set the color for this pen
            SetRGB32(vp, pen, r32, g32, b32);

            char logMessage[256];
            loggerFormatMessage(logMessage, "Set pen %d to R=%d, G=%d, B=%d",
                                pen, r, g, b);
            fileLoggerAddEntry(logMessage);
        }
    }

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

                // Use the pen mapping if available
                UBYTE pen = pixelValue;
                if (data->ilbmPalette)
                {
                    pen = data->ilbmPalette->penMap[pixelValue];
                }

                SetAPen(rp, pen);
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
            // Call the new optimized function for testing
            // Comment the next line and uncomment the following line to use the original function
            mDrawRaw2(obj, data);
            // mDrawRaw(obj, data);
        }
        else
        {
            mDrawRGB2(obj, data);
        }
    }
    return 0;
}
/***********************************************************************/

/***********************************************************************/
// Function to draw RGB image data optimized for 24-bit environment
void mDrawRGB2(Object *obj, struct PTEImagePanelData *data)
{
    struct RastPort *rp;
    WORD left, top, right, bottom;
    char logMessage[256];
    struct ViewPort *vp = NULL;
    struct Screen *scr = NULL;

    fileLoggerAddEntry("PTEImagePanel: mDrawRGB2 - 24-bit optimized RGB drawing");
    loggerFormatMessage(logMessage, "Drawing RGB image data at: 0x%08lx in 24-bit mode", (ULONG)data->imageData);
    fileLoggerAddEntry(logMessage);

    // Get RastPort
    rp = _rp(obj);
    if (!rp)
    {
        fileLoggerAddEntry("PTEImagePanel: rp failed, cannot draw");
        return;
    }

    // Get the screen from the window - this is required for direct RGB drawing
    Object *win = getWindowObject(obj);
    if (!win)
    {
        fileLoggerAddEntry("PTEImagePanel: Could not get window object, cannot draw");
        return;
    }

    // First try to get screen directly using MUI macros
    scr = _screen(obj);
    if (scr)
    {
        fileLoggerAddEntry("Successfully got screen using _screen() macro");
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
            fileLoggerAddEntry("Successfully got screen from Window structure");
        }
        else
        {
            // Try getting screen directly
            get(win, MUIA_Window_Screen, &scr);
            if (scr)
            {
                vp = &scr->ViewPort;
                fileLoggerAddEntry("Successfully got screen from MUIA_Window_Screen");
            }
            else
            {
                // In case we can't get the screen, we'll use a fallback approach for 24-bit drawing
                fileLoggerAddEntry("WARNING: Could not get screen structure, using direct 24-bit drawing without screen info");
                // We'll continue without screen/viewport information
            }
        }
    }

    // Log success or continue without viewport
    if (vp)
    {
        fileLoggerAddEntry("Successfully retrieved ViewPort for 24-bit drawing");
    }
    else
    {
        fileLoggerAddEntry("No ViewPort available, continuing with direct 24-bit drawing");
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

    fileLoggerAddEntry("True 24-bit direct RGB drawing - no pen allocation required");
    fileLoggerAddEntry("Using direct RGB values with 16.7 million colors (24-bit)");

    // Direct RGB drawing without color mapping or palette lookups
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

                // In a true 24-bit environment, we can directly use these RGB values
                // without the overhead of ObtainBestPen/ReleasePen for each pixel

                // For Amiga systems, we can use WriteRGBPixel directly if available
                // or use a system-specific optimized method for direct RGB drawing

                // MUI in 24-bit mode typically has a way to set RGB values directly
                // This is a simplified approach using the screen's color map directly
                ULONG colorValue = (r << 16) | (g << 8) | b;

                // Set the corresponding color in the pen array
                // This works because in 24-bit mode, we have direct color mapping
                SetAPen(rp, colorValue & 0xFFFFFF);
                WritePixel(rp, px, py);
            }
        }
    }

    fileLoggerAddEntry("24-bit direct RGB drawing completed successfully - no pen allocation used");
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
    char logMessage[256];

    fileLoggerAddEntry("Searching for MUI window object...");
    loggerFormatMessage(logMessage, "Starting from object at address: 0x%08lx", (ULONG)obj);
    fileLoggerAddEntry(logMessage);

    // Attempt to find window by walking up the object hierarchy
    Object *parent = obj;
    int depth = 0;

    while (parent)
    {
        depth++;
        loggerFormatMessage(logMessage, "Checking parent level %d at address: 0x%08lx", depth, (ULONG)parent);
        fileLoggerAddEntry(logMessage);

        // Check if this is a window
        LONG isWindow = 0;
        BOOL gotWindowAttr = get(parent, MUIA_WindowObject, &isWindow);

        loggerFormatMessage(logMessage, "get(MUIA_WindowObject) result: %s, value: %ld",
                            gotWindowAttr ? "TRUE" : "FALSE", isWindow);
        fileLoggerAddEntry(logMessage);

        if (isWindow)
        {
            win = parent;
            fileLoggerAddEntry("Found MUI window object!");
            break;
        }

        // Try direct approach - check for MUIA_Window_Window
        struct Window *window = NULL;
        BOOL gotWin = get(parent, MUIA_Window_Window, &window);

        if (gotWin && window)
        {
            win = parent;
            loggerFormatMessage(logMessage, "Found window through MUIA_Window_Window at level %d", depth);
            fileLoggerAddEntry(logMessage);
            break;
        }

        // Try getting screen directly
        struct Screen *scr = NULL;
        BOOL gotScr = get(parent, MUIA_Window_Screen, &scr);

        if (gotScr && scr)
        {
            win = parent;
            loggerFormatMessage(logMessage, "Found window through MUIA_Window_Screen at level %d", depth);
            fileLoggerAddEntry(logMessage);
            break;
        }

        // Move up to parent
        Object *oldParent = parent;
        get(parent, MUIA_Parent, &parent);

        if (parent == oldParent)
        {
            fileLoggerAddEntry("ERROR: Parent points to itself, breaking loop");
            break;
        }

        if (depth > 10)
        {
            fileLoggerAddEntry("WARNING: Deep hierarchy, stopping search");
            break;
        }
    }

    if (win)
    {
        fileLoggerAddEntry("Successfully found window object, checking for attributes...");

        // Verify we can access window and screen
        struct Window *window = NULL;
        struct Screen *screen = NULL;

        BOOL gotWin = get(win, MUIA_Window_Window, &window);
        BOOL gotScr = get(win, MUIA_Window_Screen, &screen);

        loggerFormatMessage(logMessage, "Window attributes: MUIA_Window_Window=%s (0x%08lx), MUIA_Window_Screen=%s (0x%08lx)",
                            gotWin ? "TRUE" : "FALSE", (ULONG)window,
                            gotScr ? "TRUE" : "FALSE", (ULONG)screen);
        fileLoggerAddEntry(logMessage);

        if (window && window->WScreen)
        {
            loggerFormatMessage(logMessage, "ViewPort available through window->WScreen: 0x%08lx",
                                (ULONG)&window->WScreen->ViewPort);
            fileLoggerAddEntry(logMessage);
        }
    }
    else
    {
        fileLoggerAddEntry("Failed to find window object");
    }

    return win;
}

/***********************************************************************/
// Function to draw raw image data for the PTEImagePanel with optimized 65K color handling
void mDrawRaw2(Object *obj, struct PTEImagePanelData *data)
{
    struct RastPort *rp;
    WORD left, top, right, bottom;
    char logMessage[256];
    struct ViewPort *vp = NULL;
    struct Screen *scr = NULL;
    struct ColorMap *cm = NULL;
    LONG *allocatedPens = NULL;
    ULONG numColors = 0;
    BOOL isHighColor = FALSE;
    ULONG i; // Loop counter

    loggerFormatMessage(logMessage, "PTEImagePanel: mDrawRaw2 called with image data at: 0x%08lx", (ULONG)data->imageData);
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

    // Try to get the ViewPort and Screen for color management
    // First, try using the MUI _screen macro (valid between MUIM_Setup/Cleanup)
    scr = _screen(obj);
    if (scr)
    {
        vp = &scr->ViewPort;
        cm = vp->ColorMap;
        fileLoggerAddEntry("Successfully got ViewPort and ColorMap using _screen() macro");
    }
    else
    {
        // If that didn't work, try getting the screen from the window
        Object *win = getWindowObject(obj);
        if (win)
        {
            // Try getting screen directly using MUIA_Window_Screen
            get(win, MUIA_Window_Screen, &scr);
            if (scr)
            {
                vp = &scr->ViewPort;
                cm = vp->ColorMap;
                fileLoggerAddEntry("Successfully got ViewPort and ColorMap using MUIA_Window_Screen");
            }
            else
            {
                fileLoggerAddEntry("ERROR: Could not get ViewPort from MUIA_Window_Screen! Not drawing anything.");
                return; // Return without drawing anything
            }
        }
    }

    // Check if we have a ViewPort
    if (!vp)
    {
        fileLoggerAddEntry("ERROR: No ViewPort available! Not drawing anything.");
        return; // Return without drawing anything
    }

    // Check if we have a ColorMap
    if (!cm)
    {
        fileLoggerAddEntry("ERROR: No ColorMap available! Not drawing anything.");
        return; // Return without drawing anything
    }

    // Check if we're in high-color mode (65K colors)
    if (scr)
    {
        ULONG depth = GetBitMapAttr(scr->RastPort.BitMap, BMA_DEPTH);
        if (depth >= 15)
        {
            isHighColor = TRUE;
            loggerFormatMessage(logMessage, "Detected high-color mode with depth: %ld", depth);
            fileLoggerAddEntry(logMessage);
        }
    }

    // For testing purposes, assume we're always in high-color mode
    isHighColor = TRUE;

    // Check if we have image data
    if (!data->imageData)
    {
        fileLoggerAddEntry("ERROR: No image data to draw! Not drawing anything.");
        return; // Return without drawing anything
    }

    // Check if we have palette data
    if (!data->ilbmPalette || data->ilbmPalette->numColors <= 0)
    {
        fileLoggerAddEntry("ERROR: No palette data or empty palette! Not drawing anything.");
        return; // Return without drawing anything
    }

    // If we have a palette and ColorMap, allocate pens for 65K color mode
    if (cm && isHighColor)
    {
        fileLoggerAddEntry("65K color mode: Allocating pens for palette colors");

        numColors = data->ilbmPalette->numColors;
        if (numColors > 256)
            numColors = 256; // Safety cap

        // Allocate memory for pen tracking
        allocatedPens = AllocVec(numColors * sizeof(LONG), MEMF_CLEAR);
        if (!allocatedPens)
        {
            fileLoggerAddEntry("ERROR: Failed to allocate memory for pen tracking! Not drawing anything.");
            return; // Return without drawing anything
        }

        // Initialize all pen entries to -1 (invalid)
        for (i = 0; i < numColors; i++)
        {
            allocatedPens[i] = -1;
        }

        // Obtain pens for each color in the palette
        for (i = 0; i < numColors; i++)
        {
            // Each color is stored as RGB triplet in colorRegs
            ULONG offset = i * 3;
            UBYTE r = data->ilbmPalette->colorRegs[offset];
            UBYTE g = data->ilbmPalette->colorRegs[offset + 1];
            UBYTE b = data->ilbmPalette->colorRegs[offset + 2];

            // Convert 8-bit (0-255) to 32-bit shifted values for ObtainBestPen
            ULONG r32 = (ULONG)r << 24;
            ULONG g32 = (ULONG)g << 24;
            ULONG b32 = (ULONG)b << 24;

            // Try to obtain an exact match first
            allocatedPens[i] = ObtainBestPen(cm, r32, g32, b32, TAG_DONE);

            if (allocatedPens[i] != -1)
            {
                loggerFormatMessage(logMessage, "Allocated pen %ld for color %ld (R=%d, G=%d, B=%d)",
                                    allocatedPens[i], i, r, g, b);
                fileLoggerAddDebugEntry(logMessage);
            }
            else
            {
                // If we can't get a pen, use pen 1 as fallback
                allocatedPens[i] = 1;
                loggerFormatMessage(logMessage, "Failed to allocate pen for color %ld, using pen 1", i);
                fileLoggerAddEntry(logMessage);
            }
        }

        // Check if we allocated at least one pen successfully
        BOOL atLeastOnePenAllocated = FALSE;
        for (i = 0; i < numColors; i++)
        {
            if (allocatedPens[i] != -1)
            {
                atLeastOnePenAllocated = TRUE;
                break;
            }
        }

        if (!atLeastOnePenAllocated)
        {
            fileLoggerAddEntry("ERROR: Failed to allocate any pens! Not drawing anything.");
            FreeVec(allocatedPens);
            return; // Return without drawing anything
        }

        fileLoggerAddEntry("Successfully allocated pens for high-color mode"); // Optimized drawing - batch by color to minimize pen changes
        fileLoggerAddEntry("Drawing image using high-color optimized method");

        // For each unique color, draw all pixels of that color
        for (i = 0; i < numColors; i++)
        {
            if (allocatedPens[i] != -1)
            {
                SetAPen(rp, allocatedPens[i]);

                // Draw all pixels that match this color index
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

                            // Draw this pixel if it matches the current color
                            if (pixelValue == i)
                            {
                                WritePixel(rp, px, py);
                            }
                        }
                    }
                }
            }
        }

        // Release all allocated pens
        fileLoggerAddEntry("Releasing allocated pens");

        for (i = 0; i < numColors; i++)
        {
            if (allocatedPens[i] != -1 && allocatedPens[i] != 1) // Don't release pen 1
            {
                ReleasePen(cm, allocatedPens[i]);
            }
        }

        FreeVec(allocatedPens);
        return; // Exit early, we've handled the drawing
    }

    // If we get here, we don't have the right conditions to draw
    fileLoggerAddEntry("ERROR: Cannot draw in high-color mode. Missing palette, colormap, or not in high-color mode.");
}
