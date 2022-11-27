// #include "signal.h"
// #include "syscall.h"

// // int32_t* signal_handler[5];

// void kill(uint32_t signal_num){
//     halt(signal_num);
// }

// void ignore(uint32_t signal_num){
// }

// int32_t send_signal(uint32_t signal_num ){
//     if (signal_num<0 || signal_num>4)
//         return -1;
//     pcb_t *cur_pcb = get_pcb(cur_pid);
//     cur_pcb->signal_array[signal_num]=1;
//     return 0;
// }

// int32_t handle_signal(int32_t pid){
//     int32_t i;
//     pcb_t *cur_pcb = get_pcb(cur_pid);
//     for (i=0;i<=2;i++){
//         if (cur_pcb->signal_array[i]!=0) kill(i);
//     }
//     for (i=3;i<=4;i++){
//         if (cur_pcb->signal_array[i]!=0) ignore(i);
//     }
// }


