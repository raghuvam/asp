#include <stdio.h>
#include "messaging.h"
#include <stdlib.h>
int main()
{
	init_messaging();	
	while(1)
	{
		char msg[100];
		printf("Write a new message: \n");
		//scanf("%[^\n]",msg);
		getline(msg,NULL,stdin);
		printf("sending message\n");
		send_message(msg);
		printf("sent message\n");

		sleep(0.5);

	}

	return 0;
}