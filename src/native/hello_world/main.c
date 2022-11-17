//------------------------------------------------------------------------------
//
// seL4 native hello world application
//
// Copyright (C) 2019, HENSOLDT Cyber GmbH
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <sel4/sel4.h>

//------------------------------------------------------------------------------
int main(void)
{
    seL4_DebugNameThreadFmt(seL4_CapInitThreadTCB, "%s-%d", "root", seL4_CapInitThreadTCB);
    seL4_DebugDumpScheduler();

    // ToDo: fix test runner to flush logs to console when a line matched or
    //       the whole test has passed. Otherwise we may not see the output
    //       as the log monitor thread gets killed before it can print things.
    //       test has passed. Quick-hack is no print the magic line that makes
    //       the test pass.
    //
    // printf("Hello, world!\n");

    return 0;
}
