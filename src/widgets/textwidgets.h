#ifndef TEXTWIDGETS_H
#define TEXTWIDGETS_H

/* clang-format off */

#define TxtInput(ftxt) \
    StringObject, \
        StringFrame, \
        MUIA_String_Contents, ftxt, \
        MUIA_String_MaxLen, 100, \
    End

/* clang-format on */

#endif // TEXTWIDGETS_H