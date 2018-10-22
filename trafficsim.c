#include <linux/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

#define __NR_cs1550_down 326
#define __NR_cs1550_up 325

struct cs1550_sem {
    int value;
    struct Node *front;
    struct Node *back;
};

struct semQueues {
    int north[10];      //queue to hold the cars in north direction
    int south[10];      //queue to hold the cars in south direction

    int northFront;     //keeps track of location of front of north queue
    int southFront;     //keeps track of location of front of south queue
    int northBack;      //keeps track of back of north queue to add cars to end of queue in north
    int southBack;      //keeps track of back of south queue to add cars to end of queue in south

    int northCount;     //keeps track of north's car number
    int southCount;     //keeps track of south's car number

    struct cs1550_sem northFull;    //semaphore who's value is 10 when north queue is full
    struct cs1550_sem southFull;    //semaphore who's value is 10 when south queue is full

    struct cs1550_sem northEmpty;   //semaphore who's value is 0 when north queue is empty
    struct cs1550_sem southEmpty;   //semaphore who's value is 0 when south queue is full

    struct cs1550_sem northSem;     //semaphore for mutex in north
    struct cs1550_sem southSem;     //semaphore for mutex in south

    struct cs1550_sem NSFull;   //semaphore for both queues being full
};

void down(struct cs1550_sem *sem) {
    syscall(__NR_cs1550_down, sem);
}

void up(struct cs1550_sem *sem) {
    syscall(__NR_cs1550_up, sem);
}

struct semQueues *mem;

void producerNorth() {
    while (1) {
        if(mem->northCount <= 10) {
            down(&(mem->northEmpty));   //decrement north queue availability
            down(&(mem->northSem));     //lock resources used by producer north
            *(mem->north + mem->northBack) = mem->northBack;    //assign back to new back of queue
            mem->northBack = (mem->northBack + 1) % 10;         //new back is newest car in line
            //https://stackoverflow.com/questions/5141960/get-the-current-time-in-c
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );

            printf("Car %d coming from the North direction arrived in the queue at time %s .\n", mem->northCount, asctime (timeinfo));
            mem->northCount++;          //increment cars in line
            up(&(mem->northSem));       //set north sem back to 1
            up(&(mem->northFull));      //increase full sem
            up(&(mem->NSFull));         //increase north/south sem
            if ((rand()) % 10 > 8) {    // 80% chance car is following -> if not, sleep 20 seconds
                sleep(20);
            }
        }
    }
}

void producerSouth() {
    while (1) {
        if (mem->southCount <= 10) {         //same comments as producerNorth except southified
            down(&(mem->southEmpty));
            down(&(mem->southSem));
            *(mem->south + mem->southBack) = mem->southBack;
            mem->southBack = (mem->southBack + 1) % 10;

            time_t rawtime;
            struct tm *timeinfo;

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            printf("Car %d coming from the South direction arrived in the queue at time %s .\n", mem->southCount, asctime(timeinfo));
            mem->southCount++;
            up(&(mem->southSem));
            up(&(mem->southFull));
            up(&(mem->NSFull));
            if ((rand()) % 10 > 8) {
                sleep(20);
            }
        }
    }
}

void flagman() {
    while (1) {
        if ((mem->northCount == 1) && (mem->southCount == 1)) {
            printf("\nThe flagperson is now asleep.\n");
            down(&(mem->NSFull));   //lock all three sems
            down(&(mem->northSem));
            down(&(mem->southSem));
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            if (mem->northCount == 1) {
                printf("\nThe flagperson is now awake.\n");
                printf("\nCar %d coming from the North direction, blew their horn at time %s .\n", mem->northCount, asctime (timeinfo));
            }
            else if (mem->southCount == 1) {
                printf("\nThe flagperson is now awake.\n");
                printf("\nCar %d coming from the South direction, blew their horn at time %s .\n", mem->southCount, asctime (timeinfo));
            }
            up(&(mem->southSem));
            up(&(mem->northSem));
            up(&(mem->NSFull));
        }

        while (mem->northCount > 1) {
            down(&(mem->northFull));
            down(&(mem->northSem));
            mem->northFront = (mem->northFront + 1) % 10;
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            mem->northCount--;
            printf("Car %d coming from the North direction left the construction zone at time %s .\n", mem->northCount, asctime (timeinfo));
            up(&(mem->northSem));
            up(&(mem->northEmpty));
            down(&(mem->NSFull));
            sleep(2);

            if(mem->southCount == 10)
                break;
        }

        while (mem->southCount > 1) {
            down(&(mem->southFull));
            down(&(mem->southSem));
            mem->southFront = (mem->southFront + 1) % 10;
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            mem->southCount--;
            printf("Car %d coming from the South direction left the construction zone at time %s.\n", mem->southCount, asctime (timeinfo));
            up(&(mem->southSem));
            up(&(mem->southEmpty));
            down(&(mem->NSFull));
            sleep(2);

            if(mem->northCount == 10)
                break;
        }
    }
}

int main() {
    mem = mmap(NULL, sizeof(struct semQueues), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    mem->northCount = 1;
    mem->northBack = 0;
    mem->southCount = 1;
    mem->southBack = 0;

    mem->northEmpty.value = 10;
    mem->northEmpty.front = NULL;
    mem->northEmpty.back = NULL;
    mem->southEmpty.value = 10;
    mem->southEmpty.front = NULL;
    mem->southEmpty.back = NULL;

    mem->northFull.value = 0;
    mem->northFull.front = NULL;
    mem->northFull.back = NULL;
    mem->southFull.value = 0;
    mem->southFull.front = NULL;
    mem->southFull.back = NULL;

    mem->northSem.value = 1;
    mem->northSem.front = NULL;
    mem->northSem.back = NULL;
    mem->southSem.value = 1;
    mem->southSem.front = NULL;
    mem->southSem.back = NULL;

    mem->NSFull.value = 0;
    mem->NSFull.front = NULL;
    mem->NSFull.back = NULL;

    if (fork() == 0) {
        flagman();
    } if (fork() == 0) {
        producerNorth();
    } if (fork() == 0) {
        producerSouth();
    } wait(NULL);
}