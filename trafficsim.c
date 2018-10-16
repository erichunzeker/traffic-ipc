#include <linux/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define __NR_cs1550_down 326
#define __NR_cs1550_up 325

struct cs1550_node {
    struct cs1550_node* next;
    struct task_struct* task;
};

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


int main() {
    printf("test");
    return 0;
}

