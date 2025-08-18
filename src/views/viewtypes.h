#ifndef VIEWTYPES_H
#define VIEWTYPES_H

/* clang-format off */

#define DefaultSubWindow(name, id, width, height) WindowObject, \
    MUIA_Window_Title, name, \
    MUIA_Window_ID, id, \
    MUIA_Window_Width, width, \
    MUIA_Window_Height, height, \
    WindowContents

/* clang-format on */

#endif