#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#define MAX_NODES 30

int x1;
int x2;
int sqnum;
struct my_msgbuf
{
    long mtype;
    int sequence_num;
    int operation_num;
    char name[200];
    char mtext[200]; // Modified to be used for additional data
};
struct graph_data
{   
    int numNodes;
    int adjacencyMatrix[MAX_NODES][MAX_NODES];
};
struct graph_vertex
{
    int ver;
};

void write_to_shared_memory()
{
    key_t shm_key = sqnum;
    int shm_id = shmget(shm_key, sizeof(struct graph_data), IPC_CREAT | 0666);
    x1=shm_id;
    if (shm_id == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    struct graph_data *shared_memory = (struct graph_data *)shmat(shm_id, NULL, 0);

    if ((void *)shared_memory == (void *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    int num_nodes;

    // Get the number of nodes from the user
    printf("Enter the number of nodes of the graph: ");
    scanf("%d", &num_nodes);

    shared_memory->numNodes = num_nodes;

    // Get the adjacency matrix from the user and write to shared memory
    printf("Enter the adjacency matrix, each row on a separate line, and elements of a single row separated by whitespace characters:\n");

    for (int i = 0; i < num_nodes; ++i)
    {
        for (int j = 0; j < num_nodes; ++j)
        {
            scanf("%d", &shared_memory->adjacencyMatrix[i][j]);
        }
    }
    //memcpy(shared_memory,&shared_memory, sizeof(struct graph_data));
    // Detach from shared memory
   
}


// Function to read from shared memory
void read_from_shared_memory()
{
    key_t shm_key = sqnum+10000;
    int shm_id = shmget(shm_key, sizeof(struct graph_vertex), IPC_CREAT | 0666);
    x2=shm_id;
    if (shm_id == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    struct graph_vertex *shared_memory = (struct graph_vertex *)shmat(shm_id, NULL, 0);

    if ((void *)shared_memory == (void *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    int vertex;

    // Get the number of nodes from the user
    printf("Enter the starting vertex : ");
    scanf("%d", &vertex);

    shared_memory->ver = vertex;

}

int main()
{
    key_t key;
    int msqid;

    if ((key = ftok("loadBalancer.c", 'B')) == -1)
    {
        perror("Error in ftok\n");
        exit(1);
    }

    if ((msqid = msgget(key, 0666)) == -1)
    {
        perror("Error in msgget\n");
        exit(2);
    }
    while(1)
    {
    struct my_msgbuf buf;

    // Get user input for the request
    int a,b;
    printf("Enter Sequence Number: ");
    scanf("%d", &a);

    printf("Enter Operation Number: ");
    scanf("%d", &b);

    printf("Enter Graph File Name: ");
    scanf("%199s", buf.name);

    buf.operation_num=b;
    buf.sequence_num=a;
    sqnum=a;

    // Send the request to the load balancer via the message queue
    buf.mtype = 101; // load Balancer type 1 

if (buf.operation_num == 1 || buf.operation_num == 2)
    {
        // For write operations
        write_to_shared_memory();
    }
    else if (buf.operation_num == 3 || buf.operation_num == 4)
    {
        // For read operations
        read_from_shared_memory();
    }

    if (msgsnd(msqid, &buf, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
    {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    struct my_msgbuf bufx;
    bufx.mtype = buf.sequence_num;

    if (msgrcv(msqid, &bufx, sizeof(struct my_msgbuf) - sizeof(long), a , 0) == -1)
    {
        perror("Error in msgrcv\n");
        exit(1);
    }

        if (bufx.operation_num == 1 || bufx.operation_num == 2)
    {
        printf("%s\n", bufx.mtext);
        struct graph_data *shared = shmat(x1, NULL, 0);
        if (shmdt(shared) == -1)
        {
            perror("shmdt");
            exit(EXIT_FAILURE);
        }
    }

    else if (bufx.operation_num == 3 || bufx.operation_num == 4)
    {
        printf("%s\n", bufx.mtext);
        struct graph_data *shared = shmat(x2, NULL, 0);
        if (shmdt(shared) == -1)
        {
            perror("shmdt");
            exit(EXIT_FAILURE);
        }
    }

    
    }
    return 0;
}
