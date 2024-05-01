#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_NODES 30



struct my_msgbuf
{
    long mtype;
    int sequence_num;
    int operation_num;
    char name[200];
    char mtext[200];
};

struct graph_data
{   
    int numNodes;
    int adjacencyMatrix[MAX_NODES][MAX_NODES];
};

sem_t mutex;

void *handleRequest(void *arg)
{
    struct my_msgbuf *msg = (struct my_msgbuf *)arg;
    int sequenceNumber = msg->sequence_num;
    int operationNumber = msg->operation_num;
    char graphFileName[200];
    strcpy(graphFileName, msg->name);

    key_t key = sequenceNumber;
    if (key == -1)
    {
        perror("Error in ftok\n");
        pthread_exit(NULL);
    }

    int shmid = shmget(key, sizeof(struct graph_data), 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("Error in shmget\n");
        pthread_exit(NULL);
    }

    struct graph_data *sharedGraphData = shmat(shmid, NULL, 0);
    if (sharedGraphData == (void *)-1)
    {
        perror("Error in shmat\n");
        pthread_exit(NULL);
    }

    FILE *graphFile;
    if (operationNumber == 1)
    {
        graphFile = fopen(graphFileName, "w");
    }
    else
    {
        graphFile = fopen(graphFileName, "w");
    }
// maxnode hatao n dalo
    if (graphFile != NULL)
    {   fprintf(graphFile, "%d ", sharedGraphData->numNodes);
        fprintf(graphFile, "\n");
        for (int i = 0; i < sharedGraphData->numNodes; i++)
        {
            for (int j = 0; j < sharedGraphData->numNodes; j++)
            {
                fprintf(graphFile, "%d ", sharedGraphData->adjacencyMatrix[i][j]);
            }
            fprintf(graphFile, "\n");
        }
        fclose(graphFile);

        key_t keyx = ftok("loadBalancer.c", 'B');
        if (keyx == -1)
        {
            perror("Error in ftok\n");
            pthread_exit(NULL);
        }

        int msgid = msgget(keyx, 0666);
        if (msgid == -1)
        {
            perror("Error in msgget\n");
            pthread_exit(NULL);
        }

        struct my_msgbuf response;
        response.mtype = sequenceNumber;
        response.sequence_num = sequenceNumber;
        response.operation_num = operationNumber;

        if (operationNumber == 1)
        {
            strcpy(response.mtext, "File successfully added");
        }
        else
        {
            strcpy(response.mtext, "File successfully modified");
        }

        if (msgsnd(msgid, &response, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
        {
            perror("msgsnd");
            pthread_exit(NULL);
        }
    }
    else
    {
        perror("Error opening graph file\n");
    }

    shmdt(sharedGraphData);
    pthread_exit(NULL);
}

int main()
{
    key_t key;

    if ((key = ftok("loadBalancer.c", 'B')) == -1)
    {
        perror("Error in ftok\n");
        exit(1);
    }

    int msgid;
    if ((msgid = msgget(key, 0666 | IPC_CREAT)) == -1)
    {
        perror("Error in msgget\n");
        exit(2);
    }

    sem_init(&mutex, 0, 1);

    while (1)
    {
        struct my_msgbuf buf;
        buf.mtype = 102;
        if (msgrcv(msgid, &buf, sizeof(struct my_msgbuf) - sizeof(long), buf.mtype, 0) == -1)
        {
            perror("Error in msgrcv\n");
            exit(1);
        }

        pthread_t tid;
        if (pthread_create(&tid, NULL, handleRequest, (void *)&buf) != 0)
        {
            perror("Error creating thread\n");
            exit(1);
        }
    }

    sem_destroy(&mutex);

    return 0;
}
