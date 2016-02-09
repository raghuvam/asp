/*
 ============================================================================
 Name        : multi_threading.c
 Author      : Sai raghu vamsi Anumula
 Version     : v1.0
 Copyright   : Your copyright notice
 Description : Multithreaded Mapper & Reducer Program using Pthreads
 ============================================================================
 
 RUNNING THE CODE:
1. Compile using make.
    $ make all
2. Running the code.
    $ ./map_red_sum  <input_text_file>  <no.of mappers>  <no.of reducers>  <no.of summarizers>
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define BUFFER_SIZE 10

int numReducers = 1;
int numMappers = 1;
int numSummarizers =1;

int nReducers = 1;
int nMappers = 1;
int nSummarizers =1;

// mapper pool updater variables
pthread_mutex_t mpool;
pthread_cond_t mpool_empty;
pthread_cond_t mpool_full;

struct words
{
	char data[25];
	int count;
	struct words *next;
};


char curr_c = '0';

struct words* head;

struct words *mpool_buffer[BUFFER_SIZE];
int m_index=0;
int mpool_out=0;
int mpool_in =0;
int mpool_eof = 0;

// reducer pool variables
pthread_mutex_t rpool;
pthread_cond_t rpool_empty;
pthread_cond_t rpool_full;

struct words *rpool_buffer[BUFFER_SIZE];
int r_index=0;
int rpool_out=0;
int rpool_in =0;
int rpool_eof = 0;

// summariser pool variables
pthread_mutex_t spool;
pthread_cond_t spool_empty;
pthread_cond_t spool_full;

struct words *spool_buffer[BUFFER_SIZE];
int s_index=0;
int spool_out=0;
int spool_in =0;
int spool_eof = 0;

int w_index=0;
int wpool_out=0;
int wpool_in=0;

// summariser pool variables
pthread_mutex_t table;
pthread_cond_t table_empty;
pthread_cond_t table_full;

struct words *table_buffer[BUFFER_SIZE];
int t_index=0;
int table_out=0;
int table_in =0;
int table_eof = 0;

pthread_mutex_t nmap,nred,nsum;

void print_list(struct words *top)
{
	while(top !=NULL)
	{
		printf("(%s,%d)\n",top->data,top->count);
		top = top->next;
	}
}

void write_to_mpool()
{
	////////////
	// Write the words linked to list to the Mapper buffer
	////////////
		pthread_mutex_lock(&mpool);

		while(m_index == BUFFER_SIZE)
		{
			pthread_cond_wait(&mpool_full,&mpool);
		}

		mpool_buffer[mpool_in] = head;
		mpool_in = (mpool_in + 1 ) % BUFFER_SIZE;
		m_index++;

		printf("W: m_index: %d mpool_eof: %d\n",m_index,mpool_eof);

		if(m_index == 1)
			pthread_cond_broadcast(&mpool_empty);

		pthread_mutex_unlock(&mpool);
	//Update Head to Null pointer
    head =NULL;
}

// Word handle: Checks the first letter of the new word and update it to the linked
//              list. Creates linked list of words starting with same character. 
//
void word_handle(char nword[])
{
	if(curr_c == '0')
        curr_c = nword[0];

    else
    { //Checks if the word is new starting charater
      if(curr_c != nword[0])
      {
        curr_c = nword[0];
        //Write the previous linked list to the Mapper pool [function has mpool lock] 
        write_to_mpool();
      }
    }

    if (head == NULL)
    {	
        head = (struct words*)malloc(sizeof(struct words));
        strcpy(head->data,nword);
        head->next =NULL;
    }

    else
    {
    	struct words* new = (struct words*)malloc(sizeof(struct words));
        strcpy(new->data,nword);
        new->next =head;
        head =new;
    }
}
// get counts of each word in the set.
struct words* get_wordcount(struct words *nwords)
{	
	struct words *top = NULL;

	while(nwords != NULL)
	{
		bool isOld = false;
		bool done = false;
		 if (top == NULL)
    	{	
    		// If a new word arrives the head points to NULL
    		// so a new node is created in the linked list and head
    		// points to that node
        	top = (struct words*)malloc(sizeof(struct words));
        	strcpy(top->data,nwords->data);
        	top->count =1;
        	top->next =NULL;
        	done = true;
    	}

    	struct words* temp = top;
    
    	while(temp != NULL && done != true)
    	{	
    		// Check if the word is already present in the linked list
        	if(strcmp(temp->data, nwords->data) == 0)
        	{
        		//Increments the counter if the word is already present
	        	temp->count++;
       	 		isOld =true;
        		done = true;
        	}
        	temp = temp->next;
     	}
     
     	if(isOld == false && done != true)
     	{
     		// If the a new word is read, new struct pairs is created
     		// and the counter "value" is initialized to '1'.
        	struct words* new = (struct words*)malloc(sizeof(struct words));
       	 	strcpy(new->data,nwords->data);
        	new->count=1;
        	new->next =top;
        	top =new;
      	}	
      	nwords = nwords->next;	
	}
	return top;
}

void get_tuples_list(struct words *nwords)
{
	while(nwords != NULL)
	{
		nwords->count =1;		
		nwords = nwords->next;
	}
}


struct words* get_lettercount(struct words *top)
{   
    int tcount =0;
    struct words* temp = top;
    while(top != NULL)
    {
        tcount = tcount + top->count;
        temp = top;
        top = top->next;
    }
    struct words *rv = (struct words*)malloc(sizeof(struct words));
    strcpy(rv->data,temp->data);
    rv->count = tcount;
    return rv;
}

void write_to_file(FILE *fh,struct words *top)
{
	while(top !=NULL)
	{

		fprintf(fh,"(%s,%d)\n",top->data,top->count);
		top = top->next;
	}
}


void *mapper_pool_updater( void *file )
{
	//printf("In the mapper pool updater\n");
	FILE *fh;
	char word[100];
	char *filename = (char *)file;
	fh = fopen(filename,"r");
	/* Aquire lock to mpool and write the data to mpool buffer */

	while(fscanf(fh,"%s",word) == 1)
	{
		printf("nw: %s \n",word);
		word_handle(word);
	}
	
	if(head != NULL)
		write_to_mpool();
	
	mpool_eof = 1;

	fclose(fh);
	pthread_exit(NULL);
}


void *mapper(void *id)
{	

	int *tid = (int *)id;
	struct words *nwords;
	int eof=0;
	while(m_index > 0 || !mpool_eof)
	{
		pthread_mutex_lock(&mpool);

		while(m_index == 0 && !mpool_eof)
			pthread_cond_wait(&mpool_empty,&mpool); // getting stuck here after data is written

        if(m_index == 0 && mpool_eof)
        {
            pthread_mutex_unlock(&mpool);
            break;
        }
		nwords = mpool_buffer[mpool_out];
		mpool_out = (mpool_out + 1) % BUFFER_SIZE;
		m_index--;

		if(m_index == BUFFER_SIZE-1)
			pthread_cond_broadcast(&mpool_full);

        pthread_mutex_unlock(&mpool);
		
		get_tuples_list(nwords);
        
		// Write the tuples to the reducer pool
		pthread_mutex_lock(&rpool);

		while(r_index == BUFFER_SIZE)
		{
			pthread_cond_wait(&rpool_full,&rpool);
		}
		//
		rpool_buffer[rpool_in] = nwords;
		rpool_in = (rpool_in + 1 ) % BUFFER_SIZE;
		r_index++;

		if(r_index == 1)
			pthread_cond_broadcast(&rpool_empty);
      
		pthread_mutex_unlock(&rpool);

		printf("R: m_index: %d mpool_eof: %d\n",m_index,mpool_eof);
	}
	
    printf("Exiting mapper thread \n");
    
    pthread_mutex_lock(&nmap);
    nMappers--;
    pthread_mutex_unlock(&nmap);
    
    pthread_cond_broadcast(&mpool_empty);
    if(nMappers == 0)
    {
        rpool_eof =1;
        pthread_cond_broadcast(&rpool_empty);
        printf("All Mappers exited\n");    
    }
	

	pthread_exit(NULL);
}


void *reducer(void *id)
{	
	int *tid = (int *)id; 
	printf("Created thread %d\n",*tid);
	struct words* nwords;
	
	while(r_index > 0 || !rpool_eof)
	{
		pthread_mutex_lock(&rpool);

		while(r_index == 0 && ! rpool_eof)
			pthread_cond_wait(&rpool_empty,&rpool); // getting stuck here after data is written

		if(rpool_eof == 1 && r_index == 0)
		{
			printf("rpool empty \n");
            pthread_mutex_unlock(&rpool);
			break;
		}

		nwords = rpool_buffer[rpool_out];
		rpool_buffer[rpool_out] = NULL;
		rpool_out = (rpool_out + 1) % BUFFER_SIZE;
		r_index--;

		if(r_index == BUFFER_SIZE-1)
			pthread_cond_broadcast(&rpool_full);

        pthread_mutex_unlock(&rpool);
		
		printf("tid: %d R r_index: %d rpool_eof: %d\n",*tid,r_index,rpool_eof);
		// release the lock and get the word counts
		struct words *cwords = get_wordcount(nwords);
		////////////
		// write words count to summarizer pool
		pthread_mutex_lock(&spool);

		while(w_index == BUFFER_SIZE || s_index == BUFFER_SIZE)
		{
			pthread_cond_wait(&spool_full,&mpool);
		}
		//strcpy(mpool_buffer[mpool_in],word); // copy the scanned word to the buffer
		spool_buffer[spool_in] = cwords;
		spool_in = (spool_in + 1 ) % BUFFER_SIZE;
		s_index++;
		w_index++;
		
		if(w_index == 1 && s_index == 1)
			pthread_cond_broadcast(&spool_empty);		
        pthread_mutex_unlock(&spool);

		printf("tid: %d W w_index: %d spool_eof: %d\n",*tid,w_index,spool_eof);

	}
	printf("Exiting Reducer thread tid: %d\n",*tid);
    
    pthread_mutex_lock(&nred);
    nReducers--;
    pthread_mutex_unlock(&nred);

    pthread_cond_broadcast(&rpool_empty);
    if(nReducers == 0)
    {
        printf("All Reducers done \n");
        spool_eof = 1;
        pthread_cond_broadcast(&spool_empty);
    }
	pthread_exit(NULL);
}

void * wcount_writer()
{
	struct words *nwords;
	
	FILE *fh;
	fh = fopen("./wordCount.txt","w");
	while(w_index > 0 || !spool_eof)
	{
	pthread_mutex_lock(&spool);

		while(w_index == 0 && !spool_eof)
			pthread_cond_wait(&spool_empty,&spool); // getting stuck here after data is written

		if(w_index == 0 && spool_eof)
        {
            pthread_mutex_unlock(&spool);
			break;
        }

		nwords = spool_buffer[wpool_out];
		wpool_out = (wpool_out + 1) % BUFFER_SIZE;
		w_index--;

		if(w_index == BUFFER_SIZE-1)
			pthread_cond_broadcast(&spool_full);

		pthread_mutex_unlock(&spool);

		write_to_file(fh,nwords);
		printf("R w_index: %d spool_eof: %d\n",w_index,spool_eof);

	}
	close(fh);
	printf("Exiting wcount_writer\n");
	
	pthread_exit(NULL);
}

void *summarizer(void *id)
{
    int *tid = (int *)id; 
    printf("Created thread %d\n",*tid);

	struct words *nwords;
	while(s_index > 0 || !spool_eof)
	{
	    pthread_mutex_lock(&spool);

		while(s_index == 0 && !spool_eof)
			pthread_cond_wait(&spool_empty,&spool); // getting stuck here after data is written

		if(s_index == 0 && spool_eof ==1)
        {   
            pthread_mutex_unlock(&spool);
			break;
        }
		nwords = spool_buffer[spool_out];
		spool_out = (spool_out + 1) % BUFFER_SIZE;
		s_index--;
		
		if(s_index == BUFFER_SIZE-1)
			pthread_cond_broadcast(&spool_full);

        // release the lock and get the word counts
        struct words *cwords = get_lettercount(nwords);

        ////////////
        // write words count to summarizer pool
        pthread_mutex_lock(&table);

        while(t_index == BUFFER_SIZE)
            pthread_cond_wait(&table_full,&table);

        //strcpy(mpool_buffer[mpool_in],word); // copy the scanned word to the buffer
        table_buffer[table_in] = cwords;
        table_in = (table_in + 1 ) % BUFFER_SIZE;
        t_index++;
        
        if(t_index == 1)
            pthread_cond_broadcast(&table_empty);


        pthread_mutex_unlock(&spool);
		pthread_mutex_unlock(&table);
	}
    //get lock to the sum count
    pthread_mutex_lock(&nsum);
    nSummarizers--;
    pthread_mutex_unlock(&nsum);

    pthread_cond_broadcast(&spool_empty);
    if(nSummarizers == 0)
    {
        printf("All Summarizers exited\n");
        table_eof = 1;
        pthread_cond_broadcast(&table_empty);
    }
	pthread_exit(NULL);
}



void *table_writer()
{
    FILE *fh;
    fh = fopen("./letterCount.txt","w");
    struct words *nwords;
    while(t_index > 0 || !table_eof)
    {
        pthread_mutex_lock(&table);

        while(t_index == 0 && !table_eof)
            pthread_cond_wait(&table_empty,&table); // getting stuck here after data is written

        if(t_index == 0 && table_eof ==1)
        {   
            pthread_mutex_unlock(&table);
            break;
        }
        nwords = table_buffer[table_out];
        table_out = (table_out + 1) % BUFFER_SIZE;
        t_index--;
        
        if(t_index == BUFFER_SIZE-1)
            pthread_cond_broadcast(&table_full);
        pthread_mutex_unlock(&table);
        fprintf(fh, "(%c,%d)\n",nwords->data[0],nwords->count);

    }

    printf("Exiting letter counter\n");
    close(fh);
    pthread_exit(NULL);

}


int main(int argc, char* argv[]) {
	if (argc < 5)
	{
		perror("./multi_threading input.txt <num_of_mappers> \n");
		exit(0);
	}
    numMappers = atoi(argv[2]);
    numReducers = atoi(argv[3]);
    numSummarizers = atoi(argv[4]);

    nReducers = numReducers;
    nMappers = numMappers;
    nSummarizers = numSummarizers;

    printf("Creating %d Mappers, %d Reducers, %d Summarizers\n",nMappers,nReducers,nSummarizers );

	//initialize mapper pool mutex variable
	pthread_mutex_init(&mpool,NULL);
	pthread_cond_init(&mpool_full,NULL);
	pthread_cond_init(&mpool_empty,NULL);

	//initialize reducer pool mutex variable
	pthread_mutex_init(&rpool,NULL);
	pthread_cond_init(&rpool_full,NULL);
	pthread_cond_init(&rpool_empty,NULL);

	//initialize reducer pool mutex variable
	pthread_mutex_init(&spool,NULL);
	pthread_cond_init(&spool_full,NULL);
	pthread_cond_init(&spool_empty,NULL);

    //initialize reducer pool mutex variable
    pthread_mutex_init(&table,NULL);
    pthread_cond_init(&table_full,NULL);
    pthread_cond_init(&table_empty,NULL);

    // thread count locks
    pthread_mutex_init(&nmap,NULL);
    pthread_mutex_init(&nred,NULL);
    pthread_mutex_init(&nsum,NULL);

	// Declare thread variables.
	pthread_t MapperUpdater;
	pthread_t Mappers[10]; // mapper threads
	pthread_t Reducers[10];
	pthread_t WcountWriter;
	pthread_t Summarizers[10];
    pthread_t TableWriter;

	// Create Mapper pool updater thread.
	int rc;


	rc = pthread_create(&MapperUpdater, NULL, mapper_pool_updater, (void *)argv[1]);
	if (rc)
	{
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
     }

    // Create Mapper threads
    int i;
    for(i =0; i < numMappers; i++)
    {
    	int *tid = (int *)malloc(sizeof(int));
		*tid = i;
    	rc = pthread_create(&Mappers[i], NULL, mapper, (void *)tid);
		if (rc){
         	printf("ERROR; return code from pthread_create() is %d\n", rc);
         	exit(-1);
      	}

    }
    // Create Reducer threads
    i = 0;
    for(i =0; i < numReducers; i++)
    {
    	int *tid = (int *)malloc(sizeof(int));
		*tid = i;
    	rc = pthread_create(&Reducers[i], NULL, reducer, (void *)tid);
		if (rc){
         	printf("ERROR; return code from reducer pthread_create() is %d\n", rc);
         	exit(-1);
      	}

    }
    
    // Create Mapper pool updater thread.
	rc = pthread_create(&WcountWriter, NULL, wcount_writer,NULL);
	if (rc)
	{
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
     }
	
    // Create Summerizers threads
    i = 0;
    for(i =0; i < numSummarizers; i++)
    {
        int *tid = (int *)malloc(sizeof(int));
        *tid = i;
    	rc = pthread_create(&Summarizers[i], NULL, summarizer, (void *)tid);
		if (rc){
         	printf("ERROR; return code from reducer pthread_create() is %d\n", rc);
         	exit(-1);
      	}

    }

    // Create Table writer thread.
    rc = pthread_create(&TableWriter, NULL, table_writer,NULL);
    if (rc)
    {
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
     }

     
    //Join Threads
    //Join Mapper updater
    pthread_join(MapperUpdater,NULL);
    //Join Mappers
    for(i=0;i< numMappers; i++)
        pthread_join(Mappers[i],NULL);
    // Join Reducers
    for(i=0;i< numReducers; i++)
        pthread_join(Reducers[i],NULL);
    //Join Summarizers
    for(i=0;i< numSummarizers; i++)
        pthread_join(Summarizers[i],NULL);
    //Join word count writer
    pthread_join(WcountWriter,NULL);
    //Join Letter count updater
    pthread_join(TableWriter,NULL);

	pthread_exit(NULL);



	return EXIT_SUCCESS;
	exit(1);
}
