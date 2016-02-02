#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) 
{
	if(argc!=2)
	{
		perror("Input format: <./combiner> <mapper_input_file_name>\n");
		return 0;
	}

    int fd[2];
    pipe(fd);

	pid_t child_a,child_b;

	// Start reducer using fork
	child_a = fork();

	if (child_a == 0) //Inside reducer
	{	
		close(fd[1]);   //close write end of pipe
        int ret = dup2(fd[0],0);	//redirect read end to stdin
        if (ret < 0) perror("dup2");   //print error if dup fails
		char *const args[] = { "reducer", NULL }; 
		int r = execv("./reducer", args);	//exec to make reducer programmer run
		if (r==-1)
		{
			perror("execv");	

		}
		close(fd[0]);  //close read end of pipe 
	}

	if( child_a>0)  //inside parent
	{
		//Start mapper using fork
		child_b = fork();
		if (child_b == 0)   //inside mapper
		{	
			close(fd[0]);	//close read end of pipe
		    int ret = dup2(fd[1],1);	//redirect write end to stdout
		    if (ret < 0) perror("dup2");
			char *const args[] = { "mapper", argv[1], NULL };
			int r = execv("./mapper", args);	//exec to make mapper programmer run
			if (r==-1) perror("execv");	
			close(fd[1]);	//close write end of pipe
		}
	

		// close parent's pipes
		close(fd[0]);
		close(fd[1]);

		// wait for the reducer child processes to finish
		int status;
		waitpid(child_a,&status,0);
		printf("Done!!! :)\n");
		return 0;
	
	}


}

