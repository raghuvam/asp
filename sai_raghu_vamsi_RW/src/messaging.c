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
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>


#define BUFFER_SIZE 10
static int BUFFER_ELE_SIZE = 256;

void exit_messaging();



struct buffer_area
{
    pthread_mutex_t mutex;
    int size;
    int count;
    int flag;
};

static struct buffer_area *mbuf;
static int buffer_fd;
static char *message;
static int msg_fd;


int init_messaging()
{
	/*Open file descriptors for each variable
	/*mutex variable*/


    buffer_fd = shm_open("_buffer_area_",O_RDWR |O_CREAT , (mode_t)0600 );
    ftruncate(buffer_fd,sizeof(struct buffer_area));

    if(buffer_fd == -1)
    {
    	perror("buffer file failed");
    	return -1;
    }

    /* mmap to allocate memory to each shared varaible */
    mbuf = (struct buffer_area *)mmap(NULL,sizeof(struct buffer_area),PROT_READ | PROT_WRITE,MAP_SHARED,buffer_fd,0);
    /* Report error if mmap fails*/
    if(mbuf == MAP_FAILED) 
    {
        perror ("mmap buffer init failed");
        return 1;
    }

    if(mbuf->flag != -111)
    {
        pthread_mutex_init(&mbuf->mutex,NULL);
        mbuf->size = BUFFER_ELE_SIZE;
        mbuf->count =0; 
        mbuf->flag =-111;
    }

    msg_fd = shm_open("_msg_area_",O_RDWR |O_CREAT  , (mode_t)0600 );
    ftruncate(msg_fd,mbuf->size);

    if(msg_fd == -1)
    {
        perror("buffer file failed");
        return -1;
    }

    /* mmap to allocate memory to each shared varaible */
    message = (char *)mmap(NULL,mbuf->size,PROT_READ | PROT_WRITE,MAP_SHARED,msg_fd,0);
    /* Report error if mmap fails*/
    if(message == MAP_FAILED) 
    {
        perror ("mmap buffer init failed");
        return 1;
    }

	return 1;
}

int ninit_messaging()
{
    /*Open file descriptors for each variable
    /*mutex variable*/


    buffer_fd = shm_open("_buffer_area_",O_RDWR |O_CREAT | O_TRUNC , (mode_t)0600 );
    ftruncate(buffer_fd,sizeof(struct buffer_area));

    if(buffer_fd == -1)
    {
        perror("buffer file failed");
        return -1;
    }

    /* mmap to allocate memory to each shared varaible */
    mbuf = (struct buffer_area *)mmap(NULL,sizeof(struct buffer_area),PROT_READ | PROT_WRITE,MAP_SHARED,buffer_fd,0);
    /* Report error if mmap fails*/
    if(mbuf == MAP_FAILED) 
    {
        perror ("mmap buffer init failed");
        return 1;
    }

    if(mbuf->flag != -111)
    {
        pthread_mutex_init(&mbuf->mutex,NULL);
        mbuf->size = BUFFER_ELE_SIZE;
        mbuf->count =0; 
        mbuf->flag =-111;
    }

    msg_fd = shm_open("_msg_area_",O_RDWR |O_CREAT | O_TRUNC , (mode_t)0600 );
    ftruncate(msg_fd,mbuf->size);

    if(msg_fd == -1)
    {
        perror("buffer file failed");
        return -1;
    }

    /* mmap to allocate memory to each shared varaible */
    message = (char *)mmap(NULL,mbuf->size,PROT_READ | PROT_WRITE,MAP_SHARED,msg_fd,0);
    /* Report error if mmap fails*/
    if(message == MAP_FAILED) 
    {
        perror ("mmap buffer init failed");
        return 1;
    }

    return 1;
}

void exit_messaging()
{
    size_t sz = mbuf->size;
    /*
 if (munmap(message, mbuf->size) == -1) {
    printf("prod: Unmap failed: %s\n", strerror(errno));
    exit(1);
   }*/
    munmap(message, mbuf->size);

  /* close the shared memory segment as if it was a file */
  if (close(msg_fd) == -1) {
    printf("Close failed: %s\n", strerror(errno));
    exit(1);
  }

  if (munmap(mbuf, sizeof(struct buffer_area)) == -1) {
    printf("1prod: Unmap failed: %s\n", strerror(errno));
    exit(1);
  }

  /* close the shared memory segment as if it was a file */
  if (close(buffer_fd) == -1) {
    printf("1Close failed: %s\n", strerror(errno));
    exit(1);
  }

  
}

int send_message(char *msg)
{
    
    init_messaging();
	
	pthread_mutex_lock(&mbuf->mutex);
	
    if(mbuf->count == 1)
    {
        pthread_mutex_unlock(&mbuf->mutex);
        exit_messaging();
        
        return 0;
    }
    
    if(strlen(msg) > mbuf->size)
    {
        mbuf->size = strlen(msg);
        ftruncate(msg_fd,mbuf->size);  
        /* mmap to allocate memory to each shared varaible */
        message = (char *)mmap(NULL,mbuf->size,PROT_READ | PROT_WRITE,MAP_SHARED,msg_fd,0);
        /* Report error if mmap fails*/
        if(message == MAP_FAILED) 
        {
            perror ("mmap buffer init failed");
            return 1;
        }
         
    }

	strcpy(message,msg);

    /*printf("Buffer updated: %s\n",message);*/
    mbuf->count =1;

	pthread_mutex_unlock(&mbuf->mutex);
    exit_messaging();
    return 1;
}

char *read_message()
{
	
    init_messaging();
    pthread_mutex_lock(&mbuf->mutex);

	if(mbuf->count == 0)
    {
        pthread_mutex_unlock(&mbuf->mutex);
        exit_messaging();
        return NULL;
    }
	
    
    mbuf->count =0;
    char *temp = (char *)malloc(strlen(message));
	strcpy(temp,message);

	pthread_mutex_unlock(&mbuf->mutex);
    exit_messaging();
	return temp;

}

int get_msg_count()
{
	int rt=0;
    init_messaging();
	/*get the mutex lock */
	pthread_mutex_lock(&mbuf->mutex);

	/* copy counter value to return variable*/
	rt = mbuf->count;

	pthread_mutex_unlock(&mbuf->mutex);

    exit_messaging();
	return rt;

}

int print_buf_info()
{   
    init_messaging();
    /*get the mutex lock */
    pthread_mutex_lock(&mbuf->mutex);
    printf("\nBUFFER INFO: \n");

    printf("   flag  -> %d  \n",mbuf->flag);
    printf("   MSG COUNT ->   %d  \n\n",mbuf->count);


    pthread_mutex_unlock(&mbuf->mutex);

    exit_messaging();

    return 0;

}