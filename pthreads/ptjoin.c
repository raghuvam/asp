#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

void * start_thread (void *message)
{
    while(1)
    {
        printf ("%s\n", (const char *) message);    
    }
    
}

int main (void)
{
    pthread_t thing1, thing2;
    
    const char *message1 = "Thing 1";
    const char *message2 = "Thing 2";

    /* Create two threads, each with a different message. */
    pthread_create(&thing1, NULL, start_thread, (void *) message1);
    pthread_create(&thing2, NULL, start_thread, (void *) message2);
    
    /*
    * Wait for the threads to exit. If we didn't join here,
    * we'd risk terminating this main thread before the
    * other two threads finished.
    */
    
    pthread_detach(thing1);
    pthread_detach(thing2);
    return 0;
}