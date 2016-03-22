#ifndef _MESSAGING_H_
#define _MESSAGING_H_

int init_messaging();
int send_message(char *msg);
char *read_message();
int get_msg_count();

#endif
