#include <linux/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define __NR_cs1550_down 326
#define __NR_cs1550_up 325

struct cs1550_sem {
    int value;
    struct cs1550_node* front;
    struct cs1550_node* back;
};

void down(struct cs1550_sem *sem) {
    syscall(__NR_cs1550_down, sem);
}
void up(struct cs1550_sem *sem) {
    syscall(__NR_cs1550_up, sem);
}

int north[10];
int south[10];

int *northFront;
int *northBack;

int *southFront;
int *southBack;



int *northCount;
int *southCount;

struct cs1550_sem northSem;
struct cs1550_sem southSem;

struct cs1550_sem northSemEmpty;
struct cs1550_sem southSemEmpty;

struct cs1550_sem northSemFull;
struct cs1550_sem southSemFull;

struct cs1550_sem NSFull;


void producerNorth();
void producerSouth();
void flagMan();

int main() {
    *northCount = 0;
    *southCount = 0;
    *northFront = 0;
    *northBack = 0;
    *southFront = 0;
    *southBack = 0;

    void *ptr = mmap(NULL, (sizeof(north) + sizeof(south) ), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

    northFront = ptr;

    northSem.value = 1;
    southSem.value = 1;
    northSem.front = NULL;
    southSem.front = NULL;
    northSem.back = NULL;
    southSem.back = NULL;

    northSemEmpty.value = 10;
    southSemEmpty.value = 10;
    northSemEmpty.front = NULL;
    southSemEmpty.front = NULL;
    northSemEmpty.back = NULL;
    southSemEmpty.back = NULL;

    northSemFull.value = 0;
    southSemFull.value = 0;
    northSemFull.front = NULL;
    southSemFull.front = NULL;
    northSemFull.back = NULL;
    southSemFull.back = NULL;

    NSFull.value = 0;
    NSFull.front = NULL;
    NSFull.back = NULL;

    printf("TRAFFICSIM - eric hunzeker\n\n");

    if(fork() == 0) {
        producerNorth();
    } if(fork() == 0) {
        producerSouth();
    } if(fork() == 0) {
        flagMan();
    }

    wait(NULL);
    return 0;
}

void producerNorth() {
    while(1) {
        down(&northSemEmpty);
        down(&northSem);
        *(north + *northBack) = *northBack;
        *northBack = (*northBack + 1) % 10;
        printf("Car %d coming from the %c direction arrived in the queue at time %d.\n", *northCount);
        if(NSFull.value == 0) {
            printf("Car %d coming from the %c direction, blew their horn at time %d.", *northCount);
        }
        *northCount++;
        up(&northSem);
        up(&northSemFull);
        up(&NSFull);

        if(rand() % 10 > 8)
            sleep(20);
    }
};

void producerSouth() {
    while(1) {
        down(&southSemEmpty);
        down(&southSem);
        *(south + *southBack) = *southBack;
        *southBack = (*southBack + 1) % 10;
        printf("Car %d coming from the %c direction arrived in the queue at time %d.\n", *southCount);
        if(NSFull.value == 0) {
            printf("Car %d coming from the %c direction, blew their horn at time %d.", *southCount);
        }
        *southCount++;
        up(&southSem);
        up(&southSemFull);
        up(&NSFull);

        if(rand() % 10 > 8)
            sleep(20);
    }
};

void flagMan() {
    *northCount = 0;
    *southCount = 0;
    while (1) {
        if (*northCount == 0 && *southCount == 0) {
            printf("The flagperson is now asleep.\n");
            down(&NSFull);

            if(*northCount > 0) {
                printf("Car %d coming from the %c direction, blew their horn at time %d.");
            }

            up(&NSFull);

        }



        while(*northCount > 0) {
            down(&northSemFull);
            down(&northSem);
            *northFront = (*northFront + 1) % 10;
            printf("Car %d coming from the %c direction left the construction zone at time %d.", *northFront);
            *northCount--;
            up(&northSem);
            up(&northSemEmpty);
            down(&NSFull);
            sleep(2);

            if(*southCount == 10) {
                break;
            }
        }

        while(*southCount > 0) {
            down(&southSemFull);
            down(&southSem);
            *southFront = (*southFront + 1) % 10;
            printf("Car %d coming from the %c direction left the construction zone at time %d.", *southFront);
            *southCount--;
            up(&southSem);
            up(&southSemEmpty);
            down(&NSFull);
            sleep(2);

            if(*northCount == 10) {
                break;
            }
        }
    }
}