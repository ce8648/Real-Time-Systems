#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <iostream>
#include <chrono>
#include <cmath>
#include <unistd.h>
#include <signal.h>

void *taskTau_1(void *);
void *taskTau_2(void *);
void *taskTau_3(void *);
pthread_mutex_t mutex_section1 = PTHREAD_MUTEX_INITIALIZER;

void pinCPU (int cpu_number)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);

    CPU_SET(cpu_number, &mask);

    if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) == -1)
    {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }
}

void setSchedulingPolicy (int newPolicy, int priority)
{
    sched_param sched;
    int oldPolicy;
    if (pthread_getschedparam(pthread_self(), &oldPolicy, &sched)) {
        perror("pthread_setschedparam");
        exit(EXIT_FAILURE);
    }
    sched.sched_priority = priority;
    if (pthread_setschedparam(pthread_self(), newPolicy, &sched)) {
        perror("pthread_setschedparam");
        exit(EXIT_FAILURE);
    }
}

void workload_1ms (void)
{
    int repeat = 100000; // tune this for the right amount of workload
    for (int i = 0; i <= repeat; i++){
        // add some computation here (e.g., use sqrt() in cmath)
        sqrt(sqrt(i));
    }
}

int r1 = 0, r2 = 0, r3 = 0;

extern int main(void)
{
    pinCPU (11);
    pthread_t tau_1, tau_2, tau_3;
    pthread_t *pool = (pthread_t *)malloc(10*sizeof(pthread_t));

    // similarly, the PIP is applied to the mutex
    pthread_mutexattr_t mutexattr_prioinherit;
    int mutex_protocol;
    pthread_mutexattr_init (&mutexattr_prioinherit);
    pthread_mutexattr_getprotocol (&mutexattr_prioinherit, &mutex_protocol);
    if (mutex_protocol != PTHREAD_PRIO_INHERIT)
    {
        pthread_mutexattr_setprotocol (&mutexattr_prioinherit, PTHREAD_PRIO_INHERIT);
    }
    pthread_mutex_init (&mutex_section1, &mutexattr_prioinherit);    

    if (pthread_create(&tau_1, NULL, taskTau_1, NULL) != 0)
	    perror("pthread_create"), exit(1); 

    if (pthread_create(&tau_2, NULL, taskTau_2, NULL) != 0)
	    perror("pthread_create"), exit(1); 

    if (pthread_create(&tau_3, NULL, taskTau_3, NULL) != 0)
	    perror("pthread_create"), exit(1); 

    for(int i=0;i<8;i++){
	    if(pthread_create(&pool[i],NULL,taskTau_2,(void*)&r2)!=0)
		    perror("pthread_create"), exit(1); 
    }

  
    if (pthread_join(tau_1, NULL) != 0)
	    perror("pthread_join"),exit(1);

    if (pthread_join(tau_2, NULL) != 0)
	    perror("pthread_join"),exit(1);
    
    if (pthread_join(tau_3, NULL) != 0)
	    perror("pthread_join"),exit(1);
    
    for(int i=0;i<8;i++){
        if(pthread_join(pool[i],NULL)!=0)
		    perror("pthread_join"), exit(1);
    }

    //do_wrap_up(r1, r2);

  return 0; 
}

void *taskTau_1(void *pnum_times)
{
    //pinCPU (0);
    int period = 50000; // unit: microsecond
    int delta;
    setSchedulingPolicy (SCHED_FIFO, 99);
    while (1){
        std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
        pthread_mutex_lock (&mutex_section1);
	    // TODO: add some workload here
	    workload_1ms ();
        pthread_mutex_unlock (&mutex_section1);
        std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
        delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        if (delta > period){
            continue;
        }
        else{
            usleep (period-delta);
        }
        std::cout << "99 priority task response time: " << delta << std::endl;
    }
    return(NULL);
}

void *taskTau_2(void *pnum_times)
{
    //pinCPU (0);
    int period = 100000; // unit: microsecond
    int delta;
    setSchedulingPolicy (SCHED_FIFO, 98);
    while (1){
        std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
        for (int j = 0; j < 8; j++){
            workload_1ms ();
        }
        std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
        delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        if (delta > period){
            continue;
        }
        else{
            usleep (period-delta);
        }
        //std::cout << "98 priority task response time: " << delta << std::endl;
    }
    return(NULL);
}

void *taskTau_3(void *pnum_times)
{
    //pinCPU (0);
    int period = 200000; // unit: microsecond
    int delta;
    setSchedulingPolicy (SCHED_FIFO, 97);
    while (1){
        std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
        
        for (int j = 0; j < 1; j++){
            workload_1ms ();
        }
        pthread_mutex_lock (&mutex_section1);
	    // TODO: add some workload here
	    for (int j = 0; j < 10; j++){
            workload_1ms ();
        }
        pthread_mutex_unlock (&mutex_section1);
        for (int j = 0; j < 1; j++){
            workload_1ms ();
        }
        
        std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
        delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        if (delta > period){
            continue;
        }
        else{
            usleep (period-delta);
        }
        //std::cout << "97 priority task response time: " << delta << std::endl;
    }
    return(NULL);
}
