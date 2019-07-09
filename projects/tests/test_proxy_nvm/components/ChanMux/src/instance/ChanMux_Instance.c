/*
 *  WAN/LAN/NetworkStack Channel MUX
 *
 *  Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#include "ChanMux/ChanMux.h"
#include "ChanMux_config.h"
#include "seos/seos_err.h"
#include "assert.h"
#include <camkes.h>

static uint8_t proxyNVM_Tester1FifoBuf[PAGE_SIZE];
static uint8_t proxyNVM_Tester2FifoBuf[PAGE_SIZE];

static const ChanMuxConfig_t cfgChanMux =
{
    .numChannels = CHANMUX_NUM_CHANNELS,
    .outputDataport = {
        .io  = (void**) &outputDataPort,
        .len = PAGE_SIZE
    },
    .channelsFifos = {
        {
            // Channel 0
            .buffer = NULL,
            .len = 0
        },
        {
            // Channel 1
            .buffer = NULL,
            .len = 0
        },
        {
            // Channel 2
            .buffer = NULL,
            .len = 0
        },
        {
            // Channel 3
            .buffer = NULL,
            .len = 0
        },
        {
            // Channel 4
            .buffer = NULL,
            .len = 0
        },
        {
            // Channel 5
            .buffer = NULL,
            .len = 0
        },
        {
            // Channel 6
            .buffer = proxyNVM_Tester1FifoBuf,
            .len = sizeof(proxyNVM_Tester1FifoBuf)
        },
        {
            // Channel 7
            .buffer = proxyNVM_Tester2FifoBuf,
            .len = sizeof(proxyNVM_Tester2FifoBuf)
        }
    }
};

const ChannelDataport_t dataports[] =
{
    {
        .io  = NULL,
        .len = 0
    },
    {
        .io  = NULL,
        .len = 0
    },
    {
        .io  = NULL,
        .len = 0
    },
    {
        .io  = NULL,
        .len = 0
    },
    {
        .io  = NULL,
        .len = 0
    },
    {
        .io  = NULL,
        .len = 0
    },
    {
        .io  = (void**) &proxyNVM_Tester1DataPort,
        .len = PAGE_SIZE
    },
    {
        .io  = (void**) &proxyNVM_Tester2DataPort,
        .len = PAGE_SIZE
    }
};

//------------------------------------------------------------------------------
const ChanMuxConfig_t*
ChanMux_config_getConfig(void)
{
    return &cfgChanMux;
}

//------------------------------------------------------------------------------
void
ChanMux_dataAvailable_emit(unsigned int chanNum)
{
    Debug_LOG_TRACE("%s: chan %u",
                    __func__, chanNum);
    switch (chanNum)
    {
<<<<<<< HEAD
    case CHANNEL_MAIN_DATA:
        dataAvailableMain_emit();
        break;

    default:
        Debug_LOG_ERROR("%s(): invalid channel %u", __func__, chanNum);
        break;
=======
    case CHANNEL_NVM_USER1_DATA:
    case CHANNEL_NVM_USER2_DATA:
        dataAvailableMain_emit();
        break;
    default:
        Debug_LOG_ERROR("%s(): invalid channel %u", __func__, chanNum);
        break;
>>>>>>> integration
    }
}

//------------------------------------------------------------------------------
static ChanMux*
ChanMux_getInstance(void)
{
    // singleton
    static ChanMux  theOne;
    static ChanMux* self = NULL;
    static Channel_t channels[CHANMUX_NUM_CHANNELS];

    static const ChanMux_MuxInf muxinf =
    {
        .lock = Mutex_lock,
        .unlock = Mutex_unlock,
    };

    if ((NULL == self) && ChanMux_ctor(&theOne,
                                       channels,
                                       ChanMux_config_getConfig(),
<<<<<<< HEAD
                                       NULL,
=======
                                       &muxinf,
>>>>>>> integration
                                       ChanMux_dataAvailable_emit,
                                       Output_write))
    {
        self = &theOne;
    }

    return self;
}

void
ChanMuxOut_takeByte(char byte)
{
    ChanMux_takeByte(ChanMux_getInstance(), byte);
}

//==============================================================================
// CAmkES Interface
//==============================================================================

//------------------------------------------------------------------------------
seos_err_t
ChanMuxIn_write(
    unsigned int  chanNum,
    size_t        len,
    size_t*       lenWritten)
{
    Debug_LOG_TRACE("%s(): channel %u, len %u", __func__, chanNum, len);

    // set defaults to error
    *lenWritten = 0;

    const ChannelDataport_t* dp = NULL;
    switch (chanNum)
    {
    //---------------------------------
<<<<<<< HEAD
    case CHANNEL_MAIN_DATA:
=======
    case CHANNEL_NVM_USER1_DATA:
    case CHANNEL_NVM_USER2_DATA:
>>>>>>> integration
        dp = &dataports[chanNum];
        break;
    //---------------------------------
    default:
        Debug_LOG_ERROR("%s(): invalid channel %u", __func__, chanNum);
        return SEOS_ERROR_ACCESS_DENIED;
    }

    Debug_ASSERT( NULL != dp );

    seos_err_t ret = ChanMux_write(ChanMux_getInstance(), chanNum, dp, &len);

    *lenWritten = len;

    Debug_LOG_TRACE("%s(): channel %u, lenWritten %u", __func__, chanNum, len);

    return ret;
}


//------------------------------------------------------------------------------
seos_err_t
ChanMuxIn_read(
    unsigned int  chanNum,
    size_t        len,
    size_t*       lenRead)
{
    Debug_LOG_TRACE("%s(): channel %u, len %u", __func__, chanNum, len);

    // set defaults to error
    *lenRead = 0;

    const ChannelDataport_t* dp = NULL;
    switch (chanNum)
    {
<<<<<<< HEAD
    //---------------------------------
    case CHANNEL_MAIN_DATA:
=======

    //---------------------------------
    case CHANNEL_NVM_USER1_DATA:
    case CHANNEL_NVM_USER2_DATA:
>>>>>>> integration
        dp = &dataports[chanNum];
        break;
    //---------------------------------
    default:
        Debug_LOG_ERROR("%s(): invalid channel %u", __func__, chanNum);
        return SEOS_ERROR_ACCESS_DENIED;
    }


    Debug_ASSERT( NULL != dp );
    seos_err_t ret = ChanMux_read(ChanMux_getInstance(), chanNum, dp, &len);
    *lenRead = len;

    Debug_LOG_TRACE("%s(): channel %u, lenRead %u", __func__, chanNum, len);

    return ret;
}
