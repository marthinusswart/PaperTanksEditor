#ifndef COLORPALETTEVIEW_H
#define COLORPALETTEVIEW_H

#include <proto/gadtools.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>
#include <proto/icon.h>
#include <clib/alib_protos.h>

#include "viewtypes.h"
#include "../widgets/listwidgets.h"
#include "../widgets/imagewidgets.h"
#include "../widgets/pteimagepanel.h"
#include "../widgets/ptecolorpalettepanel.h"
#include "../graphics/png/imgpngutils.h"
#include "../utils/windowlogger.h"
#include "../utils/filelogger.h"
#include "../graphics/graphics.h"

extern void createColorPaletteView(Object *app);

#endif