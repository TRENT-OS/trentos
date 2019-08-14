/*
 *  SEOS Network Stack CAmkES App for timer client
 *
 *  Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */

#include <stdio.h>

#include <camkes.h>

#define MSECS_TO_SLEEP   500

/* run the control thread */
int run(void)
{
    printf("Starting the client\n");
    printf("------Sleep for %d mseconds------\n", MSECS_TO_SLEEP);

    while (1)

    {
        toutinf_sleep(MSECS_TO_SLEEP);
        e_timeout_nwstacktick_emit();
        e_timeout_nwstacktick_2_emit();
    }

    return 0;
}
