#include "filelogger.h"

FileLogger *fileLogger = NULL;

void fileLoggerInit(const char *filename)
{
    BPTR progDir = 0;
    BPTR oldDir = 0;
    struct Process *proc = NULL;
    char header[256];
    LONG i;

    // Clean up any existing logger
    if (fileLogger)
    {
        fileLoggerClose();
    }

    // Allocate memory for the logger using Amiga OS memory functions
    fileLogger = (FileLogger *)AllocMem(sizeof(FileLogger), MEMF_CLEAR);
    if (!fileLogger)
    {
        return;
    }

    // Initialize the structure
    fileLogger->logFile = 0;
    fileLogger->isInitialized = FALSE;
    fileLogger->isDebug = FALSE;

    // Clear the path buffer
    for (i = 0; i < 256; i++)
    {
        fileLogger->logFilePath[i] = '\0';
    }

    // Get the current process
    proc = (struct Process *)FindTask(NULL);
    if (!proc)
    {
        FreeMem(fileLogger, sizeof(FileLogger));
        fileLogger = NULL;
        return;
    }

    // Get PROGDIR: lock
    progDir = proc->pr_HomeDir;
    if (!progDir)
    {
        FreeMem(fileLogger, sizeof(FileLogger));
        fileLogger = NULL;
        return;
    }

    // Change to PROGDIR:
    oldDir = CurrentDir(progDir);

    // Copy the filename (simple string copy without strncpy)
    if (filename)
    {
        LONG len = 0;
        // Calculate length
        while (filename[len] && len < 255)
        {
            len++;
        }
        // Copy string
        for (i = 0; i < len && i < 255; i++)
        {
            fileLogger->logFilePath[i] = filename[i];
        }
        fileLogger->logFilePath[i] = '\0';
    }
    else
    {
        // Default filename
        const char *defaultName = "application.log";
        for (i = 0; defaultName[i] && i < 255; i++)
        {
            fileLogger->logFilePath[i] = defaultName[i];
        }
        fileLogger->logFilePath[i] = '\0';
    }

    // Try to open existing file for append
    fileLogger->logFile = Open(fileLogger->logFilePath, MODE_READWRITE);
    if (fileLogger->logFile)
    {
        // File exists, seek to end for append
        Seek(fileLogger->logFile, 0, OFFSET_END);
    }
    else
    {
        // File doesn't exist, create it
        fileLogger->logFile = Open(fileLogger->logFilePath, MODE_NEWFILE);
    }

    // Restore original directory
    CurrentDir(oldDir);

    if (!fileLogger->logFile)
    {
        FreeMem(fileLogger, sizeof(FileLogger));
        fileLogger = NULL;
        return;
    }

    // Mark as initialized
    fileLogger->isInitialized = TRUE;

    // Write session header (simple string building)
    const char *headerText = "\n=== LOG SESSION STARTED ===\n";
    for (i = 0; headerText[i] && i < 255; i++)
    {
        header[i] = headerText[i];
    }
    header[i] = '\0';

    FPuts(fileLogger->logFile, header);

    // Flush using available DOS function
    if (fileLogger->logFile)
    {
        // Force write by closing and reopening in append mode
        Close(fileLogger->logFile);
        fileLogger->logFile = Open(fileLogger->logFilePath, MODE_READWRITE);
        if (fileLogger->logFile)
        {
            Seek(fileLogger->logFile, 0, OFFSET_END);
        }
    }
}

void fileLoggerAddEntry(const char *entry)
{
    char logLine[512];
    struct DateStamp ds;
    ULONG minutes, seconds;
    LONG i, j;
    char timeStr[16];

    if (!fileLogger || !fileLogger->isInitialized || !fileLogger->logFile)
    {
        return;
    }

    if (!entry)
    {
        return;
    }

    // Get current date/time using basic DOS function
    DateStamp(&ds);

    // Convert ticks to minutes and seconds (simplified)
    // ds.ds_Tick contains 1/50th seconds since midnight
    // ds.ds_Minute contains minutes since midnight
    minutes = ds.ds_Minute % 60;
    seconds = (ds.ds_Tick / 50) % 60;

    // Build simple time string HH:MM:SS (simplified to MM:SS)
    timeStr[0] = '0' + ((ds.ds_Minute / 60) % 24) / 10; // Hours tens
    timeStr[1] = '0' + ((ds.ds_Minute / 60) % 24) % 10; // Hours ones
    timeStr[2] = ':';
    timeStr[3] = '0' + minutes / 10; // Minutes tens
    timeStr[4] = '0' + minutes % 10; // Minutes ones
    timeStr[5] = ':';
    timeStr[6] = '0' + seconds / 10; // Seconds tens
    timeStr[7] = '0' + seconds % 10; // Seconds ones
    timeStr[8] = '\0';

    // Build log line manually: [timestamp] message\n
    i = 0;
    logLine[i++] = '[';

    // Copy timestamp
    for (j = 0; timeStr[j] && i < 510; j++, i++)
    {
        logLine[i] = timeStr[j];
    }

    logLine[i++] = ']';
    logLine[i++] = ' ';

    // Copy message
    for (j = 0; entry[j] && i < 510; j++, i++)
    {
        logLine[i] = entry[j];
    }

    logLine[i++] = '\n';
    logLine[i] = '\0';

    // Write to file
    FPuts(fileLogger->logFile, logLine);

    // Force immediate write by closing and reopening
    Close(fileLogger->logFile);
    fileLogger->logFile = Open(fileLogger->logFilePath, MODE_READWRITE);
    if (fileLogger->logFile)
    {
        Seek(fileLogger->logFile, 0, OFFSET_END);
    }
}

void fileLoggerClose(void)
{
    if (fileLogger)
    {
        if (fileLogger->isInitialized && fileLogger->logFile)
        {
            // Write session footer
            FPuts(fileLogger->logFile, "=== LOG SESSION ENDED ===\n\n");

            Close(fileLogger->logFile);
        }

        // Free memory using Amiga OS function
        FreeMem(fileLogger, sizeof(FileLogger));
        fileLogger = NULL;
    }
}

void fileLoggerSetDebug(BOOL enableDebug)
{
    if (fileLogger && fileLogger->isInitialized)
    {
        fileLogger->isDebug = enableDebug;
        
        // Log the debug state change
        if (enableDebug)
        {
            fileLoggerAddEntry("Debug logging enabled");
        }
        else
        {
            fileLoggerAddEntry("Debug logging disabled");
        }
    }
}

BOOL loggerFormatMessage(char *outBuf, const char *format, ...)
{
    if (!outBuf || !format)
        return FALSE;

    va_list args;
    va_start(args, format);

    vsprintf(outBuf, format, args);

    va_end(args);
    return TRUE;
}

void fileLoggerAddDebugEntry(const char *entry)
{
    if (!fileLogger || !fileLogger->isInitialized)
    {
        return;
    }
    
    // Only log if debug mode is enabled
    if (fileLogger->isDebug)
    {
        char debugEntry[512];
        LONG i, j;
        
        // Add "DEBUG: " prefix to the message
        const char *prefix = "DEBUG: ";
        
        // Copy prefix
        for (i = 0; prefix[i] && i < 510; i++)
        {
            debugEntry[i] = prefix[i];
        }
        
        // Copy the actual message
        for (j = 0; entry[j] && (i + j) < 510; j++)
        {
            debugEntry[i + j] = entry[j];
        }
        
        debugEntry[i + j] = '\0';
        
        // Log using the standard entry function
        fileLoggerAddEntry(debugEntry);
    }
}
