#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>

struct msg_buffer
{
    long msg_type;
    int signal;
};

void semaphore_wait(int semid)
{
    struct sembuf op = {0, -1, 0};
    if (semop(semid, &op, 1) == -1)
    {
        perror("Semaphore wait failed");
        exit(1);
    }
}

void semaphore_signal(int semid)
{
    struct sembuf op = {0, 1, 0};
    if (semop(semid, &op, 1) == -1)
    {
        perror("Semaphore signal failed");
        exit (1);
    }
}

int main()
{
    key_t key_msg = ftok("atmmsg", 65);
    key_t key_shm = ftok("atmshm", 75);
    key_t key_sem = ftok("atmsem", 85);

    int msgid = msgget(key_msg, 0666 | IPC_CREAT);
    if (msgid == -1)
    {
        perror("Message queue creation failed");
        exit(1);
    }

    int shmid = shmget(key_shm, 1024, 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("Shared memory creation failed");
        exit(1);
    }

    char *shared_memory = (char *)shmat(shmid, NULL, 0);
    if (shared_memory == (char *)-1)
    {
        perror("Shared memory attachment failed");
        exit(1);
    }

    int semid = semget(key_sem, 1, 0666 | IPC_CREAT);
    if (semid == -1)
    {
        perror("Semaphore creation failed");
        exit(1);
    }

    struct msg_buffer message;

    if (msgrcv(msgid, &message, sizeof(message), 1, 0) == -1)
    {
        perror("Message receive failed");
        exit(1);
    }

    if (message.signal == 1)
    {
        semaphore_wait(semid);
        printf("%s\n", shared_memory);
        semaphore_signal(semid);
    }

    shmdt(shared_memory);

    return 0;
}


