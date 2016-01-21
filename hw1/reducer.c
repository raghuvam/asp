

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

struct pairs
{
char key[100];
int value;
struct pairs *next;
};

char curr_c = '0';
struct pairs* head;

int print_contents()
{
    struct pairs* temp = head;
    struct pairs* prev;
    while(temp != NULL)
    {
        printf("(%s,%d) \n",temp->key,temp->value);
        prev = temp;
        temp = temp->next;
        free(prev);
    } 
    head =NULL;
    return 0;
}

int word_handle(char nword[]){
    if(curr_c == '0')
        curr_c = nword[0];

    else
    {
      if(curr_c != nword[0])
      {
        curr_c = nword[0];
        //printf("Now curr_c: %c \n", curr_c);
        //printf("Printing contents \n");
        print_contents(head);
      }
    }
    
    bool isOld = false;
    
    if (head == NULL)
    {
        //printf("new series of %c \n", curr_c);
        head = (struct pairs*)malloc(sizeof(struct pairs));
        strcpy(head->key,nword);
        head->value =1;
        head->next =NULL;
        return 2;
    }

    struct pairs* temp = head;
    
    while(temp != NULL)
    {
        if(strcmp(temp->key, nword) == 0)
        {
        temp->value++;
        isOld =true;
        return 1;
        }
        temp = temp->next;
     }
     
     if(isOld == false)
     {
        struct pairs* new = (struct pairs*)malloc(sizeof(struct pairs));
        strcpy(new->key,nword);
        new->value=1;
        new->next =head;
        head =new;
      }
      return 0;  
}

int main()
{
    
    char line[100];
    FILE * fh;
    size_t st;
    fh = fopen("./output.txt","w+");
    //printf("EOF: ",EOF);
    while(1)
    {   
        if(scanf("%s",line) > 0)
        {   
            word_handle(line);

            }
        else
        {
            fclose(fh);
            break;
            }
    }
    
return 0;
}
