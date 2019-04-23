#pragma once

#include "LibLogs/LogFifo.h"

typedef struct
{
    LogFifo             logFifo;
    LogFifo_Element     logFifoBuff[];
}
LogFifoDataPort_t;
