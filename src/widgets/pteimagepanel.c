// Include your header
#include "pteimagepanel.h"

/***********************************************************************/

/* Prototypes */
extern struct Library *MUIMasterBase;

DISPATCHER(PTEImagePanelDispatcher);
IPTR SAVEDS mNew(struct IClass *cl, Object *obj, struct opSet *msg);

static ULONG STACKARGS DoSuperNew(struct IClass *const cl, Object *const obj, const ULONG tags, ...)
{
    return (DoSuperMethod(cl, obj, OM_NEW, &tags, NULL));
}

struct MUI_CustomClass *createPTEImagePanelClass(void)
{
    char logMessage[256];
    fileLoggerAddEntry("PTEImagePanel: Creating MUI_CreateCustomClass");
    loggerFormatMessage(logMessage, "Dispatcher address: 0x%08lx", PTEImagePanelDispatcher);
    fileLoggerAddEntry(logMessage);
    struct MUI_CustomClass *obj = MUI_CreateCustomClass(NULL, MUIC_Rectangle, NULL, sizeof(struct PTEImagePanelData), PTEImagePanelDispatcher);
    if (!obj)
    {
        fileLoggerAddEntry("PTEImagePanel: Failed to create custom class");
        return NULL;
    }
    else
    {
        fileLoggerAddEntry("PTEImagePanel: Custom class created successfully");
        loggerFormatMessage(logMessage, "Custom class obj address: 0x%08lx", (ULONG)obj);
        fileLoggerAddEntry(logMessage);
        return obj;
    }
}

/***********************************************************************/

/***********************************************************************/

IPTR SAVEDS mNew(struct IClass *cl, Object *obj, struct opSet *msg)
{

    fileLoggerAddEntry("PTEImagePanel: mNew called");

    /*ULONG result;*/
    struct TagItem *tags, *tag;

    obj = (Object *)DoSuperNew(cl, obj, TAG_MORE, msg->ops_AttrList);

    return (ULONG)obj;
}

/***********************************************************************/

/***********************************************************************/

// ULONG __saveds __asm PTEImagePanelDispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
DISPATCHER(PTEImagePanelDispatcher)
{
    char logMessage[256];
    fileLoggerAddEntry("PTEImagePanel: PTEImagePanelDispatcher called");

    if (!msg->MethodID)
    {
        fileLoggerAddEntry("PTEImagePanel: PTEImagePanelDispatcher called with NULL MethodID");
        loggerFormatMessage(logMessage, "PTEImagePanel: PTEImagePanelDispatcher called with MethodID: 0x%08lx", msg->MethodID);
        fileLoggerAddEntry(logMessage);
        loggerFormatMessage(logMessage, "msg pointer Address: 0x%08lx", (ULONG)msg);
        fileLoggerAddEntry(logMessage);
        loggerFormatMessage(logMessage, "PTEImagePanel: Obj Address: 0x%08lx", (ULONG)obj);
        fileLoggerAddEntry(logMessage);
        loggerFormatMessage(logMessage, "PTEImagePanel: Cl Address: 0x%08lx", (ULONG)cl);
        fileLoggerAddEntry(logMessage);
        return 0
    }

    // loggerFormatMessage(logMessage, "PTEImagePanel: PTEImagePanelDispatcher called with MethodID: 0x%08lx", msg->MethodID);
    // fileLoggerAddEntry(logMessage);
    // loggerFormatMessage(logMessage, "msg pointer: 0x%08lx", (ULONG)msg);
    // fileLoggerAddEntry(logMessage);

    switch (msg->MethodID)
    {
    case OM_NEW:
        return mNew(cl, obj, (APTR)msg);
        // case OM_DISPOSE:                    return mDispose(cl, obj, (APTR)msg);
        // case OM_GET:                        return mGet(cl, obj, (APTR)msg);
        // case OM_SET:                        return mSets(cl, obj, (APTR)msg);

    default:
        return DoSuperMethodA(cl, obj, msg);
    }
}

/**********************************************************************/
