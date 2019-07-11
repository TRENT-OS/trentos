/**
 *
 * @author Carmelo Pintaudi
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#include "CamkesLogger.h"
#include "LibLogs/Logger.h"
#include <camkes.h>

static void
enqueue(Logger* logger, Log* log)
{
    FifoLogger_enqueue(logger, log);
    logAvailable_emit(); // emits the signal that a new log is there
}

static const Logger_Iface vtable =
{
    .dtor       = FifoLogger_dtor,
    .enqueue    = enqueue,
    .dequeue    = FifoLogger_dequeue
};

bool
CamkesLogger_ctor(FifoLogger* self,
                  LogFifo* logFifo,
                  uint8_t mask)
{
    bool isOK = FifoLogger_ctor(self, logFifo, mask);

    if (isOK)
    {
        self->parent.vtable = &vtable;
    }
    return isOK;
}
