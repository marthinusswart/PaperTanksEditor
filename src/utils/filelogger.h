#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <exec/memory.h>
#include <stdarg.h>
#include <stdio.h>

typedef struct
{
    BPTR logFile;          // File handle for the log file
    char logFilePath[256]; // Full path to the log file
    BOOL isInitialized;    // Flag to track initialization status
    BOOL isDebug;          // Flag to control debug message logging
} FileLogger;

extern void fileLoggerInit(const char *filename);
extern void fileLoggerAddEntry(const char *entry);
extern void fileLoggerAddDebugEntry(const char *entry);
extern void fileLoggerAddErrorEntry(const char *entry);
extern void fileLoggerSetDebug(BOOL enableDebug);
extern void fileLoggerClose(void);
extern BOOL loggerFormatMessage(char *outBuf, const char *format, ...);
extern FileLogger *fileLogger;

#endif