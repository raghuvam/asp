#include <stdio.h>
#include "messaging.h"
#include <stdlib.h>

int main()
{	
	char opt;
	char *msg;
	//init_messaging();
	while(1)
	{
		int val = get_msg_count();
		printf("Num of messages left %d\n", val);
		printf("Do you want to read a msg: ");
		scanf("%c",&opt);
		if(opt == 'y')
		{
			msg = read_message();
			printf("%s\n",msg);
		}

		sleep(0.5);
			
	}

	return 0;
}