#include "windowlogger.h"

WindowLogger *windowlogger;

void windowLoggerInit(APTR list)
{
    windowlogger = (WindowLogger *)malloc(sizeof(WindowLogger));
    if (windowlogger)
    {
        windowlogger->loggerList = list; // Assuming list is a pointer to a MUI List object
    }
    else
    {
        // Handle memory allocation failure
        printf("Failed to initialize logger: Memory allocation error\n");
    }
}

void windowLoggerAddEntry(const char *entry)
{
    if (windowlogger && windowlogger->loggerList)
    {
        // Assuming loggerList is a MUI List object
        DoMethod(windowlogger->loggerList, MUIM_List_InsertSingle, entry, MUIV_List_Insert_Bottom);
    }
    else
    {
        printf("Logger not initialized or list is NULL\n");
    }
}