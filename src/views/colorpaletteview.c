#include "colorpaletteview.h"

void createColorPaletteView(Object *app)
{

    windowLoggerAddEntry("Creating About View...");

    /* clang-format off */

    APTR subWindow = DefaultSubWindow("Color Palette", MAKE_ID('C', 'O', 'L', 'P'), 600, 300),

        VGroup, GroupFrame,
            Child, PTEColorPalettePanelObject,
                MUIA_Width, 100,
                MUIA_Height, 50,
                MUIA_Background, MUII_ButtonBack,
                PTEA_BorderColor, 1,
                PTEA_BorderMargin, 0,
                PTEA_DrawBorder, TRUE,
                PTEA_ColorPalette, outImgPalette,
            End,                  
        End,
    End;

    /* clang-format on */

    if (subWindow)
    {
        DoMethod(app, OM_ADDMEMBER, subWindow);
        DoMethod(subWindow, MUIM_Set, MUIA_Window_Open, TRUE);
        DoMethod(subWindow, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, subWindow, 3, MUIM_Set, MUIA_Window_Open, FALSE);
    }
}
