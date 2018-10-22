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
    int north[10];
    int south[10];

    int northFront;
    int southFront;
    int northBack;
    int southBack;

    int northCount;
    int southCount;

    struct cs1550_sem northFull;
    struct cs1550_sem southFull;

    struct cs1550_sem northEmpty;
    struct cs1550_sem southEmpty;

    struct cs1550_sem northSem;
    struct cs1550_sem southSem;

    struct cs1550_sem NSFull;
};

void down(struct cs1550_sem *sem) {
    syscall(__NR_cs1550_down, sem);
}

void up(struct cs1550_sem *sem) {
    syscall(__NR_cs1550_up, sem);
}

struct semQueues *mem;

void producerNorth() {
    int n = 1;
    while (1) {
        down(&(mem->northEmpty));
        down(&(mem->northSem));
        *(mem->north + mem->northBack) = mem->northBack;
        mem->northBack = (mem->northBack + 1) % 10;

        //https://stackoverflow.com/questions/5141960/get-the-current-time-in-c

        time_t rawtime;
        struct tm * timeinfo;

        time ( &rawtime );
        timeinfo = localtime ( &rawtime );

        printf("Car %d coming from the North direction arrived in the queue at time %s .\n", n++, asctime (timeinfo));
        mem->northCount++;
        up(&(mem->northSem));
        up(&(mem->northFull));
        up(&(mem->NSFull));
        if ((rand()) % 10 > 8) {
            sleep(20);
        }
    }
}

void producerSouth() {
    int n = 1;
    while (1) {
        down(&(mem->southEmpty));
        down(&(mem->southSem));
        *(mem->south + mem->southBack) = mem->southBack;
        mem->southBack = (mem->southBack + 1) % 10;

        time_t rawtime;
        struct tm * timeinfo;

        time ( &rawtime );
        timeinfo = localtime ( &rawtime );

        printf("Car %d coming from the South direction arrived in the queue at time %s .\n", n++, asctime (timeinfo));
        mem->southCount++;
        up(&(mem->southSem));
        up(&(mem->southFull));
        up(&(mem->NSFull));
        if ((rand()) % 10 > 8) {
            sleep(20);
        }
    }
}

void flagman() {
    int northCar = 1;
    int southCar = 1;
    while (1) {
        if ((mem->northCount == 1) && (mem->southCount == 1)) {
            printf("\nThe flagperson is now asleep.\n");
            down(&(mem->NSFull));
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            if (mem->northCount > 1) {
                printf("\nCar %d coming from the North direction, blew their horn at time %s .\n", northCar, asctime (timeinfo));
            }
            else if (mem->southCount > 1) {
                printf("\nCar %d coming from the South direction, blew their horn at time %s .\n", southCar, asctime (timeinfo));
            }
            up(&(mem->NSFull));
        }

        while (mem->northCount > 1 && mem->southCount == 11) {
            down(&(mem->northFull));
            down(&(mem->northSem));
            mem->northFront = (mem->northFront + 1) % 10;
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            printf("Car %d coming from the North direction left the construction zone at time %s .\n", northCar++, asctime (timeinfo));
            mem->northCount--;
            up(&(mem->northSem));
            up(&(mem->northEmpty));
            down(&(mem->NSFull));
            sleep(2);
        }

        while (mem->southCount > 1 && mem->northCount < 11) {
            down(&(mem->southFull));
            down(&(mem->southSem));
            mem->southFront = (mem->southFront + 1) % 10;

            time_t rawtime;
            struct tm * timeinfo;

            time ( &rawtime );
            timeinfo = localtime ( &rawtime );

            printf("Car %d coming from the South direction left the construction zone at time %s.\n", southCar++, asctime (timeinfo));
            mem->southCount--;
            up(&(mem->southSem));
            up(&(mem->southEmpty));
            down(&(mem->NSFull));
            sleep(2);
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
    }
    wait(NULL);
}