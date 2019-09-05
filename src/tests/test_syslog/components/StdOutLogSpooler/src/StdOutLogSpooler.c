/**
 * @author Carmelo Pintaudi
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#include "LibLogs/Logger.h"
#include "instance/StdOutLogSpooler_Config.h"
#include "LibLogs/FifoLogger.h"

int
run(void)
{
    while (true)
    {
        logAvailable_wait();

        LogFifo**           logFifos     = LogSpooler_Config_getLogFifos();
        LogFifo_Element**   logFifoBuffs = LogSpooler_Config_getLogFifoBuffs();

        for (int i = 0; i < LogSpooler_Config_NUM_FIFOS; i++)
        {
            while (!LogFifo_isEmpty(logFifos[i]))
            {
                // Make copy of the Fifo Object, this fifo is single producer
                // single consumer thread safe, we need a copy object to get
                // the first element by assigning to the copy object the correct
                // buffer (an input dataport). Then the pop() method can be
                // called safely on the original object
                LogFifo logFifo = *logFifos[i];
                logFifo.fifo    = logFifoBuffs[i];
                LogFifo_Element const* el = LogFifo_getFirst(&logFifo);

                if (el != NULL && el->logString != NULL)
                {
                    // This spooler is a printf one. Other spoolers can rely
                    // on an Output_write() method. For exaple we can connect
                    // the spooler to the ChanMux
                    printf("%s\n", el->logString);
                }
                LogFifo_pop(logFifos[i]);
            }
        }
    }
    return -1;
}
/** @} */

