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
        state[ph_id] = EATING;
        sleep(2);
        printf("Philosopher %d takes fork %d and %d\n",ph_id,LEFT,ph_id);
        printf("Philosopher %d is Eating\n",ph_id);
        sem_post(&S[ph_id]);
    }
}

void take_fork(int ph_id){
    sem_wait(mutex);
    state[ph_id] = HUNGRY;
    printf("Philosopher %d is Hungry\n",ph_id);
    test(ph_id);
    sem_post(mutex);
    sem_wait(&S[ph_id]);
    //sleep(1);
}

void put_fork(int ph_id){
    sem_wait(mutex);
    state[ph_id] = THINKING;
    
    printf("Philosopher %d putting fork %d and %d down\n",ph_id,LEFT,ph_id);
    printf("Philosopher %d is thinking\n",ph_id);
    
    test(LEFT);
    test(RIGHT);
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
    //state = malloc(N*sizeof(int));
    //S = malloc(N*sizeof(sem_t));

    int state_fd = open("statefile",O_RDWR |O_CREAT | O_TRUNC, (mode_t)0600 );
    ftruncate(state_fd,sizeof(N*sizeof(int)));
    if(state_fd == -1) 
    {
        perror ("state file open failed");
        return 1;
    }

    int mutex_fd = open("mutexfile",O_RDWR |O_CREAT | O_TRUNC, (mode_t)0600 );
    ftruncate(mutex_fd,sizeof(sem_t));
    if(mutex_fd == -1) 
    {
        perror ("mutex file open failed");
        return 1;
    }

    int sem_fd = open("semfile",O_RDWR |O_CREAT | O_TRUNC, (mode_t)0600 );
    ftruncate(sem_fd,N*sizeof(sem_t));
    if(sem_fd == -1) 
    {
        perror ("sem file open failed");
        return 1;
    }

    int b_fd = open("barfile",O_RDWR |O_CREAT | O_TRUNC, (mode_t)0600 );
    ftruncate(b_fd,sizeof(pthread_barrier_t));
    if(b_fd == -1) 
    {
        perror ("barrier file open failed");
        return 1;
    }

    state = mmap(NULL,N*sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED,state_fd,0);
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
        chid =fork();
        if(chid < 0)
        {
            perror("Couldn't create child");
            exit(1);
        }

        else if (chid == 0)
        {  while(iter < M)
            {
                printf("PHILOSOPHER = %d | PPROCESS ID = %d | ITERATION = %d\n",i,getpid(),iter+1);
                sleep(1);
                take_fork(i);
                sleep(2);
                put_fork(i);
                iter++;
                sleep(0.5);
                pthread_barrier_wait(barrier1);
            }
            exit(1);
        }
    }

    for(i =0; i < N; i++)
        wait(NULL);

    exit(1);
    return 0;   
}