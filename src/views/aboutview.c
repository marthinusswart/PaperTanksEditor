#include "aboutview.h"

void createAboutView(Object *app, UBYTE *pngImageData)
{
    APTR list;
    static const char IN_About[] = "Paper Tanks Editor is the editor to create new levels and scenarios for the game Paper Tanks.\
                                    \nThis editor can also edit Tanks and add new ones.\
                                    \nThis editor is written in C using the MUI GUI toolkit.";

    windowLoggerAddEntry("Creating About View...");

    /* clang-format off */

    APTR subWindow = DefaultSubWindow("About", MAKE_ID('A', 'B', 'T', 'T'), 600, 300),

        VGroup, GroupFrame,
            MUIA_Group_Spacing, 4,
                Child, HGroup,
                    Child, VGroup,
                        Child, HGroup,                      
                            Child, PTEImagePanelObject,
                                MUIA_Background, MUII_ButtonBack,
                                PTEA_ImageData, NULL,
                                PTEA_ImageHeight, 100,
                                PTEA_ImageWidth, 100,                                
                                PTEA_IsPNG, TRUE,
                            End,
                            Child, VSpace (105),
                        End,
                        Child, HSpace (105),
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
