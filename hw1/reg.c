
#include<stdio.h>
#include<stdlib.h>


int main(int argc, char* argv[])
{
    if(argc != 2)
    {
    	perror("Give Input filename as argument");
    	exit(0);
    }
    
    char line[100];
    FILE * fh;
    fh = fopen(argv[1],"r");
    int i = 0;
    while(!feof(fh))
    {
    	i++;
        fscanf(fh,"(%[^,]%*[^)])\n",line);
        printf("handling %s\n",line);
    }
    
return 0;
}
