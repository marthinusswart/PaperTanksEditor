#ifndef IMAGEWIDGETS_H
#define IMAGEWIDGETS_H

/* clang-format off */
#define Image(nr) ImageObject, \
    MUIA_Image_Spec, nr, \
End

#define ScaledImage(imgData,width,height) ImageObject, \
    MUIA_Image_Spec, imgData, \
    MUIA_FixWidth, width, \
    MUIA_FixHeight, height, \
End

#define ScaledCustomImage(imgData,width,height) NewObject(NULL, MUIC_Area, \
    MUIA_FixWidth, width, \
    MUIA_FixHeight, height, \
    MUIA_UserData, imgData, \
    MUIA_InputMode, MUIV_InputMode_None, \
    TAG_DONE)
/* clang-format on */

#endif
