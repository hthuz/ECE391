#include "signal.h"
#include "syscall.h"
#include "lib.h"
#include "terminal.h"

int interrupt_shell_flag[3] = {0};
sighandler_t default_handle[MAX_SIGNUM+1];

// -------------------------------initilize the process--------------------------//
/*
 * init_process_signal
 * DESCRIPTION: init the variable in the specific process
 * INPUTS: pcb_t *the_pcb
 * OUTPUTS: none
 * RETURN VALUE: 0
 * SIDE EFFECT: none
 */
int32_t init_process_signal(pcb_t *the_pcb)
{
    int i;
    if (the_pcb == NULL)
        return -1;
    for (i = 0; i <= MAX_SIGNUM; i++)
    {
        (the_pcb->the_signal).blocked[i] = 0;
        // the_pcb->the_signal->pre_blocked[i] =0;
        (the_pcb->the_signal).sigpending[i] = 0;
        (the_pcb->the_signal).sighand[i] = default_handle[i];
    }
    (the_pcb->the_signal).alarm_time = 0;

    return 0;
}

// ----------------------------specify the action of signal, to replace default for cur_process--------------------------//

/*
 * set_signal
 * DESCRIPTION: init the variable in the specific process
 * INPUTS: int signum: the # of signal
 *         sighandler_t action: the function pointer in user space
 * OUTPUTS: none
 * RETURN VALUE: 0
 * SIDE EFFECT: none
 */
int32_t set_signal(int signum, sighandler_t action)
{
    int32_t fl;
    if (signum < 0 || signum > MAX_SIGNUM || action == NULL)
        return -1;
    cli_and_save(fl);
    if (action != NULL){
        pcb_t *cur_pcb = get_pcb(cur_pid);
        (cur_pcb->the_signal).sighand[signum] = action;
    }
    restore_flags(fl);
    return 0;
}

// ----------------------------deliver signal to process--------------------------//

/*
 * send_signal
 * DESCRIPTION: change the process struct value of the_signal
 * INPUTS: uint32_t signum: the # of signal
 * OUTPUTS: none
 * RETURN VALUE: 0
 * SIDE EFFECT: none
 */
int32_t send_signal(uint32_t signum)
{
    int32_t flag;

    if (signum < 0 || signum > MAX_SIGNUM)
    {
        printf("send_signal wrong");
        return -1;
    }
    cli_and_save(flag);
    if (signum <= 3 && signum >= 2)
    {
        termin_t showing_terminal = terminals[cur_tid];
        int32_t showing_pid = showing_terminal.pid;
        pcb_t *showing_pcb = get_pcb(showing_pid);
        (showing_pcb->the_signal).sigpending[signum] = 1;


        // If User Interrupt and send to shell program, stop shell reading
        if(signum == 2 && showing_terminal.task_is_shell == 1)
            interrupt_shell_flag[cur_tid] = 1;

    }
    else
    {
        pcb_t *run_pcb = get_pcb(cur_pid);
        (run_pcb->the_signal).sigpending[signum] = 1;
    }

    restore_flags(flag);
    return 0;
}


// ----------------when the signal is no longer "pending signal"--------------//
// -------------after deliver the signal, mask other signals--------------//


/*
 * tackle_signal
 * DESCRIPTION: takle signal, if it is in the default, execute it
 *              if it is in user space, use the assembly code in signal_asm.S
 * INPUTS: switch_para hard_context
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: none
 */
void tackle_signal(switch_para hard_context)
{
    // printf("start to tackle signal\n");
    int32_t fl;
    int32_t signum = 0;
    int32_t i;
    // termin_t showing_terminal = terminals[cur_tid];
    // int32_t showing_pid = showing_terminal.pid;

    pcb_t *cur_pcb = get_pcb(cur_pid);
    if (hard_context.rcs != USER_CS)
        return;
    cli_and_save(fl);
    while (
        (
            (cur_pcb->the_signal).blocked[signum] == 1 || (cur_pcb->the_signal).sigpending[signum] == 0) &&
        (signum <= MAX_SIGNUM))
    {
        signum++;
    }

    if (signum == MAX_SIGNUM+1)
    {
        restore_flags(fl);
        return;
    }
    // mask signal and record them in pre_blocked
    for (i = 0; i <= MAX_SIGNUM; i++)
    {
        (cur_pcb->the_signal).pre_blocked[i] = (cur_pcb->the_signal).blocked[i];
        (cur_pcb->the_signal).sigpending[i] = 0;
        (cur_pcb->the_signal).blocked[i] = 0;
    }

    sighandler_t hand_func = (cur_pcb->the_signal).sighand[signum];
    if (hand_func == default_handle[signum]){
        default_handle[signum]();
        restore_block();
    }
    else{
        user_mode_handler(signum,hand_func,&hard_context);
        // restore_block in sigreturn, thank you!
    }

    restore_flags(fl);
}

// In any case, when the handler returns, the system restores
// the mask that was in place before the handler was entered.
// If any signals that become unblocked by this restoration are pending,
// the process will receive those signals immediately,
// before returning to the code that was interrupted.

// ----------------restore the mask information--------------//

/*
 * restore_block
 * DESCRIPTION: retore the mask information
 * INPUTS: switch_para hard_context
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: none
 */
void restore_block()
{
    int32_t i;
    int32_t flag;

    cli_and_save(flag);
    pcb_t *cur_pcb = get_pcb(cur_pid);
    for (i = 0; i <= MAX_SIGNUM; i++)
    {
        (cur_pcb->the_signal).blocked[i] = (cur_pcb->the_signal).pre_blocked[i];
    }
    restore_flags(flag);
}
