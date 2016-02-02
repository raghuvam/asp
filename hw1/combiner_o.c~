#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char* argv[])
{
		if(argc != 2)
		{
			perror("Give Input filename as argument");
    		exit(0);
		}
        int     fd[2];
        pid_t   childpid;
        pid_t   childpid_2;
        pipe(fd);
        
        if((childpid = fork()) == -1)
        {
                perror("fork");
                exit(1);
        }

        if(childpid == 0)
        {
        		//reducer child process
        			close(fd[1]);	//write end close
                	dup2(fd[0],0);	// dup read end to stdin	
                	execl("./reducer","reducer",(char *)0);	// start execution of reducer
                	printf("exiting child 1\n");
                	close(fd[0]); // close the read end of the pipe on reducer side
                
        }
        else
        {
        		if((childpid_2 = fork()) == -1)
        		{
                	perror("fork");
                	exit(1);
        		}
        		
        		if(childpid_2 == 0)
        		{	
        		//mapper child process
                	close(fd[0]); // close read end
                	dup2(fd[1],1);	// dup write end
                	execl("./mapper","mapper",argv[1],(char *)0); // execute mapper 
                	close(fd[1]); // close write end
                	printf("exiting 1");
                
        		}
        		
        		else
        		{
        		close(fd[0]);
        		close(fd[1]);
        		int status;
        		waitpid(childpid,&status,0); // wait till reducer finishes
        		printf("exiting combiner\n");	
        		
        		}
                
        }
        
        return(0);
}
