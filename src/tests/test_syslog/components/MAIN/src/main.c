#include "LibDebug/Debug.h"

#include <stdio.h>
#include <camkes.h>

#include "CamkesLogger.h"
#include "LibLogs/SysLog.h"

int run()
{
    FifoLogger stdOutLogger;

    if (!LogFifo_ctor(&logFifoDataPort->logFifo,
                      logFifoDataPort->logFifoBuff,
                      (PAGE_SIZE - sizeof(logFifoDataPort->logFifo))
                      / sizeof(LogFifo_Element)))
    {
        return -1;
    }
    if (!CamkesLogger_ctor(&stdOutLogger,
                           &logFifoDataPort->logFifo,
                           Log_MASK_ALL))
    {
        return -1;
    }
    if (!SysLog_ctor()
        || !SysLog_app(FifoLogger_TO_LOGGER(&stdOutLogger)))
    {
        return -1;
    }
    log_info("SUCCESS");

    return 0;
}
