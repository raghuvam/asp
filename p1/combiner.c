/* Combiner.c by Sai Raghu Vamsi Anumula */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char* argv[])
{
		if(argc != 2)
		{
			//check if input file is given or not
			perror("Give Input filename as argument \n Eg: $ ./combiner input.txt \n");
    		exit(0);
		}
        // file descriptor for pipe
        int     fd[2];
        
        pid_t   childpid; // pid of child 1
        pid_t   childpid_2; // pid of child 2
        
        // Create pipe send data from mapper to reducer
        pipe(fd);
        
        if((childpid = fork()) == -1)
        {	
        	//Exit if fork fails
            perror("fork");
            exit(1);
        }

        if(childpid == 0)
        {
        		//reducer child process
        			close(fd[1]);	//write end close
                	dup2(fd[0],0);	// dup read end to stdin	
                	execl("./reducer","reducer",(char *)0);	// start execution of reducer
                	close(fd[0]); // close the read end of the pipe on reducer side
                
        }
        else
        {		// Create 2nd Child process, which is mapper
        		if((childpid_2 = fork()) == -1)
        		{	// Exit if the fork() fails
                	perror("fork");
                	exit(1);
        		}
        		
        		if(childpid_2 == 0)
        		{	
        			//mapper child process
                	close(fd[0]); // close read end of the pipe
                	dup2(fd[1],1);	// dup write end of the pipe to stdout
                	execl("./mapper","mapper",argv[1],(char *)0); // execute mapper program
                	close(fd[1]); // close write end once mapper finishes
        		}
        		
        		else
        		{	
        			// Close the read end and write end for the Parent Process
        			close(fd[0]);
        			close(fd[1]);
        			// Intialize the status pointer for Parent Process
        			int status;
        			waitpid(childpid,&status,0); // wait till reducer finishes
        		}
                
        }
        
        return(0);
}
