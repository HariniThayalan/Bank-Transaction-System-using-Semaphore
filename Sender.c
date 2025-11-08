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
        exit(1);
    }
}

void get_last_transaction(const char *pin, char *transaction_details, int *balance)
{
    FILE *file = fopen("customers.txt", "r");
    if (!file)
    {
        strcpy(transaction_details, "Error: Unable to access customers.txt\n");
        *balance = -1;
        return;
    }

    char line[256];
    char file_pin[10];
    int file_balance;
    int found = 0;

    while (fgets(line, sizeof(line), file))
    {
        if (sscanf(line, "PIN:%s | Operation:%*[^|] | Amount:%*d | Final Balance:%d", file_pin, &file_balance) == 2)
        {
            file_pin[strcspn(file_pin, "\n")] = '\0';
            if (strcmp(file_pin, pin) == 0)
            {
                found = 1;
                strcpy(transaction_details, line); // Store the latest match
                *balance = file_balance;
            }
        }
    }

    fclose(file);

    if (!found)
    {
        strcpy(transaction_details, "Error: Account does not exist\n");
        *balance = -1;
    }
}

void log_transaction(const char *pin, const char *operation, int amount, int final_balance)
{
    FILE *file = fopen("customers.txt", "a");
    if (!file)
    {
        perror("Error: Failed to open customers.txt for logging");
        exit(1);
    }
    fprintf(file, "PIN:%s | Operation:%s | Amount:%d | Final Balance:%d\n", pin, operation, amount, final_balance);
    fclose(file);
}

int main()
{
    key_t key_msg = ftok("atmmsg", 65);
    key_t key_shm = ftok("atmshm", 75);
    key_t key_sem = ftok("atmsem", 85);

    int msgid = msgget(key_msg, 0666 | IPC_CREAT);
    int shmid = shmget(key_shm, 1024, 0666 | IPC_CREAT);
    char *shared_memory = (char *)shmat(shmid, NULL, 0);
    if (shared_memory == (char *)-1)
    {
        perror("Shared memory attachment failed");
        exit(1);
    }

    int semid = semget(key_sem, 1, 0666 | IPC_CREAT);
    semctl(semid, 0, SETVAL, 1);

    struct msg_buffer message;
    message.msg_type = 1;

    char pin[10], operation[20];
    int amount, balance, new_balance;

    scanf("%s", pin);
    scanf("%s", operation);
    scanf("%d", &amount);

    semaphore_wait(semid);

    char last_transaction[256];
    get_last_transaction(pin, last_transaction, &balance);

    if (balance == -1 && strcmp(operation, "Create") != 0)
    {
        sprintf(shared_memory, "Error: Account does not exist for PIN %s", pin);
    }
    else
    {
        if (strcmp(operation, "Deposit") == 0)
        {
            new_balance = balance + amount;
            log_transaction(pin, "Deposit", amount, new_balance);
            sprintf(shared_memory, "PIN: %s\nOperation: Deposit\nAmount: %d\nFinal Balance: %d",
                    pin, amount, new_balance);
        }
        else if (strcmp(operation, "Withdraw") == 0)
        {
            if (amount > balance)
            {
                sprintf(shared_memory, "PIN: %s\nOperation: Withdraw\nFailed: Insufficient Balance", pin);
            }
            else
            {
                new_balance = balance - amount;
                log_transaction(pin, "Withdraw", amount, new_balance);
                sprintf(shared_memory, "PIN: %s\nOperation: Withdraw\nAmount: %d\nFinal Balance: %d",
                        pin, amount, new_balance);
            }
        }
        else if (strcmp(operation, "Check") == 0)
        {
            sprintf(shared_memory, "PIN: %s\nOperation: Check Balance\nBalance: %d",
                    pin, balance);
        }
        else if (strcmp(operation, "Create") == 0)
        {
            log_transaction(pin, "Create Account", amount, amount);
            sprintf(shared_memory, "Account created successfully\nPIN: %s\nInitial Balance: %d", pin, amount);
        }
        else
        {
            sprintf(shared_memory, "Invalid Operation");
        }
    }

    semaphore_signal(semid);
    shmdt(shared_memory);

    message.signal = 1;
    msgsnd(msgid, &message, sizeof(message), 0);

    return 0;
}


