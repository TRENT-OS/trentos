/*
 *  SEOS Network Stack CAmkES App for timer client
 *
 *  Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */
#include <stdio.h>

#include <platsupport/plat/timer.h>
#include <sel4utils/sel4_zf_logif.h>

#include <camkes.h>

#define NS_IN_SECOND 1000000000ULL
#define NS_IN_MSEC   1000000ULL

ttc_t timer_drv;

/* this callback handler is meant to be invoked when the first interrupt
 * arrives on the interrupt event interface.
 * Note: the callback handler must be explicitly registered before the
 * callback will be invoked.
 * Also the registration is one-shot only, if it wants to be invoked
 * when a new interrupt arrives then it must re-register itself.  Or it can
 * also register a different handler.
 */
void irq_handle(void)
{
    int error;


    ttc_handle_irq(&timer_drv);



    ttc_stop(&timer_drv);


    /* Signal the RPC interface. */
    error = sem_post();
    ZF_LOGF_IF(error != 0, "Failed to post to semaphore");

    error = irq_acknowledge();
    ZF_LOGF_IF(error != 0, "Failed to acknowledge interrupt");


}

void timeout__init()
{
    /* Structure of the timer configuration in platsupport library */
    ttc_config_t config;

    /*
     * Provide hardware info to platsupport.
     * Note, The following only works for zynq7000 platform. Other platforms may
     * require other info. Check the definition of timer_config_t and manuals.
     */
    config.vaddr = (void*)reg;
    config.clk_src = 0;
    config.id = TMR_DEFAULT;


    int error = ttc_init(&timer_drv, config);
    assert(error == 0);

}

void timeout_sleep(int msec)
{

    ttc_set_timeout(&timer_drv, msec * NS_IN_MSEC, false);

    ttc_start(&timer_drv);


    int error = sem_wait();
    ZF_LOGF_IF(error != 0, "Failed to wait on semaphore");
}
