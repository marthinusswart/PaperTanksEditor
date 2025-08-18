#ifndef LISTWIDGETS_H
#define LISTWIDGETS_H

/* clang-format off */

#define List(ftxt) \
    ListviewObject, \
        MUIA_Weight, 150, \
        MUIA_Listview_Input, FALSE, \
        MUIA_Listview_List, \
            FloattextObject, \
                MUIA_Frame, MUIV_Frame_ReadList, \
                MUIA_Background, MUII_ReadListBack, \
                MUIA_Floattext_Text, ftxt, \
                MUIA_Floattext_TabSize, 4, \
                MUIA_Floattext_Justify, FALSE, \
            End, \
    End

#define List2(ftxt, fixedHeight) \
    ListviewObject, \
        MUIA_Weight, 150, \
        MUIA_Listview_Input, FALSE, \
        MUIA_FixHeight, fixedHeight, \
        MUIA_Listview_List, \
            FloattextObject, \
                MUIA_Frame, MUIV_Frame_ReadList, \
                MUIA_Background, MUII_ReadListBack, \
                MUIA_Floattext_Text, ftxt, \
                MUIA_Floattext_TabSize, 4, \
                MUIA_Floattext_Justify, TRUE, \
            End, \
    End

/* clang-format on */

#endif