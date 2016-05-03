d//////////////////////////////////////////////////
//                                              //
// Title: Dining Philosopher- Multi process code//
// Author: Sai Raghu Vamsi Anumula              //
// UFID: 49939544                               //
//                                              //        
//////////////////////////////////////////////////
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

#define THINKING 0
#define HUNGRY 1
#define EATING 2

int N;

#define LEFT (ph_id-1+N)%N
#define RIGHT (ph_id+1)%N

pthread_barrier_t *barrier1;
pthread_barrierattr_t battr;
sem_t *mutex,*S;
int *state;

void test(int ph_id){
    if (state[ph_id] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING){
        //Change state to EATING if left & right people are not eating
        state[ph_id] = EATING;
        sleep(1);
        //printf("Philosopher %d takes fork %d and %d \n",ph_id,LEFT,ph_id);
        printf("Philosopher %d is EATING\n",ph_id);
        sem_post(&S[ph_id]);
    }
}

void take_fork(int ph_id){
    sem_wait(mutex);
    // get mutex and change state to HUNGRY
    state[ph_id] = HUNGRY;
    sleep(0.1);
    printf("Philosopher %d is HUNGRY\n",ph_id);
    test(ph_id);
    sem_post(mutex);
    sem_wait(&S[ph_id]);
}

void put_fork(int ph_id){
    //get mutex
    sem_wait(mutex);
    //change the state to thinking
    state[ph_id] = THINKING;
    sleep(0.1);
    //printf("Philosopher %d putting down fork %d and %d\n",ph_id,LEFT,ph_id);
    printf("Philosopher %d is THINKING\n",ph_id);
    //check left and right to allow them to eat
    test(LEFT);
    test(RIGHT);
    //release mutex
    sem_post(mutex);
}

int main(int argc,char* argv[])
{ 

    int M = atoi(argv[2]);
    int iter =0;
    pid_t chid;
    int i;
    N = atoi(argv[1]);
    // Initialize variables
    // INitialize file descriptors for each shared variable
    int state_fd = open("_statefile_",O_RDWR |O_CREAT | O_TRUNC, (mode_t)0600 );
    ftruncate(state_fd,sizeof(N*sizeof(int)));
    if(state_fd == -1) 
    {
        perror ("state file open failed");
        return 1;
    }

    int mutex_fd = open("_mutexfile_",O_RDWR |O_CREAT | O_TRUNC, (mode_t)0600 );
    ftruncate(mutex_fd,sizeof(sem_t));
    if(mutex_fd == -1) 
    {
        perror ("mutex file open failed");
        return 1;
    }

    int sem_fd = open("_semfile_",O_RDWR |O_CREAT | O_TRUNC, (mode_t)0600 );
    ftruncate(sem_fd,N*sizeof(sem_t));
    if(sem_fd == -1) 
    {
        perror ("sem file open failed");
        return 1;
    }

    int b_fd = open("_barfile_",O_RDWR |O_CREAT | O_TRUNC, (mode_t)0600 );
    ftruncate(b_fd,sizeof(pthread_barrier_t));
    if(b_fd == -1) 
    {
        perror ("barrier file open failed");
        return 1;
    }
    // mmap to allocate memory to each shared varaible 
    state = mmap(NULL,N*sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED,state_fd,0);
    // Report error if mmap fails
    if(state == MAP_FAILED) 
    {
        perror ("mmap mutex failed");
        return 1;
    }
    // Init state arrary
    for(i =0; i < N ; i++)
        state[i] = THINKING;

    mutex = (sem_t *)mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE,MAP_SHARED,mutex_fd,0);

    if(mutex == MAP_FAILED) 
    {
        perror ("mmap mutex failed");
        return 1;
    }

    S = mmap(NULL,N*sizeof(sem_t),PROT_READ | PROT_WRITE,MAP_SHARED,sem_fd,0);

    if(S == MAP_FAILED) 
    {
        perror ("mmap sem failed");
        return 1;
    }

    barrier1 = mmap(NULL,sizeof(pthread_barrier_t),PROT_READ | PROT_WRITE,MAP_SHARED,b_fd,0);

    if(barrier1 == MAP_FAILED) 
    {
        perror ("mmap barrier failed");
        return 1;
    }

    // Init mutex 
    if( sem_init(mutex, 1, 1) != 0 )
    { fprintf(stderr,"sem_init error"), exit(-2);}
    
    // Init phil sems
    for(i=0;i<N;i++)
        sem_init(&S[i],1,0);
    int r;
    r = pthread_barrierattr_init(&battr);
    if(r)
    {
        fprintf(stderr, "pthread_barrierattr_init: %s\n", strerror(r));
        exit(1);
    }
    
    r = pthread_barrierattr_setpshared(&battr,PTHREAD_PROCESS_SHARED);

    if(r)
    {
        fprintf(stderr, "pthread_barrieratt_set: %s\n", strerror(r));
        exit(1);
    }

    // init barrier
    int rc = pthread_barrier_init(barrier1, &battr, N);
    if (rc) 
    {
        fprintf(stderr, "pthread_barrier_init: %s\n", strerror(rc));
        exit(1);
    }
    


    for(i =0; i< N ; i++)
    {   
        // Fork N child processes
        chid =fork();
        if(chid < 0)
        {
            perror("Couldn't create child");
            exit(1);
        }

        else if (chid == 0)
        {  // Start philosopher loop in each of child process.
           while(iter < M)
            {
                printf("PHILOSOPHER = %d | PPROCESS ID = %d | ITERATION = %d\n",i,getpid(),iter+1);
                sleep(1);
                take_fork(i);
                sleep(1);
                put_fork(i);
                iter++;
                sleep(1);
                pthread_barrier_wait(barrier1);
            }
            exit(1);
        }
    }
    
    // Wait until all the childs exit.
    for(i =0; i < N; i++)
        wait(NULL);

    //Un-map shared memory 
    munmap(state, N*sizeof(int));
    munmap(mutex, sizeof(sem_t));
    munmap(S,N*sizeof(sem_t));
    munmap(barrier1,sizeof(pthread_barrier_t));

    // Remove files once done 
    remove("_barfile_");
    remove("_semfile_");
    remove("_mutexfile_");
    remove("_statefile_");

    exit(1);
    return 0;   
}
