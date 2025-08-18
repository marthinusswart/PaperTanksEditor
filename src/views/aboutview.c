#include "aboutview.h"

void createAboutView(Object *app)
{
    APTR list;
    Object *customImg;
    static const char IN_About[] = "Paper Tanks Editor is the editor to create new levels and scenarios for the game Paper Tanks.\
                                    \nThis editor can also edit Tanks and add new ones.\
                                    \nThis editor is written in C using the MUI GUI toolkit.";

    UBYTE *logo = NULL;
    // if (!loadILBMToBitmapObject("PROGDIR:assets/HardDisk.mbr", &logo))
    // {
    //     loggerAddEntry("Failed to load logo image. Using placeholder.");
    // }
    // else
    // {
    //     loggerAddEntry("Logo image loaded successfully.");
    //     // Create custom image area
    //     if (!createCustomImageArea(logo, 14, 8, &customImg))
    //     {
    //         loggerAddEntry("Failed to create custom image area for logo.");
    //         logo = NULL; // Clear logo if creation failed
    //     };
    // }

    windowLoggerAddEntry("Creating About View...");

    /* clang-format off */

    APTR subWindow = DefaultSubWindow("About", MAKE_ID('A', 'B', 'T', 'T'), 400, 200),

        VGroup, GroupFrame,
            MUIA_Group_Spacing, 4,
                Child, HGroup,
                    Child, VGroup,                    
                        Child, logo ? customImg : ScaledImage(MUII_HardDisk, 14, 8),
                        Child, RectangleObject,
                    End, // Top area placeholder
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
