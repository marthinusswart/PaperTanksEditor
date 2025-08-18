#ifndef WINDOWLOGGER_H
#define WINDOWLOGGER_H

#include <proto/muimaster.h>
#include <proto/intuition.h>
#include <libraries/mui.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    APTR loggerList; // List of log entries
} WindowLogger;

extern void windowLoggerInit(APTR list);
extern void windowLoggerAddEntry(const char *entry);
extern WindowLogger *windowLogger;

#endif