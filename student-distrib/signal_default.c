#include "signal.h"
#include "syscall.h"
#include "lib.h"
#include "terminal.h"

sighandler_t default_handle[5];
// -------------------------------default function--------------------------//
/*
 * init_default
 * DESCRIPTION: init_default
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: none
 */
void init_default()
{
    default_handle[0] = divzero_handler;
    default_handle[1] = segfault_handler;
    default_handle[2] = interrupt_handler;
    default_handle[3] = alarm_handler;
    default_handle[4] = user1_handler;
}

// For most kinds of signals, the default action is to terminate the process
// 256 if the program dies by an exception
/*
 * divzero_handler
 * DESCRIPTION: halt(256) because it is exception
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: none
 */
int32_t divzero_handler()
{
    int32_t fl;
    cli_and_save(fl);
    halt(256);
    restore_flags(fl);
    return -1;
}
/*
 * segfault_handler
 * DESCRIPTION: halt(256) because it is exception
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: none
 */
int32_t segfault_handler()
{
    int32_t fl;
    cli_and_save(fl);
    halt(256);
    restore_flags(fl);
    return -1;
}
/*
 * segfault_handler
 * DESCRIPTION: halt(1) because it is abnormally exit
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: none
 */
int32_t interrupt_handler()
{
    int32_t fl;
    cli_and_save(fl);
    halt(1);
    restore_flags(fl);
    return -1;
}
//  For certain kinds of signals that represent “harmless” events,
//  the default action is to do nothing.
/*
 * alarm_handler
 * DESCRIPTION: print alarm reach
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: 0
 * SIDE EFFECT: none
 */
int32_t alarm_handler()
{
    return 0;
}

/*
 * user1_handler
 * DESCRIPTION: none
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: 0
 * SIDE EFFECT: none
 */
int32_t user1_handler()
{
    return 0;
}
