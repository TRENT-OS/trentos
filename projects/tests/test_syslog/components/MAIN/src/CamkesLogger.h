/**
 *
 * @author Carmelo Pintaudi
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#include "LibLogs/FifoLogger.h"

bool
CamkesLogger_ctor(FifoLogger* self,
                  LogFifo* logFifo,
                  uint8_t mask);
