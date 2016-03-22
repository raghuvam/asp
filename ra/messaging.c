#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include "messaging.h"


#define BUFFER_SIZE 10
#define BUFFER_ELE_SIZE 256

static pthread_mutex_t *mutex;
static pthread_cond_t *buffer_empty;
static pthread_cond_t *buffer_full;

static int *count;

struct message
{
	char data[100];
};
static struct message *msg_buffer;

static int buffer_in = 0;
static int buffer_out = 0;


int init_messaging()
{
	//Open file descriptors for each variable
	//mutex variable
	int mutex_fd  = open("_mutex_",O_RDWR |O_CREAT | O_TRUNC, (mode_t)0600 );
	ftruncate(mutex_fd,sizeof(pthread_mutex_t));
	
	if(mutex_fd == -1) 
    {
        perror ("mutex file open failed");
        return -1;
    }

    int bf_full_fd  = open("_full_",O_RDWR |O_CREAT | O_TRUNC, (mode_t)0600 );
    ftruncate(bf_full_fd,sizeof(pthread_cond_t));

    if(bf_full_fd == -1)
    {
    	perror("cond mutex init failed");
    	return -1;
    }

    int bf_empty_fd  = open("_empty_",O_RDWR |O_CREAT | O_TRUNC, (mode_t)0600 );
    ftruncate(bf_empty_fd,sizeof(pthread_cond_t));

    if(bf_empty_fd == -1)
    {
    	perror("cond mutex init failed");
    	return -1;
    }

    int buffer_fd = open("_buffer_",O_RDWR |O_CREAT | O_TRUNC, (mode_t)0600 );
    ftruncate(buffer_fd,BUFFER_SIZE*sizeof(struct message));

    if(buffer_fd == -1)
    {
    	perror("buffer file failed");
    	return -1;
    }

    int count_fd = open("_count_",O_RDWR |O_CREAT | O_TRUNC, (mode_t)0600 );
    ftruncate(count_fd,sizeof(int));

    if(count_fd == -1)
    {
    	perror("count file failed");
    	return -1;
    }

    // mmap to allocate memory to each shared varaible 
    count = (int *)mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED,count_fd,0);
    // Report error if mmap fails
    if(count == MAP_FAILED) 
    {
        perror ("mmap count init failed");
        return 1;
    }

    // mmap to allocate memory to each shared varaible 
    msg_buffer = mmap(NULL,BUFFER_SIZE*sizeof(struct message),PROT_READ | PROT_WRITE,MAP_SHARED,buffer_fd,0);
    // Report error if mmap fails
    if(msg_buffer == MAP_FAILED) 
    {
        perror ("mmap buffer init failed");
        return 1;
    }

    // mmap to allocate memory to each shared varaible 
    mutex = mmap(NULL,sizeof(pthread_mutex_t),PROT_READ | PROT_WRITE,MAP_SHARED,mutex_fd,0);
    // Report error if mmap fails
    if(mutex == MAP_FAILED) 
    {
        perror ("mmap mutex failed");
        return 1;
    }

    // mmap to allocate memory to each shared varaible 
    buffer_full = mmap(NULL,sizeof(pthread_cond_t),PROT_READ | PROT_WRITE,MAP_SHARED,bf_full_fd,0);
    // Report error if mmap fails
    if(buffer_full == MAP_FAILED) 
    {
        perror ("mmap buffer full mutex failed");
        return 1;
    }

    // mmap to allocate memory to each shared varaible 
    buffer_empty = mmap(NULL,sizeof(pthread_cond_t),PROT_READ | PROT_WRITE,MAP_SHARED,bf_empty_fd,0);
    // Report error if mmap fails
    if(buffer_empty == MAP_FAILED) 
    {
        perror ("mmap buffer empty mutex failed");
        return 1;
    }

	pthread_mutex_init(mutex,NULL);
	pthread_cond_init(buffer_full,NULL);
	pthread_cond_init(buffer_empty,NULL);



	return 1;
}


int send_message(char *msg)
{
	//get the mutex lock 
	pthread_mutex_lock(mutex);
	
	while(*count == BUFFER_SIZE)
	 {
		pthread_cond_wait(buffer_full,mutex);
	 }

	memcpy(msg_buffer[buffer_in].data,msg,strlen(msg));
	buffer_in = (buffer_in + 1) % BUFFER_SIZE;
	*count = *count + 1;

	if(*count == 1)
		pthread_cond_broadcast(buffer_empty);

	pthread_mutex_unlock(mutex);
	//doubt 
    msg =NULL;
    return 1;
}

char *read_message()
{
	//get the mutex lock 
	char temp[100];
	pthread_mutex_lock(mutex);

	while(*count == 0)
		pthread_cond_wait(buffer_empty,mutex);
	// use memcpy
	memcpy(temp,msg_buffer[buffer_out].data, strlen(msg_buffer[buffer_out].data));
	buffer_out = (buffer_out +1) % BUFFER_SIZE;
	*count = *count - 1;

	if(*count == (BUFFER_SIZE-1))
		pthread_cond_broadcast(buffer_full);

	pthread_mutex_unlock(mutex);
	
	return temp;

}

int get_msg_count()
{
	int rt=0;

	//get the mutex lock 
	pthread_mutex_lock(mutex);

	// copy counter value to return variable
	rt = *count;

	pthread_mutex_unlock(mutex);

	return rt;

}