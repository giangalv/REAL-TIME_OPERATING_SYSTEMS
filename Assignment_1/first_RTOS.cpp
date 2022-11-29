// FIRST ASSIGNMENT RTOS - Galvagni Gianluca s5521188

// APPLICATION:
// <1> Design an application with 3 threads J1, J2, J3, whose periods are 300ms, 500ms, and 800ms, 
//     plus an aperiodic thread J4 in background which is triggered by J2.
// <2> The threads shall just "waste time," as we did in the exercise with threads.
// <3> Design a simple driver with only open, close, and write system calls.
// <4> During its execution, every task 
//     (a) opens the special file associated with the driver;
//     (b) writes to the driver its identifier plus open square brackets (i.e., [1, [2, [3, or [4);
//     (c) close the special files;
//     (d) performs operations (i.e., wasting time);
//     (e) performs (i)(ii) and (iii) again to write to the driver its identifier, 
//         but with closed square brackets (i.e., 1], 2], 3] or 4]).
// <5> The write system call writes on the kernel log the string received from the thread. 
//     A typical output of the system, when reading the kernel log, can be the following [11][2[11]2][3[11]3][4]. 
//     This sequence clearly shows that some threads can be preempted by other threads 
//     (if this does not happen, try to increase the computational time of longer tasks).
// <6> Finally, modify the code of all tasks to use semaphores. 
//     Every thread now protects all its operations (i) to (v) with a semaphore,which prevents other tasks from preempting. 
//     Specifically, use semaphores with a priority ceiling access protocol.  

//-------------------------------------LIBRARIES----------------------------------------
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

//-------------------------------GLOBAL VARIABLES------------------------------------------
#define PERIOD_1 300000000 // 300ms in nanoseconds
#define PERIOD_2 500000000  // 500ms in nanoseconds
#define PERIOD_3 800000000  // 800ms in nanoseconds

#define PERIODIC_TASKS 3
#define APERIODIC_TASKS 1
#define TASKS PERIODIC_TASKS + APERIODIC_TASKS

#define INNERLOOP 100
#define OUTERLOOP 2000

#define MAX_PRIORITY 99
#define MIN_PRIORITY 1

#define MAX_SEMAPHORES 4

//---------------------------------FUNCTIONS-----------------------------------------------

// function to waste time
void waste_time(){
    int i;
    for(int l=0; l<FIRST_LOOP; l++){
        for(int k=0; k<SECOND_LOOP; k++){
            i = i + 1;           
        }
    }
}

// INIZIALIZATION OF PERIODIC TASKS

void task1_code(){
    waste_time();
}
void task2_code()
{
    waste_time();
}
void task3_code()
{
    waste_time();
}

// INIZIALIZATION OF APERIODIC TASKS

void task4_code()
{
    waste_time();
}

// CARATERISTICS FUNCTIONS OF PERIODIC TRHEADS

void *task1(void *arg)
{
    struct timespec next_activation;
    struct timespec period;
    period.tv_sec = 0;
    period.tv_nsec = PERIOD_1;
    clock_gettime(CLOCK_MONOTONIC, &next_activation);
    while(1)
    {
        task1_code();
        next_activation.tv_nsec += period.tv_nsec;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_activation, NULL);
    }
}

void *task2(void *arg)
{
    struct timespec next_activation;
    struct timespec period;
    period.tv_sec = 0;
    period.tv_nsec = PERIOD_2;
    clock_gettime(CLOCK_MONOTONIC, &next_activation);
    while(1)
    {
        task2_code();
        next_activation.tv_nsec += period.tv_nsec;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_activation, NULL);
    }
}

void *task3(void *arg)
{
    struct timespec next_activation;
    struct timespec period;
    period.tv_sec = 0;
    period.tv_nsec = PERIOD_3;
    clock_gettime(CLOCK_MONOTONIC, &next_activation);
    while(1)
    {
        task3_code();
        next_activation.tv_nsec += period.tv_nsec;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_activation, NULL);
    }
}

// CARATERISTICS FUNCTIONS OF APERIODIC TRHEADS

void *task4(void *arg)
{
    // set thread affinity
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    // infinite loop
    while(1)
    {
        task4_code();
    }
}

long int periods[TASKS] = {PERIOD_1, PERIOD_2, PERIOD_3, 0};
struct timespec next_activation[TASKS];
double wcet[TASKS] = {0, 0, 0, 0};
pthread_attr_t attr[TASKS];
pthread_t thread[TASKS];
struct sched_param param[TASKS];
int missed_deadlines[TASKS] = {0, 0, 0, 0};

// ---------------------------------MAIN----------------------------------------------------

int main()
{
    // Set the task periods in nanoseconds
    peridiods[0] = PERIOD_1;
    peridiods[1] = PERIOD_2;
    peridiods[2] = PERIOD_3;

    // for aperiority task we set the period to 0
    peridiods[3] = 0;

    // Set the WCET in nanoseconds
    wcet[0] = 0;
    wcet[1] = 0;
    wcet[2] = 0;
    wcet[3] = 0;

    // set the max and min priority
    struct sched_param priomax;
    priomax.sched_priority = MAX_PRIORITY;
    struct sched_param priomin;
    priomin.sched_priority = MIN_PRIORITY; 

    if (geteuid() == 0)
    {
        // Set the scheduling policy to SCHED_FIFO
        pthread_attr_setschedpolicy(&task1_attr, SCHED_FIFO);
        pthread_attr_setschedpolicy(&task2_attr, SCHED_FIFO);
        pthread_attr_setschedpolicy(&task3_attr, SCHED_FIFO);
        pthread_attr_setschedpolicy(&task4_attr, SCHED_FIFO);

        // Set the scheduling priority of the threads
        task1_param.sched_priority = priorities[0];
        task2_param.sched_priority = priorities[1];
        task3_param.sched_priority = priorities[2];
        task4_param.sched_priority = priorities[3];

        // Set the scheduling parameters of the threads
        pthread_attr_setschedparam(&task1_attr, &task1_param);
        pthread_attr_setschedparam(&task2_attr, &task2_param);
        pthread_attr_setschedparam(&task3_attr, &task3_param);
        pthread_attr_setschedparam(&task4_attr, &task4_param);
    }

    int i;
    for(i=0; i<TASKS; i++){
        
        struct timespec next_activation;
        {
            clock_gettime(CLOCK_MONOTONIC, &next_activation);
            next_activation.tv_sec += peridiods[i]/1000000000;
            next_activation.tv_nsec += peridiods[i]%1000000000;
        };
        
        // CREATE THREADS
        if(i==0){
            task1_code();
        }
        else if(i==1){
            task2_code();
        }
        else if(i==2){
            task3_code();
        }

        // APERIODIC TASK
        else if(i==3){
            task4_code();
        }

        // SET THE THREADS AFFINITY
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);
    }


    exit(0);
}




