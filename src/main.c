/*
 * Sample AmigaOS 3.1 program with MUI 3.8
 * Compiled with VBCC
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <libraries/mui.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/filelogger.h"
#include "utils/windowlogger.h"
#include "views/mainmenu.h"
#include "widgets/listwidgets.h"
#include "views/aboutview.h"
#include "widgets/pteimagepanel.h"
#include "widgets/widgetconstants.h"
#include "widgets/ptecolorpalettepanel.h"
#include "graphics/graphics.h"
#include "graphics/png/imgpngutils.h"
#include "graphics/png/pngutils.h"

/* MUI Libraries */
struct Library *MUIMasterBase = NULL;

/* Function prototypes */
BOOL init_libs(void);
void cleanup_libs(void);
Object *create_gui(void);

static APTR list, aboutView;

int main(void)
{
    APTR app, window, strip, bt1, imagePanel;
    ULONG sigs = 0;
    BOOL running = TRUE;
    UBYTE *outImageData = NULL;
    UBYTE *outRGBImageData = NULL;

    /* Initialize libraries */
    if (!init_libs())
    {
        printf("Failed to initialize libraries!\n");
        return RETURN_FAIL;
    }

    // Initialize
    fileLoggerInit("papertanks.log");
    fileLoggerSetDebug(TRUE);

    // Log messages
    fileLoggerAddEntry("Application started");

    // Init Widget classes
    initializePTEImagePanel();
    initializePTEColorPalettePanel();

    if (!pteImagePanelClass || !pteColorPalettePanelClass)
    {
        fileLoggerAddErrorEntry("Failed to create PTEImagePanel or PTEColorPalettePanel class");
        cleanup_libs();
        return RETURN_FAIL;
    }

    if (!pteImagePanelClass->mcc_Class || !pteColorPalettePanelClass->mcc_Class)
    {
        fileLoggerAddErrorEntry("Failed to create PTEImagePanel or PTEColorPalettePanel class instance");
        cleanup_libs();
        return RETURN_FAIL;
    }

    /* PNG Test */
    fileLoggerAddInfoEntry("Testing PNG loading capability...");
    PNGImage *pngImageData = NULL;
    BOOL pngLoaded = loadPNGToBitmapObject("PROGDIR:assets/ui/tank.png", &pngImageData);

    /* PNG Test 2 */
    fileLoggerAddInfoEntry("Testing PNG loading capability...");
    PNGImage *pngImageData2 = NULL;
    Img8BitPalette *outImgPalette = NULL;
    BOOL pngLoaded2 = loadPNGToBitmapObject("PROGDIR:assets/tank2.png", &pngImageData2);
    if (!loadPaletteFromPNGImage32(pngImageData2, &outImgPalette))
    {
        fileLoggerAddErrorEntry("Failed to load palette from PNG image");
    }

    if (pngLoaded && pngLoaded2)
    {
        fileLoggerAddInfoEntry("PNG image loaded successfully");
        // We'll use this PNG data in our PTEImagePanel
    }
    else
    {
        fileLoggerAddErrorEntry("Failed to load PNG image");
        // Clean up in case of partial initialization
        if (pngImageData)
        {
            free(pngImageData);
            pngImageData = NULL;
        }

        if (pngImageData2)
        {
            free(pngImageData2);
            pngImageData2 = NULL;
        }
    }

    /* clang-format off */

    /* Create the GUI */
    app = ApplicationObject,
        MUIA_Application_Title, "Paper Tanks Editor",
        MUIA_Application_Version, "$VER: 0.1.0",
        MUIA_Application_Copyright, "(C) 2025 Matt Swart",
        MUIA_Application_Author, "Matt Swart",
        MUIA_Application_Description, "Paper Tanks Editor.",
        MUIA_Application_Base, "SHOWHIDE",        

        MUIA_Application_Window, window = WindowObject,

            MUIA_Window_Title, "Paper Tanks Editor",
            MUIA_Window_ID, MAKE_ID('T', 'A', 'N', 'K'),
            MUIA_Window_Width, 800,
            MUIA_Window_Height, 600,
            MUIA_Window_Menustrip, strip = MUI_MakeObject(MUIO_MenustripNM, MainMenuData, MUIO_MenustripNM_CommandKeyCheck),

                WindowContents, VGroup, GroupFrame,
                    
                    Child, VGroup,
                        Child, RectangleObject, MUIA_Width, 100, MUIA_Height, 50, MUIA_Background, MUII_ButtonBack, MUIA_Frame, "box", End, 
                        // Create the custom object with attributes in MUI style                                               
                        Child, PTEImagePanelObject,  
                            MUIA_Width, 100,
                            MUIA_Height, 50,
                            MUIA_Background, MUII_ButtonBack,
                            PTEA_BorderColor, 1,
                            PTEA_BorderMargin, 5,
                            PTEA_DrawBorder, TRUE,
                            PTEA_ImageData, pngImageData2->data,
                            PTEA_ImageHeight, pngImageData2->height,
                            PTEA_ImageWidth, pngImageData2->width,
                            PTEA_HasTransparency, pngImageData2->hasTransparency,
                            PTEA_IsPNG, TRUE,
                        End,
                        Child, PTEColorPalettePanelObject,
                            MUIA_Width, 100,
                            MUIA_Height, 50,
                            MUIA_Background, MUII_ButtonBack,
                            PTEA_BorderColor, 1,
                            PTEA_BorderMargin, 10,
                            PTEA_DrawBorder, TRUE,
                            PTEA_ColorPalette, outImgPalette,
                        End,
                    End,

                    Child, VGroup, GroupFrameT("Status Messages"),                    
                        Child, list = List2("Ready...", 35),
                    End,                   

                End,                
            End,        
        End;

    /* clang-format on */

    if (!app)
    {
        printf("Failed to create MUI application!\n");
        cleanup_libs();
        return RETURN_FAIL;
    }

    /* Set up notifications */
    DoMethod(window, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

    /* Open the window */
    set(window, MUIA_Window_Open, TRUE);

    /* Init UI Status Messages */
    windowLoggerInit(list);

    /* Main event loop */
    while (running)
    {
        ULONG id = DoMethod(app, MUIM_Application_NewInput, &sigs);

        switch (id)
        {
        case MUIV_Application_ReturnID_Quit:
        case MEN_QUIT:
            fileLoggerAddEntry("Quite Called");
            running = FALSE;
            break;
        case MEN_ABOUT:
            createAboutView(app, pngImageData->data);
            break;
        }

        if (running && sigs)
            Wait(sigs);
    }

    /* Clean up */
    set(window, MUIA_Window_Open, FALSE);
    MUI_DisposeObject(app);

    /* Free allocated resources */
    if (pngImageData)
        free(pngImageData);

    if (pngImageData2)
    {
        free(pngImageData2);
        pngImageData2 = NULL;
    }

    cleanup_libs();

    // Close when done
    fileLoggerClose();

    return RETURN_OK;
}

BOOL init_libs(void)
{
    /* Open MUI Master Library */
    MUIMasterBase = OpenLibrary(MUIMASTER_NAME, MUIMASTER_VMIN);
    if (!MUIMasterBase)
    {
        printf("Cannot open %s version %d or higher!\n", MUIMASTER_NAME, MUIMASTER_VMIN);
        return FALSE;
    }

    return TRUE;
}

void cleanup_libs(void)
{
    if (MUIMasterBase)
    {
        CloseLibrary(MUIMasterBase);
        MUIMasterBase = NULL;
    }
}