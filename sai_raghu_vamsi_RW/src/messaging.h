#ifndef _MESSAGING_H_
#define _MESSAGING_H_
    
    #ifdef __cplusplus
    extern "C" {
    #endif

    extern int init_messaging();
    extern int ninit_messaging();
    extern int send_message(char *msg);
    extern char *read_message();
    extern void exit_messaging();
    extern int print_buf_info();
    extern int get_msg_count();

   #ifdef __cplusplus
   }
   #endif
#endif