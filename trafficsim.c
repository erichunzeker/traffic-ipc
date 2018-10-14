
#define __NR_cs1550_down 326;
#define __NR_cs1550_up 325;

struct cs1550_sem
{
    int value;
    //Some queue of your devising
};

void down(cs1550_sem *sem) {
    syscall(__NR_cs1550_down, sem);
}

void up(cs1550_sem *sem) {
    syscall(__NR_cs1550_up, sem);
}

