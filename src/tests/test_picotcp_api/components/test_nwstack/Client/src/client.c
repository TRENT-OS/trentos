/* @TAG(DATA61_BSD) */
/*
 * CAmkES tutorial part 1: components with RPC. Client part.
 */

#include <stdio.h>

/* generated header for our component */
#include <camkes.h>

#define MSECS_TO_SLEEP   500

/* run the control thread */
int run(void) {
    printf("Starting the client\n");
    printf("------Sleep for %d mseconds------\n", MSECS_TO_SLEEP);

    while(1)
    /* invoke the RPC function */
    {
        hello_sleep(MSECS_TO_SLEEP);
        e_timeout_nwstacktick_emit();
 //       e_timeout_nwstacktick_2_emit();
    }

     return 0;
}
