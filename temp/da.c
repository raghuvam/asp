

#include <stdio.h>
#include <malloc.h>

int main(int argc, char* argv[])
{

int n = atoi(argv[1]);
int *arr = malloc(n*sizeof(int))
printf("addr arr %x",arr);


int i =0;
for(i =0;i <n;i++)
{
    arr[i] = i;
} 
for(i =0;i <n;i++)
    printf("%d \n",arr[i]);   
    

return 0;
}
