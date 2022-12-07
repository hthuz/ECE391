#include "signal.h"
#include "syscall.h"
#include "lib.h"
#include "terminal.h"

sighandler_t default_handle[5];


// -------------------------------initilize the process--------------------------//
int32_t init_process_signal( pcb_t* the_pcb){
    int i;
    if (the_pcb==NULL)
        return -1;
    for( i=0;i<=4;i++){
        (the_pcb->the_signal).blocked[i] = 0;
        // the_pcb->the_signal->pre_blocked[i] =0;
        (the_pcb->the_signal).sigpending[i] =0;
        (the_pcb->the_signal).sighand[i] = default_handle[i];
    }
    (the_pcb->the_signal).alarm_time=0;
    return 0;
}


// -------------------------------default function--------------------------//
void init_default(){
    default_handle[0] = divzero_handler;
    default_handle[1] = segfault_handler;
    default_handle[2] = interrupt_handler;
    default_handle[3] = alarm_handler;
    default_handle[4] = user1_handler;
}

// For most kinds of signals, the default action is to terminate the process
    // 256 if the program dies by an exception
int32_t divzero_handler(){
    int32_t fl;
    cli_and_save(fl);
    halt(256);
    restore_flags(fl);
    return -1;
}
int32_t segfault_handler(){
    int32_t fl;
    cli_and_save(fl);
    halt(256);
    restore_flags(fl);
    return -1;
}
    // or a value in the range 0 to 255 if the program executes a halt system call
int32_t interrupt_handler(){
    int32_t fl;
    cli_and_save(fl);
    halt(1);
    restore_flags(fl);
    return -1;
}
//  For certain kinds of signals that represent “harmless” events, 
//  the default action is to do nothing.
int32_t alarm_handler(){
    return 0;
}
int32_t user1_handler(){
    return 0;
}

// ----------------------------specify the action of signal, to replace default for cur_process--------------------------//
int32_t set_signal(int signum, sighandler_t action){
    int32_t fl;
    // termin_t showing_terminal = terminals[cur_tid];
    // int32_t showing_pid = showing_terminal.pid;
    pcb_t *cur_pcb = get_pcb(cur_pid);
    if (signum<0 || signum>4 || action==NULL)
        return -1;
    // sighandler_t pre_action = cur_pcb->the_signal->sighand[signum];
    cli_and_save(fl);
    if (action!=NULL)  (cur_pcb->the_signal).sighand[signum] = action;
    // if (action==NULL)  cur_pcb->the_signal->sighand[signum] = default_handle[signum];
    restore_flags(fl);
    return 0;
}


// ----------------------------deliver signal to process--------------------------//
int32_t send_signal( uint32_t signum ){
    int32_t flag;

    if (signum<0 || signum>4){
        printf("send_signal wrong");
        return -1;
    }
    cli_and_save(flag);
    if (signum<=3 && signum>=2){
        termin_t showing_terminal = terminals[cur_tid];
        int32_t showing_pid = showing_terminal.pid;


        pcb_t *showing_pcb = get_pcb(showing_pid);
        (showing_pcb->the_signal).sigpending[signum]=1;

        termin_t* running_term = get_terminal(running_tid);
        running_term->enter_pressed = 1;

        printf("pcb sigpending has been changed\n");
    }
    else {
        pcb_t *run_pcb = get_pcb(cur_pid);
        (run_pcb->the_signal).sigpending[signum]=1;
    }
    
    restore_flags(flag);
    return 0;
}


// -------------after deliver the signal, mask other signals--------------//
int32_t mask_signal(int32_t signum){
    int32_t flag;
    // termin_t showing_terminal = terminals[cur_tid];
    // int32_t showing_pid = showing_terminal.pid;
    if (signum<0 || signum>4)
        return -1;
    cli_and_save(flag);
    pcb_t *cur_pcb = get_pcb(cur_pid);
    (cur_pcb-> the_signal). blocked [signum] = 1; 
    restore_flags(flag);
    return 0;
}


int32_t unmask_signal(int32_t signum){
    int32_t flag;
    // termin_t showing_terminal = terminals[cur_tid];
    // int32_t showing_pid = showing_terminal.pid;
    if (signum<0 || signum>4)
        return -1;
    cli_and_save(flag);

    pcb_t *cur_pcb = get_pcb(cur_pid);
    (cur_pcb-> the_signal). blocked [signum] = 0; 
    restore_flags(flag);
    return 0;
}



// ----------------when the signal is no longer "pending signal"--------------//

asmlinkage void tackle_signal(switch_para hard_context){
    // printf("start to tackle signal\n");
    int32_t fl;
    int32_t signum=0;
    int32_t i;
    // termin_t showing_terminal = terminals[cur_tid];
    // int32_t showing_pid = showing_terminal.pid;

    pcb_t *cur_pcb = get_pcb(cur_pid);
    if (hard_context.rcs != USER_CS)
        return;
    cli_and_save(fl);
    while ( 
            (
                (cur_pcb->the_signal).blocked[signum] == 1 || (cur_pcb->the_signal).sigpending[signum]==0
            )&& (signum<=4)     
          ) {
        signum++;
    }

    if (signum==5){
        restore_flags(fl);
        return;
    }
    // find the specific signum, deal with it
    for( i=0;i<=4;i++){
        (cur_pcb->the_signal).pre_blocked[i] = (cur_pcb->the_signal).blocked[i];
        (cur_pcb->the_signal).sigpending[i]=0;
        (cur_pcb->the_signal).blocked[i]=0;
    }

    sighandler_t hand_func = (cur_pcb->the_signal).sighand[signum];
    if (hand_func == default_handle[signum]){
        default_handle[signum]();
        restore_block();
    }
    else{
        // need to complete
    }

    restore_flags(fl);
}

// In any case, when the handler returns, the system restores 
// the mask that was in place before the handler was entered.
// If any signals that become unblocked by this restoration are pending, 
// the process will receive those signals immediately, 
// before returning to the code that was interrupted.



// ----------------restore the mask information--------------//
void restore_block(){
    int32_t i;
    int32_t flag;

    // termin_t showing_terminal = terminals[cur_tid];
    // int32_t showing_pid = showing_terminal.pid;

    cli_and_save(flag);
    pcb_t *cur_pcb = get_pcb(cur_pid);
    for( i=0;i<=4;i++){
        (cur_pcb->the_signal).blocked[i] = (cur_pcb->the_signal).pre_blocked[i];
    }
    restore_flags(flag);
}

