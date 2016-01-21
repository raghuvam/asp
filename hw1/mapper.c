
#include<stdio.h>
#include<stdlib.h>

int main()
{
    
    char line[100];
    FILE * fh;
    fh = fopen("./input.txt","r");
    
    while(1)
    {
        fscanf(fh,"%s",line);
        if(!feof(fh))
            printf("%s\n",line);
        else
            break;
    }
    
return 0;
}
