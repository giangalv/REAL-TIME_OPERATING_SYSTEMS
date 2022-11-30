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

// COMPILE WITH: g++ -lpthread <file_name>.cpp -o <file_name>

//-------------------------------------LIBRARIES----------------------------------------
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

//-------------------------------GLOBAL VARIABLES------------------------------------------
#define PERIOD_1 300000000 // 300ms in nanoseconds
#define PERIOD_2 500000000  // 500ms in nanoseconds
#define PERIOD_3 800000000  // 800ms in nanoseconds

#define PERIODIC_TASKS 3
#define APERIODIC_TASKS 1
#define TASKS PERIODIC_TASKS + APERIODIC_TASKS

#define INNERLOOP 100
#define OUTERLOOP 2000

// INIZIALIZATION OF PERIODIC TASKS

void task1_code()
void task2_code()
void task3_code()

// INIZIALIZATION OF APERIODIC TASKS

void task4_code()

// CARATERISTICS FUNCTIONS OF PERIODIC TRHEADS

void *task1(void *arg)
void *task2(void *arg)
void *task3(void *arg)

// CARATERISTICS FUNCTIONS OF APERIODIC TRHEADS

void *task4(void *arg)

// INIZIALIZATION OF MUTEX

pthread_mutex_t mutex_task4 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_task4 = PTHREAD_COND_INITIALIZER;

// PERIODIC TASKS CREATION
long int periods[TASKS];

struct timespec next_activation[TASKS];
double wcet[TASKS];
pthread_attr_t attr[TASKS];
pthread_t thread[TASKS];
struct sched_param param[TASKS];

// ---------------------------------MAIN----------------------------------------------------

int main()
{
    // Set the task periods in nanoseconds
    periods[0] = PERIOD_1;
    periods[1] = PERIOD_2;
    periods[2] = PERIOD_3;

    // for aperiority task we set the period to 0
    periods[3] = 0;

    // set the max and min priority
    struct sched_param priomax;
    priomax.sched_priority = sched_get_priority_max(SCHED_FIFO);
    struct sched_param priomin;
    priomin.sched_priority = sched_get_priority_min(SCHED_FIFO); 

    if (geteuid() == 0)
    {     
        // Set the scheduling parameters of the threads
        pthread_setschedparam(pthread_self(), SCHED_FIFO, &priomax);
    }
    else
    {
        printf("You must be root to run this program\n");
        exit(-1);
    }

    // open the special file associated with the driver
    int fd;
    if((fd = open("/dev/my", O_RDWR)) < 0)
    {
        printf("Error opening the device file\n");
        exit(-1);
    }

    // UNCOMMENT the following lines to using the priority ceiling mutex
    /*
    // set the priority ceiling of the mutex
    pthread_mutexattr_t mutex_semaphore_attr;

    // initialize the mutex attributes
    pthread_mutexattr_init(&mutex_semaphore_attr);

    // set the priority ceiling protocol
    pthread_mutexattr_setprotocol(&mutex_semaphore_attr, PTHREAD_PRIO_PROTECT);

    // set the priority ceiling of the mutex to the max priority
    pthread_mutexattr_setprioceiling(&mutex_semaphore_attr, priomax.sched_priority + TASKS);

    // initialize the mutex
    pthread_mutex_init(&mutex_task4, &mutex_semaphore_attr);
    */

   // string to be written on the file
    char string[200];

    // execute the periodic tasks and the aperiodic task in background
    int i;
    for(i=0; i<TASKS; i++){
        
        // initialize the time_1 and time_2 variables required to read the execution time of the tasks
        struct timespec time_1, time_2;
        clock_gettime(CLOCK_REALTIME, &time_1);
        
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

        // read the execution time of the tasks
        clock_gettime(CLOCK_REALTIME, &time_2);

        // compute the worst case execution time of the tasks
        wcet[i] = (time_2.tv_sec - time_1.tv_sec) * 1000000000 + (time_2.tv_nsec - time_1.tv_nsec);

        // write the wcet on the file
        sprintf(string, "Worst Case Execution Time of Task %d: %f", i+1, wcet[i]);
        if(write(fd, string, strlen(string)) < 0)
        {
            printf("Error writing on the device file\n");
            exit(-1);
        }
    }

    // COMPUTE THE DEADLINE OF THE TASKS
    double U = wcet[0]/periods[0] + wcet[1]/periods[1] + wcet[2]/periods[2];

    // COMPUTE THE DEADLINE OF THE APERIODIC TASK
    double Ulub = (pow(2, 1.0/PERIODIC_TASKS) - 1) * PERIODIC_TASKS;

    // check if the system is schedulable
    if(U > Ulub)
    {
        printf("The system is not schedulable\n");
        sprintf("U = %f, Ulub = %f", U, Ulub);
        if(write(fd, string, strlen(string)) < 0)
        {
            printf("Error writing on the device file\n");
            exit(-1);
        }
        exit(-1);
    }
    else
    {
        printf("The system is schedulable\n");
        sprintf("U = %f, Ulub = %f", U, Ulub);
        if(write(fd, string, strlen(string)) < 0)
        {
            printf("Error writing on the device file\n");
            exit(-1);
        }
    }

    // CLOSE THE FILE
    close(fd);

    sleep(10);

    // SET THE PRIORITY OF THE THREADS with the min priority 
    if (geteuid() == 0)
        pthread_setschedparam(pthread_self(), SCHED_FIFO, &priomin);

    // SET THE PERIODIC THREADS
    for(i=0; i<PERIODIC_TASKS; i++){

        // set the scheduling policy
        pthread_attr_init(&attr[i]);

        // set the scheduling policy
        pthread_attr_setinheritsched(&attr[i], PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&attr[i], SCHED_FIFO);

        // set the priority of the threads
        param[i].sched_priority = priomax.sched_priority + TASKS - i;

        // set the scheduling parameters of the threads
        pthread_attr_setschedparam(&attr[i], &param[i]);
    }

    // SET THE APERIODIC THREAD
    for(i=PERIODIC_TASKS; i<TASKS; i++){

        // set the scheduling policy
        pthread_attr_init(&attr[i]);

        // set the scheduling policy
        pthread_attr_setinheritsched(&attr[i], PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&attr[i], SCHED_FIFO);

        // set the priority of the threads
        param[i].sched_priority = priomax.sched_priority + TASKS - i;

        // set the scheduling parameters of the threads
        pthread_attr_setschedparam(&attr[i], &param[i]);
    }

    // DECLARE a VARIABLE to store the return value of the pthread_create function
    int iret[TASKS];

    // DECLARE a VARIABLE to read the current time
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);

    // SET the next activation time of the periodic tasks
    for(i=0; i<PERIODIC_TASKS; i++){

        // time in nanoseconds add the period to the current time
        long int next_arrival_nanoseconds = time.tv_nsec + periods[i];

        // end of the period and start of the new period
        next_activation[i].tv_sec = time.tv_sec + next_arrival_nanoseconds / 1000000000;
        next_activation[i].tv_nsec = next_arrival_nanoseconds % 1000000000;
    }

    printf("Start of the execution of the tasks\n");
    fflush(stdout);

    // CREATE the THREADS
    for(i=0; i<TASKS; i++){

        // create the threads
        iret[i] = pthread_create(&thread[i], &attr[i], task_code[i], NULL);
    }

    // JOIN the THREADS
    for(i=0; i<TASKS; i++){

        // join the threads
        pthread_join(thread[i], NULL);
    }

    printf("End of the execution of the tasks\n");
    fflush(stdout);

    // UNNCOMMENT the following lines to destroy the mutex
    /*

    // DESTROY the MUTEX
    pthread_mutex_destroy(&mutex_task4); 
    */
    exit(0);
}

// TASK 1
int task1_code()
{

    
}




