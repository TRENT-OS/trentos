#ifndef LOGSPOOLER_CONFIG_H
#define LOGSPOOLER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "LogFifoDataPort_t.h"
#include <camkes.h>

#define LogSpooler_Config_NUM_FIFOS 1

static LogFifo*             logFifos[LogSpooler_Config_NUM_FIFOS];
static LogFifo_Element*     logFifoBuffs[LogSpooler_Config_NUM_FIFOS];

INLINE LogFifo_Element**
LogSpooler_Config_getLogFifoBuffs(void)
{
    logFifoBuffs[0] = (void*) logFifoDataPort->logFifoBuff;

    return logFifoBuffs;
}

INLINE LogFifo**
LogSpooler_Config_getLogFifos(void)
{
    logFifos[0] = &logFifoDataPort->logFifo;
    //loggers[1] = ...;

    return logFifos;
}

#ifdef __cplusplus
}
#endif

#endif /* LOGSPOOLER_CONFIG_H */

