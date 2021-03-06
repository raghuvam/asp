/* Mapper.c by Sai Raghu Vamsi Anumula */
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

/* pairs struct stores the word and count the pairs are stored 
as linked list, the next pointer points to the next struct */
struct pairs
{
char key[100];
int value;
struct pairs *next;
};

char curr_c = '0';
// head pointer to the head of the linked list
struct pairs* head;

/* Prints contents of the linked list i.e key value pair of 	
   previous characters once a word with new starting character is 
   read. Also frees the memory of by deleteing nodes corresponding 
   to previous character
*/
int print_contents()
{
    struct pairs* temp = head;
    struct pairs* prev;
    while(temp != NULL)
    {	//prints (key, count) pairs
        printf("(%s,%d) \n",temp->key,temp->value);
        prev = temp;
        temp = temp->next;
        //frees memory
        free(prev);
    }
    // Points head to NULL
    head =NULL;
    return 0;
}

int word_handle(char nword[]){
    
    if(curr_c == '0')
        curr_c = nword[0];

    else
    { //Checks if the word is new starting charater
      if(curr_c != nword[0])
      {
        curr_c = nword[0];
        //Print contents of the linked list
        print_contents();
      }
    }
    
    bool isOld = false;
    
    if (head == NULL)
    {	
    	// If a new word arrives the head points to NULL
    	// so a new node is created in the linked list and head
    	// points to that node
        head = (struct pairs*)malloc(sizeof(struct pairs));
        strcpy(head->key,nword);
        head->value =1;
        head->next =NULL;
        return 2;
    }

    struct pairs* temp = head;
    
    while(temp != NULL)
    {	
    	// Check if the word is already present in the linked list
        if(strcmp(temp->key, nword) == 0)
        {
        	//Increments the counter if the word is already present
	        temp->value++;
       	 	isOld =true;
        	return 1;
        }
        temp = temp->next;
     }
     
     if(isOld == false)
     {
     	// If the a new word is read, new struct pairs is created
     	// and the counter "value" is initialized to '1'.
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
    
    char* line;
    
    size_t size=50;

    while(1)
    {   // Read stdin until it reaches EOF
        if(getline(&line,&size,stdin) != -1)
        {   
        	// Extract the word name from (word,1) pair
        	line = strtok(strtok(line,"("),",");
        	// Word handle add the word to the linked list 
            word_handle(line);
         }
        else          
            break;
    }
    // Prints rest of the nodes in the linked list if left.
    print_contents();
return 0;
}
