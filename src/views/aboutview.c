#include "aboutview.h"

void createAboutView(Object *app, UBYTE *pngImageData, ImgPalette *pngPalette)
{
    APTR list;
    static const char IN_About[] = "Paper Tanks Editor is the editor to create new levels and scenarios for the game Paper Tanks.\
                                    \nThis editor can also edit Tanks and add new ones.\
                                    \nThis editor is written in C using the MUI GUI toolkit.";

    windowLoggerAddEntry("Creating About View...");

    /* clang-format off */

    APTR subWindow = DefaultSubWindow("About", MAKE_ID('A', 'B', 'T', 'T'), 400, 200),

        VGroup, GroupFrame,
            MUIA_Group_Spacing, 4,
                Child, HGroup,
                    Child, VGroup,
                        Child, HGroup,                      
                            Child, PTEImagePanelObject,
                                MUIA_Background, MUII_ButtonBack,
                                PTEA_ImageData, pngImageData,
                                PTEA_ImageHeight, 25,
                                PTEA_ImageWidth, 25,
                                PTEA_ImgPalette, pngPalette,
                                PTEA_IsPNG, TRUE,
                            End,
                            Child, VSpace (35),
                        End,
                        Child, HSpace (35),                        
                        // Child, ScaledImage(MUII_HardDisk, 14, 8),
                        Child, RectangleObject, End,
                End,
                Child, list = List(IN_About),
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
