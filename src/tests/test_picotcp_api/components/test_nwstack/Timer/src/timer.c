/* @TAG(DATA61_BSD) */
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
void irq_handle(void) {
    int error;

    /* TASK 4: call the platsupport library to handle the interrupt. */
    /* hint: ttc_handle_irq
     */

    ttc_handle_irq(&timer_drv);


    /* TASK 5: Stop the timer. */
    /* hint: ttc_stop
     */

    ttc_stop(&timer_drv);


    /* Signal the RPC interface. */
    error = sem_post();
    ZF_LOGF_IF(error != 0, "Failed to post to semaphore");

    /* TASK 6: acknowledge the interrupt */
    /* hint 1: use the function <IRQ interface name>_acknowledge()
     */

    error = irq_acknowledge();
    ZF_LOGF_IF(error != 0, "Failed to acknowledge interrupt");


}

void hello__init() {
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

    /* TASK 7: call platsupport library to get the timer handler */
    /* hint1: ttc_init
     */

    int error = ttc_init(&timer_drv, config);
    assert(error == 0);


    /* TASK 9: Start the timer
     * hint: ttc_start
     */

   // ttc_start(&timer_drv);



}

/* TASK 7: implement the RPC function. */
/* hint 1: the name of the function to implement is a composition of an interface name and a function name:
 * i.e.: <interface>_<function>
 * hint 2: the interfaces available are defined by the component, e.g. in components/Timer/Timer.camkes
 * hint 3: the function name is defined by the interface definition, e.g. in interfaces/timer.camkes
 * hint 4: so the function would be: hello_sleep()
 * hint 5: the CAmkES 'int' type maps to 'int' in C
 * hint 6: call platsupport library function to set up the timer
 * hint 7: look at https://github.com/seL4/camkes-tool/blob/master/docs/index.md#creating-an-application
 */
void hello_sleep(int msec) {
    /* TASK 8: call platsupport library function to set up the timer */
    /* hint: ttc_set_timeout
     */

    ttc_set_timeout(&timer_drv, msec * NS_IN_MSEC, false);

    ttc_start(&timer_drv);


    int error = sem_wait();
    ZF_LOGF_IF(error != 0, "Failed to wait on semaphore");
}
